"""Compila em C++23 e executa regressao sem modificar massas ou bancos originais.

Uso na raiz do projeto: python tools/testar_tuss.py
Requer gcc e g++ no PATH (MinGW no Windows).
"""
import os
import hashlib
import json
from contextlib import closing
from pathlib import Path
import re
import subprocess
import tempfile
import sqlite3
import csv

ROOT = Path(__file__).resolve().parents[1]
CORE = ROOT / "LEGATECH_Core"
CATALOG = ROOT / "data/reference/tuss22_202501.csv"
GROUPS = ROOT / "data/reference/tuss63_202501.csv"


def run(args):
    result = subprocess.run([str(a) for a in args], capture_output=True, text=True)
    if result.stderr:
        print(result.stderr.strip())
    result.check_returncode()
    return result


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def expect_failure(args, message):
    result = subprocess.run([str(a) for a in args], capture_output=True, text=True)
    require(result.returncode == 1, message + ": retorno diferente de 1")
    require("Falha de configuracao/execucao" in result.stderr, message + ": erro nao comunicado")
    require("Auditoria confirmada" not in result.stdout, message + ": sucesso indevido")


def check_counts(output, expected):
    labels = ["Total processado", "Validos", "Validos com alerta", "Rejeitados", "Estrutura divergente"]
    counts = []
    for label in labels:
        match = re.search(r"^" + re.escape(label) + r":\s+(\d+)", output, re.MULTILINE)
        if not match:
            raise RuntimeError("Resumo ausente: " + label)
        counts.append(int(match.group(1)))
    if counts != expected:
        raise RuntimeError(f"Resumo divergente: {counts}; esperado {expected}")


manifest = json.loads((ROOT / "data/reference/manifest_tuss63_202501.json").read_text(encoding="utf-8"))
require(manifest["table"] == "63" and manifest["catalog_version"] == "202501" and manifest["records"] == 31,
        "Identificacao do snapshot 63 divergente")
# O manifesto identifica o CSV normalizado com LF. Checkout com core.autocrlf
# pode usar CRLF no Windows; canoniza somente essa representacao de nova linha.
# Nenhum codigo, termo, data ou outro byte de conteudo e normalizado.
catalog_bytes = GROUPS.read_bytes().replace(b"\r\n", b"\n")
require(hashlib.sha256(catalog_bytes).hexdigest() == manifest["catalog_sha256"], "Hash do catalogo 63 divergente")

with tempfile.TemporaryDirectory(prefix="legatech-build-") as scratch:
    build = Path(scratch)
    suffix = ".exe" if os.name == "nt" else ""
    sqlite = build / "sqlite3.o"
    run(["gcc", "-O0", "-c", CORE / "src/sqlite3.c", "-o", sqlite])
    sources = sorted((CORE / "src").glob("*.cpp"))
    common = ["g++", "-std=c++23", "-Wall", "-Wextra", "-O0", "-I", CORE / "include"]
    libraries = [] if os.name == "nt" else ["-lpthread", "-ldl"]
    app = build / ("legatech" + suffix)
    tests = build / ("test_catalogo_tuss" + suffix)
    run(common + [CORE / "main.cpp"] + sources + [sqlite] + libraries + ["-o", app])
    run(common + [ROOT / "tests/test_catalogo_tuss.cpp"] + sources + [sqlite] + libraries + ["-o", tests])
    print(run([tests, CATALOG, GROUPS]).stdout.strip())
    for name, expected in [("PR_202501_AMB_DET_sample.csv", [10, 7, 3, 0, 0]),
                           ("PR_202501_AMB_DET_sample_rejeitado.csv", [10, 6, 3, 1, 0])]:
        result = run([app, ROOT / "data/sample" / name, CATALOG])
        check_counts(result.stdout, expected)
        print("OK:", name, expected)
    # Layout historico fisico com 10 campos e competencia anterior a 2023.
    historical = build / "PR_202201_AMB_DET.csv"
    historical.write_text(
        "ID_EVENTO_ATENCAO_SAUDE;UF_PRESTADOR;DT_REALIZACAO;CD_PROCEDIMENTO;CD_TABELA_REFERENCIA;"
        "QT_ITEM_EVENTO_INFORMADO;VL_ITEM_EVENTO_INFORMADO;VL_ITEM_PAGO_FORNECEDOR;IND_PACOTE;IND_TABELA_PROPRIA\n"
        '1;"PR";"2022-01";"10101012";"22";1;100;0;0;0\n', encoding="utf-8")
    check_counts(run([app, historical, CATALOG]).stdout, [1, 1, 0, 0, 0])
    print("OK: contrato historico em CSV")
    failure = subprocess.run([str(app), str(historical), str(build / "ausente.csv")], capture_output=True, text=True)
    if failure.returncode != 1 or "Falha de configuracao" not in failure.stderr:
        raise RuntimeError("Catalogo ausente deveria interromper execucao")
    print("OK: falha de configuracao antes de processar a massa")

    # Consulta o banco real produzido pelo executavel, sem usar bancos originais.
    # O contexto sqlite3 controla a transacao; closing fecha a conexao tambem
    # em excecoes, liberando o arquivo para a limpeza temporaria no Windows.
    database = build / "auditoria.db"
    positive = ROOT / "data/sample/PR_202501_AMB_DET_sample.csv"
    negative = ROOT / "data/sample/PR_202501_AMB_DET_sample_rejeitado.csv"
    for source, expected in [(positive, [10, 7, 3, 0, 0]), (negative, [10, 6, 3, 1, 0])]:
        output = run([app, source, CATALOG, database]).stdout
        check_counts(output, expected)
        require("Auditoria confirmada" in output, "Confirmacao da auditoria ausente")
    with closing(sqlite3.connect(database)) as db, db:
        executions = db.execute("SELECT id,competencia,versao_catalogo,tabela_catalogo,total,validos,alertas,"
                                "rejeitados,estrutura_divergente,fim_utc FROM auditoria_execucao ORDER BY id").fetchall()
        require(len(executions) == 2, "Reexecucao deve preservar o historico")
        require(executions[0][1:9] == ("202501", "202501", "22", 10, 7, 3, 0, 0), "Metadados/resumo positivo")
        require(executions[1][1:9] == ("202501", "202501", "22", 10, 6, 3, 1, 0), "Metadados/resumo negativo")
        require(all(row[-1] for row in executions), "Execucao sem encerramento")
        require(db.execute("SELECT COUNT(*) FROM auditoria_resultado").fetchone()[0] == 20, "Todos os estados gravados")
        for execution, source in zip(executions, [positive, negative]):
            raw = source.read_bytes().decode("utf-8").split("\n")
            rows = db.execute("SELECT linha_fisica,linha_original,cd_tabela_referencia,cd_procedimento "
                              "FROM auditoria_resultado WHERE execucao_id=? ORDER BY linha_fisica", (execution[0],)).fetchall()
            for number, original, table, code in rows:
                require(original == raw[number - 1], "Linha original/numero fisico divergente")
                fields = original.rstrip("\r").split(";")
                require(table == fields[4].strip('"') and code == fields[3].strip('"'), "Par tabela/codigo alterado")
        require(db.execute("SELECT COUNT(*) FROM auditoria_ocorrencia WHERE tipo='MOTIVO'").fetchone()[0] > 0,
                "Motivos de rejeicao nao persistidos")
        require(db.execute("SELECT COUNT(*) FROM auditoria_ocorrencia WHERE tipo='ALERTA'").fetchone()[0] >= 6,
                "Alertas nao persistidos")
        require(db.execute("PRAGMA foreign_key_check").fetchall() == [], "Referencias de auditoria invalidas")
        require(db.execute("PRAGMA integrity_check").fetchone()[0] == "ok", "Integridade SQLite")
        require(db.execute("SELECT COUNT(*) FROM sqlite_master WHERE name='registro_tiss'").fetchone()[0] == 0,
                "Auditoria nao deve alimentar tabela operacional")
    print("OK: auditoria SQLite, tres estados, metadados, ocorrencias, linhas e reexecucao")

    mixed = build / "PR_202501_AMB_DET_misto.csv"
    header = positive.read_text(encoding="utf-8").splitlines()[0]
    mixed.write_bytes((header + "\r\n"
        '1;"PR";"2025-01";"10101012";"22";1;100;0;;0;0\r\n'
        '\r\n'
        '2;"PR";"2025-01";"018";"63";1;100;0;;0;0\r\n'
        '3;"PR";"2025-99";"10101012";"22";1;100;0;;0;0\r\n'
        'registro;truncado\r\n'
        '4;"PR";"2025-01";"010101012";"22";1;100;0;;0;0\r\n').encode("utf-8"))
    check_counts(run([app, mixed, CATALOG, database]).stdout, [5, 1, 2, 2, 1])
    with closing(sqlite3.connect(database)) as db, db:
        execution = db.execute("SELECT MAX(id) FROM auditoria_execucao").fetchone()[0]
        rows = db.execute("SELECT linha_fisica,linha_original,id_evento,cd_tabela_referencia,cd_procedimento,"
                          "estado,estrutura_divergente FROM auditoria_resultado WHERE execucao_id=? "
                          "ORDER BY linha_fisica", (execution,)).fetchall()
        require([row[0] for row in rows] == [2, 4, 5, 6, 7], "Linha vazia nao deve deslocar numero fisico")
        require(rows[3] == (6, "registro;truncado\r", None, None, None, "REJEITADO", 1), "Quarentena estrutural")
        require(rows[4][4] == "010101012", "Codigo deve preservar zeros a esquerda")
        kinds = db.execute("SELECT DISTINCT tipo FROM auditoria_ocorrencia WHERE resultado_id="
                           "(SELECT id FROM auditoria_resultado WHERE execucao_id=? AND linha_fisica=5)",
                           (execution,)).fetchall()
        require(set(kinds) == {("MOTIVO",), ("ALERTA",)}, "Rejeitado deve preservar tambem seus alertas")
        db.execute("CREATE TRIGGER falha_auditoria BEFORE INSERT ON auditoria_resultado "
                   "WHEN NEW.linha_fisica=4 BEGIN SELECT RAISE(ABORT,'falha simulada'); END")
    # O erro ocorre depois de pelo menos uma gravacao na nova execucao.
    expect_failure([app, positive, CATALOG, database], "Falha de gravacao deve cancelar execucao")
    with closing(sqlite3.connect(database)) as db, db:
        require(db.execute("SELECT COUNT(*) FROM auditoria_execucao").fetchone()[0] == 3, "Execucao parcial permaneceu")
        require(db.execute("SELECT COUNT(*) FROM auditoria_resultado").fetchone()[0] == 25, "Rollback ou historico incorreto")
        db.execute("DROP TRIGGER falha_auditoria")
    check_counts(run([app, historical, CATALOG, database]).stdout, [1, 1, 0, 0, 0])
    with closing(sqlite3.connect(database)) as db, db:
        require(db.execute("SELECT competencia FROM auditoria_execucao ORDER BY id DESC LIMIT 1").fetchone()[0] == "202201",
                "Competencia historica perdida")
    print("OK: CRLF, linhas vazias, estrutura divergente, zeros, rejeitado com alerta e rollback")

    invalid = build / "PR_202501_AMB_DET_cabecalho.csv"
    invalid.write_text("cabecalho_invalido\n1;2\n", encoding="utf-8")
    missing = build / "PR_202501_AMB_DET_ausente.csv"
    for source in [invalid, missing]:
        expect_failure([app, source, CATALOG, database], "Entrada invalida deve falhar")
    with closing(sqlite3.connect(database)) as db, db:
        require(db.execute("SELECT COUNT(*) FROM auditoria_execucao").fetchone()[0] == 4, "Entrada invalida iniciou auditoria")
    expect_failure([app, positive, CATALOG, build / "diretorio_ausente/auditoria.db"], "Banco inacessivel")
    for destination in [positive, CATALOG]:
        before = destination.read_bytes()
        expect_failure([app, positive, CATALOG, destination], "Banco deve ser diferente da entrada")
        require(destination.read_bytes() == before, "Arquivo de entrada foi alterado")
    print("OK: entrada ausente, cabecalho invalido, banco inacessivel e protecao das entradas")
    both_database = build / "auditoria_22_63.db"
    for source, expected in [(positive, [10, 10, 0, 0, 0]), (negative, [10, 9, 0, 1, 0])]:
        check_counts(run([app, source, CATALOG, both_database, GROUPS]).stdout, expected)
    with closing(sqlite3.connect(both_database)) as db, db:
        require(db.execute("SELECT tabela_catalogo,total,validos,alertas,rejeitados FROM auditoria_execucao ORDER BY id").fetchall()
                == [("22,63", 10, 10, 0, 0), ("22,63", 10, 9, 0, 1)], "Resumo/referencias dos dois catalogos")
        require(db.execute("SELECT COUNT(*) FROM auditoria_ocorrencia WHERE tipo='ALERTA'").fetchone()[0] == 0,
                "Grupos da amostra ainda geraram alerta")
        require(db.execute("SELECT DISTINCT cd_procedimento FROM auditoria_resultado WHERE cd_tabela_referencia='63' ORDER BY cd_procedimento").fetchall()
                == [("004",), ("018",)], "Zeros dos grupos perdidos na auditoria")
    unknown_groups = build / "PR_202501_AMB_DET_grupos.csv"
    unknown_groups.write_text(positive.read_text(encoding="utf-8-sig").splitlines()[0] +
        '\n1;PR;2025-01;18;63;1;100;0;;0;0\n2;PR;2025-01;999;63;1;100;0;;0;0\n'
        '3;PR;2025-01;10101012;63;1;100;0;;0;0\n4;PR;2025-01;018;22;1;100;0;;0;0\n', encoding="utf-8")
    check_counts(run([app, unknown_groups, CATALOG, both_database, GROUPS]).stdout, [4, 0, 4, 0, 0])
    with closing(sqlite3.connect(both_database)) as db, db:
        alerts = db.execute("SELECT mensagem FROM auditoria_ocorrencia o JOIN auditoria_resultado r ON r.id=o.resultado_id WHERE r.execucao_id=3").fetchall()
        require(len(alerts) == 4 and all(row[0].startswith("CODIGO_NAO_LOCALIZADO_NO_CATALOGO") for row in alerts),
                "Ausencia do par deve ser alerta com catalogo carregado")
    for groups in [build / "ausente63.csv", CATALOG]:
        expect_failure([app, positive, CATALOG, both_database, groups], "Referencia 63 ausente/incorreta")
    before = GROUPS.read_bytes()
    expect_failure([app, positive, CATALOG, GROUPS, GROUPS], "Banco deve ser diferente do catalogo 63")
    require(GROUPS.read_bytes() == before, "Catalogo 63 foi alterado")
    with closing(sqlite3.connect(both_database)) as db, db:
        require(db.execute("SELECT COUNT(*) FROM auditoria_execucao").fetchone()[0] == 3, "Referencia invalida iniciou execucao")
    print("OK: catalogos 22+63, pares isolados, grupos com zeros, auditoria e falha de referencia")

    # As duas fontes sao escolhidas pelo executavel e usam ponteiro DAOEntrada.
    columns = ("id_evento_atencao_saude", "uf_prestador", "dt_realizacao", "cd_procedimento",
               "cd_tabela_referencia", "qt_item_evento_informado", "vl_item_evento_informado",
               "vl_item_pago_fornecedor", "unidade_medida", "ind_pacote", "ind_tabela_propria")
    def create_input(path, rows):
        with closing(sqlite3.connect(path)) as db, db:
            db.execute("CREATE TABLE registro_tiss (" + ",".join(c + " TEXT" for c in columns) + ")")
            db.executemany("INSERT INTO registro_tiss VALUES (" + ",".join("?" for _ in columns) + ")", rows)

    for source, expected22, expected_both in [
            (positive, [10, 7, 3, 0, 0], [10, 10, 0, 0, 0]),
            (negative, [10, 6, 3, 1, 0], [10, 9, 0, 1, 0])]:
        with source.open(encoding="utf-8-sig", newline="") as stream:
            records = list(csv.reader(stream, delimiter=";"))[1:]
        relational = build / (source.stem + ".db")
        create_input(relational, records)
        before = relational.read_bytes()
        for catalogs, expected in [([CATALOG], expected22), ([CATALOG, GROUPS], expected_both)]:
            output = run([app, "--sqlite", relational, "202501"] + catalogs).stdout
            check_counts(output, expected)
            require("Tempo de processamento:" in output, "Tempo ausente no resumo SQLite")
        require(relational.read_bytes() == before, "Leitura SQLite alterou a entrada")
        if source == positive:
            # Mes valido, mas diferente da competencia: alerta contextual nas dez linhas.
            check_counts(run([app, "--sqlite", relational, "202502", CATALOG, GROUPS]).stdout,
                         [10, 0, 10, 0, 0])
    empty_input = build / "vazio.db"
    create_input(empty_input, [])
    check_counts(run([app, "--sqlite", empty_input, "202501", CATALOG]).stdout, [0, 0, 0, 0, 0])
    absent_input = build / "entrada_ausente.db"
    expect_failure([app, "--sqlite", absent_input, "202501", CATALOG], "Banco de entrada ausente")
    require(not absent_input.exists(), "Entrada ausente foi criada indevidamente")
    invalid_schema = build / "estrutura_invalida.db"
    with closing(sqlite3.connect(invalid_schema)) as db, db:
        db.execute("CREATE TABLE outra_tabela (id TEXT)")
    expect_failure([app, "--sqlite", invalid_schema, "202501", CATALOG], "Tabela de entrada ausente")
    with closing(sqlite3.connect(invalid_schema)) as db, db:
        db.execute("CREATE TABLE registro_tiss (id_evento_atencao_saude TEXT)")
    expect_failure([app, "--sqlite", invalid_schema, "202501", CATALOG], "Colunas de entrada ausentes")
    for competence in ["2025", "202500", "202513", "000001", "2025AA"]:
        expect_failure([app, "--sqlite", empty_input, competence, CATALOG], "Competencia invalida")
    expect_failure([app, "--sqlite", empty_input, "202501"], "Argumentos SQLite incompletos")
    print("OK: entrada SQLite por DAO no executavel, paridade CSV, contexto, tempo e falhas sem criar banco")


print("OK: limpeza do diretorio temporario concluida")
