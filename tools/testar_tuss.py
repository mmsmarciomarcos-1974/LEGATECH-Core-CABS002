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
TUSS19_DIR = ROOT / "data/reference/tuss19_202501"
TUSS60_DIR = ROOT / "data/reference/tuss60_202501"
TUSS60_CATALOG = TUSS60_DIR / "tuss60_202501.csv"


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

manifest19 = json.loads((TUSS19_DIR / "manifest.json").read_text(encoding="utf-8"))
require(manifest19["catalog_version"] == "202501" and manifest19["table"] == "19"
        and manifest19["records"] == 1292182 and manifest19["unique_codes"] == 1292182
        and manifest19["source_zip_sha256"] == "e4f7ee54d0e06c6c7f5a3374fb8cd8bedd67069e6c44f1d05b03a89298f30275",
        "Identificacao/quantidade do snapshot TUSS 19 divergente")
total19 = 0
for part in manifest19["parts"]:
    path = TUSS19_DIR / part["file"]
    # Canoniza CRLF do checkout Windows para o LF fixado no manifesto.
    payload = path.read_bytes().replace(b"\r\n", b"\n")
    require(hashlib.sha256(payload).hexdigest() == part["sha256"], "Hash de parte TUSS 19 divergente: " + part["file"])
    with path.open(encoding="utf-8", newline="") as source:
        rows = list(csv.reader(source, delimiter=";"))
    require(rows[0] == ["cd_procedimento", "inicio_vigencia", "fim_vigencia"], "Cabecalho compacto TUSS 19 divergente")
    require(len(rows) - 1 == part["records"], "Quantidade de registros TUSS 19 divergente: " + part["file"])
    total19 += len(rows) - 1
require(total19 == manifest19["records"], "Total das partes TUSS 19 divergente")

manifest60 = json.loads((TUSS60_DIR / "manifest.json").read_text(encoding="utf-8"))
require(manifest60["catalog_version"] == "202501" and manifest60["table"] == "60"
        and manifest60["records"] == 61 and manifest60["unique_codes"] == 61
        and manifest60["code_length"] == 3
        and manifest60["source_zip_sha256"] == manifest19["source_zip_sha256"],
        "Identificacao/quantidade do snapshot TUSS 60 divergente")
payload60 = TUSS60_CATALOG.read_bytes().replace(b"\r\n", b"\n")
require(hashlib.sha256(payload60).hexdigest() == manifest60["output_sha256"],
        "Hash do catalogo TUSS 60 divergente")
with TUSS60_CATALOG.open(encoding="utf-8", newline="") as source:
    rows60 = list(csv.DictReader(source, delimiter=";"))
require(len(rows60) == 61 and len({row["cd_procedimento"] for row in rows60}) == 61
        and all(len(row["cd_procedimento"]) == 3 and row["cd_procedimento"].isdigit()
                for row in rows60),
        "Codigos do snapshot TUSS 60 divergentes")

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
    print(run([tests, CATALOG, GROUPS, TUSS19_DIR, TUSS60_CATALOG]).stdout.strip())
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
        # A TUSS 20 e carregada automaticamente junto com a TUSS 22.
        require(
            executions[0][1:9] == ("202501", "202501", "22,20,60,19", 10, 7, 3, 0, 0),
            f"Metadados/resumo positivo divergentes: {executions[0][1:9]}"
        )
        require(
            executions[1][1:9] == ("202501", "202501", "22,20,60,19", 10, 6, 3, 1, 0),
            f"Metadados/resumo negativo divergentes: {executions[1][1:9]}"
        )
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
                == [("22,20,60,63,19", 10, 10, 0, 0), ("22,20,60,63,19", 10, 9, 0, 1)], "Resumo/referencias TUSS 19, 20, 22, 60 e 63")
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
    # As novas regras devem produzir as mesmas rejeicoes nas duas fontes.
    limites = build / "PR_202501_AMB_DET_limites.csv"
    with positive.open(encoding="utf-8-sig", newline="") as stream:
        cabecalho_limites = next(csv.reader(stream, delimiter=";"))
    linhas_limites = [
        ["1","PR","2025-01","10101012","22","999999","100,50","0","","0","0"],
        ["2","PR","2025-01","10101012","22",".5","100","0","","0","0"],
        ["3","PR","2025-01","10101012","22","1","100.123","0","","0","0"],
        ["9"*31,"PR","2025-01","10101012","22","1","100","0","","0","0"],
        ["5","PR","2025-01","10101012","22","1","-0.01","0","","0","0"],
        ["6","PR","2025-01","10101012","22","1","0","-0.01","","0","0"],
        ["7","PR","2025-01","10101012","22","1","0","-0.001","","0","0"],
        ["8","PR","2025-01","10101012","22","1","0","0","","0","0"],
        ["9","PR","2025-01","10101012","22","1","","0","","0","0"],
        ["10","PR","2025-01","10101012","22","1","","","","1","0"]]
    with limites.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, delimiter=";", lineterminator="\n")
        writer.writerow(cabecalho_limites)
        writer.writerows(linhas_limites)
    banco_limites = build / "limites.db"
    create_input(banco_limites, linhas_limites)
    check_counts(run([app, limites, CATALOG]).stdout, [10, 3, 1, 6, 0])
    check_counts(run([app, "--sqlite", banco_limites, "202501", CATALOG]).stdout, [10, 3, 1, 6, 0])
    print("OK: limites e precisao aplicados igualmente em CSV e SQLite")

    unidades = build / "PR_202501_AMB_DET_unidades.csv"
    linhas_unidades = [
        ["1", "PR", "2025-01", "A1", "00", "1", "100", "0", "036", "0", "1"],
        ["2", "PR", "2025-01", "A1", "00", "1", "100", "0", "", "0", "1"],
        ["3", "PR", "2025-01", "A1", "00", "1", "100", "0", "999", "0", "1"],
        ["4", "PR", "2025-01", "A1", "00", "1", "100", "0", "36", "0", "1"],
    ]
    with unidades.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, delimiter=";", lineterminator="\n")
        writer.writerow(cabecalho_limites)
        writer.writerows(linhas_unidades)
    banco_unidades = build / "unidades.db"
    create_input(banco_unidades, linhas_unidades)
    esperado_unidades = [4, 2, 0, 2, 0]
    check_counts(run([app, unidades, CATALOG]).stdout, esperado_unidades)
    check_counts(run([app, "--sqlite", banco_unidades, "202501", CATALOG]).stdout,
                 esperado_unidades)
    print("OK: TUSS 60 — dominio, zeros, campo condicional e paridade CSV/SQLite")

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


    # RF10: CSV e SQLite devem preservar os mesmos registros e ocorrencias.
    def check_export(arguments, destination, expected):
        check_counts(run([app] + arguments + ["--export", destination]).stdout, expected)
        require(destination.is_dir(), "Exportacao nao publicada")
        rows = []
        for name in ["aceitos.csv", "rejeitados.csv"]:
            with (destination / name).open(encoding="utf-8", newline="") as stream:
                current = list(csv.DictReader(stream, delimiter=";"))
            require(all((r["ESTADO"] == "REJEITADO") == (name == "rejeitados.csv") for r in current),
                    "Classificacao no arquivo CSV incorreto")
            rows.extend(current)
        require(len(rows) == expected[0], "Quantidade CSV divergente")
        with closing(sqlite3.connect(destination / "resultados.db")) as db:
            require(db.execute("SELECT total,validos,alertas,rejeitados,estrutura_divergente FROM auditoria_execucao").fetchone()
                    == tuple(expected), "Resumo SQLite exportado divergente")
            for r in rows:
                saved = db.execute("SELECT id,estado,linha_original,id_evento,cd_tabela_referencia,cd_procedimento "
                                   "FROM auditoria_resultado WHERE linha_fisica=?", (int(r["LINHA_ORIGEM"]),)).fetchone()
                require(saved is not None and saved[1:3] == (r["ESTADO"], r["LINHA_ORIGINAL"]),
                        "Estado/linha original divergente entre CSV e SQLite")
                require(tuple(x or "" for x in saved[3:]) ==
                        (r["ID_EVENTO_ATENCAO_SAUDE"], r["CD_TABELA_REFERENCIA"], r["CD_PROCEDIMENTO"]),
                        "Codigos/zeros divergentes na exportacao")
                for label, kind in [("MOTIVOS_JSON", "MOTIVO"), ("ALERTAS_JSON", "ALERTA")]:
                    messages = [x[0] for x in db.execute("SELECT mensagem FROM auditoria_ocorrencia "
                        "WHERE resultado_id=? AND tipo=? ORDER BY ordem", (saved[0], kind))]
                    require(json.loads(r[label]) == messages, "Ocorrencias perdidas ou alteradas")
        require(set(x.name for x in destination.iterdir()) ==
                {"aceitos.csv", "rejeitados.csv", "contexto.csv", "resultados.db"}, "Arquivos exportados inesperados")
        return rows

    # RF05: paridade dos motivos/alertas persistidos, alem dos totais.
    datas = build / "PR_203801_AMB_DET_datas.csv"
    linhas_datas = [[str(i), "PR", mes, "A1", "00", "1", "100", "0", "", "0", "1"]
                    for i, mes in enumerate(["2038-01", "2038-02", "2037-12", "0000-01", "2038-13"], 1)]
    with datas.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, delimiter=";", lineterminator="\n", quoting=csv.QUOTE_ALL)
        writer.writerow(cabecalho_limites)
        writer.writerows(linhas_datas)
    banco_datas = build / "datas.db"
    create_input(banco_datas, linhas_datas)
    resultados_datas = []
    for tag, args in [("csv", [datas, CATALOG]), ("sqlite", ["--sqlite", banco_datas, "203801", CATALOG])]:
        destino = build / ("datas_" + tag)
        linhas = check_export(args, destino, [5, 0, 3, 2, 0])
        resultados_datas.append(linhas)
    require(resultados_datas[0] == resultados_datas[1], "Datas: resultados CSV/SQLite divergentes")
    print("OK: datas, rejeicoes e alertas Y2K38 exportados igualmente em CSV e SQLite")

    for source, expected in [(positive, [10, 7, 3, 0, 0]), (negative, [10, 6, 3, 1, 0])]:
        before = source.read_bytes()
        check_export([source, CATALOG], build / (source.stem + "_export"), expected)
        require(source.read_bytes() == before, "Exportacao alterou CSV original")
    check_export([negative, CATALOG, GROUPS], build / "export_22_63", [10, 9, 0, 1, 0])
    check_export([mixed, CATALOG], build / "export_misto", [5, 1, 2, 2, 1])
    check_export([historical, CATALOG], build / "export_historico", [1, 1, 0, 0, 0])
    relational = build / (negative.stem + ".db")
    before = relational.read_bytes()
    check_export(["--sqlite", relational, "202501", CATALOG], build / "export_sqlite", [10, 6, 3, 1, 0])
    check_export(["--sqlite", empty_input, "202501", CATALOG], build / "export_vazio", [0, 0, 0, 0, 0])
    require(relational.read_bytes() == before, "Exportacao alterou banco original")
    # Texto com delimitador, aspas, quebra de linha e NUL deve fazer round-trip CSV.
    special_input = build / "especial.db"
    special_rows = [["id;\"\n\x00fim", "PR", "2025-01", "10101012", "22", "1", "100", "0", "", "0", "0"]]
    create_input(special_input, special_rows)
    check_export(["--sqlite", special_input, "202501", CATALOG], build / "export_especial", [1, 0, 0, 1, 0])
    existing = build / "export_sqlite"
    existing_before = {f.name: f.read_bytes() for f in existing.iterdir()}
    expect_failure([app, positive, CATALOG, "--export", existing], "Pasta existente nao pode ser sobrescrita")
    require({f.name: f.read_bytes() for f in existing.iterdir()} == existing_before, "Exportacao anterior foi alterada")
    for arguments in [[missing, CATALOG], [invalid, CATALOG],
                      ["--sqlite", invalid_schema, "202501", CATALOG]]:
        destination = build / "export_falha"
        expect_failure([app] + arguments + ["--export", destination], "Falha nao publica resultados")
        require(not destination.exists(), "Resultados parciais publicados")
        require(not list(build.glob("export_falha.tmp-*")), "Pasta temporaria nao removida")
    expect_failure([app, positive, CATALOG, "--export", build / "pai_ausente/export"], "Pai de exportacao ausente")
    print("OK: saidas CSV/SQLite por DAO, paridade de estados/ocorrencias, historico, zeros e protecao de resultados")


print("OK: limpeza do diretorio temporario concluida")
