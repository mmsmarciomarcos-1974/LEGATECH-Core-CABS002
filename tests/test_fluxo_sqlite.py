from pathlib import Path
import csv
import os
import shutil
import sqlite3
import subprocess
import sys


def exigir(condicao, mensagem):
    if not condicao:
        raise AssertionError(f"Teste falhou: {mensagem}")


ROOT = Path(__file__).resolve().parents[1]
CORE = ROOT / "LEGATECH_Core"

EXE = CORE / "bin" / "Release" / (
    "LEGATECH_Core.exe" if os.name == "nt" else "LEGATECH_Core"
)

ENTRADA_SQLITE = ROOT / "data" / "legatech.db"
TUSS22 = ROOT / "data" / "reference" / "tuss22_202501.csv"
TUSS63 = ROOT / "data" / "reference" / "tuss63_202501.csv"
TUSS20 = ROOT / "data" / "reference" / "tuss20_202501.csv"

SAIDA = ROOT / "data" / "output" / "saida_regressao_sqlite_auto"

ACEITOS = SAIDA / "aceitos.csv"
REJEITADOS = SAIDA / "rejeitados.csv"
CONTEXTO = SAIDA / "contexto.csv"
BANCO = SAIDA / "resultados.db"


def contar_csv(caminho):
    with caminho.open("r", encoding="utf-8-sig", newline="") as arquivo:
        linhas = list(csv.reader(arquivo, delimiter=";"))

    return max(0, len(linhas) - 1)


def main():
    print("=== LEGATECH Core - regressao da entrada SQLite ===")

    exigir(EXE.exists(), f"Executavel nao encontrado: {EXE}")
    exigir(
        ENTRADA_SQLITE.exists(),
        f"Banco de entrada nao encontrado: {ENTRADA_SQLITE}"
    )
    exigir(TUSS22.exists(), f"TUSS 22 nao encontrada: {TUSS22}")
    exigir(TUSS63.exists(), f"TUSS 63 nao encontrada: {TUSS63}")
    exigir(TUSS20.exists(), f"TUSS 20 nao encontrada: {TUSS20}")

    # -------------------------------------------------------
    # Confere a massa SQLite antes da execução
    # -------------------------------------------------------

    with sqlite3.connect(ENTRADA_SQLITE) as banco_entrada:

        tabela = banco_entrada.execute(
            """
            select count(*)
            from sqlite_master
            where type='table'
              and name='registro_tiss'
            """
        ).fetchone()[0]

        exigir(
            tabela == 1,
            "tabela registro_tiss nao encontrada no banco de entrada"
        )

        total_entrada = banco_entrada.execute(
            "select count(*) from registro_tiss"
        ).fetchone()[0]

        exigir(
            total_entrada == 10,
            f"massa SQLite deveria possuir 10 registros, encontrados {total_entrada}"
        )

    if SAIDA.exists():
        shutil.rmtree(SAIDA)

    ambiente = os.environ.copy()

    # Runtime do MinGW utilizado pelo Code::Blocks no Windows.
    if os.name == "nt":
        mingw = Path(
            ambiente.get(
                "LEGATECH_MINGW_BIN",
                r"C:\Program Files\CodeBlocks\MinGW\bin"
            )
        )

        if mingw.exists():
            ambiente["PATH"] = (
                str(mingw)
                + os.pathsep
                + ambiente.get("PATH", "")
            )

    comando = [
        str(EXE),
        "--sqlite",
        str(ENTRADA_SQLITE),
        "202501",
        str(TUSS22),
        str(TUSS63),
        "--export",
        str(SAIDA),
    ]

    print("\nExecutando Core em modo SQLite...")

    # Importante:
    # o Core atualmente procura a TUSS 20 por caminho relativo:
    # ../data/reference/tuss20_202501.csv
    #
    # Por isso cwd=CORE reproduz o ambiente já validado manualmente.
    processo = subprocess.run(
        comando,
        cwd=CORE,
        env=ambiente,
        check=False
    )

    exigir(
        processo.returncode == 0,
        f"Core retornou RC={processo.returncode}"
    )

    # -------------------------------------------------------
    # Artefatos esperados
    # -------------------------------------------------------

    exigir(SAIDA.exists(), "pasta de exportacao nao foi criada")
    exigir(ACEITOS.exists(), "aceitos.csv nao foi criado")
    exigir(REJEITADOS.exists(), "rejeitados.csv nao foi criado")
    exigir(CONTEXTO.exists(), "contexto.csv nao foi criado")
    exigir(BANCO.exists(), "resultados.db nao foi criado")

    # 10 registros válidos
    exigir(
        contar_csv(ACEITOS) == 10,
        "aceitos.csv deveria conter 10 registros"
    )

    exigir(
        contar_csv(REJEITADOS) == 0,
        "rejeitados.csv deveria conter 0 registros"
    )

    exigir(
        contar_csv(CONTEXTO) == 1,
        "contexto.csv deveria conter 1 execucao"
    )

    # -------------------------------------------------------
    # Auditoria SQLite
    # -------------------------------------------------------

    with sqlite3.connect(BANCO) as banco:

        execucao = banco.execute(
            """
            select
                total,
                validos,
                alertas,
                rejeitados,
                estrutura_divergente
            from auditoria_execucao
            """
        ).fetchone()

        exigir(
            execucao is not None,
            "auditoria_execucao nao encontrada"
        )

        exigir(
            execucao == (10, 10, 0, 0, 0),
            f"totais inesperados: {execucao}"
        )

        estados = dict(
            banco.execute(
                """
                select estado, count(*)
                from auditoria_resultado
                group by estado
                """
            ).fetchall()
        )

        exigir(
            estados.get("VALIDO") == 10,
            "esperados 10 registros VALIDOS"
        )

        exigir(
            estados.get("VALIDO_COM_ALERTA", 0) == 0,
            "nao eram esperados registros VALIDOS_COM_ALERTA"
        )

        exigir(
            estados.get("REJEITADO", 0) == 0,
            "nao eram esperados registros REJEITADOS"
        )

        ocorrencias = banco.execute(
            """
            select count(*)
            from auditoria_ocorrencia
            """
        ).fetchone()[0]

        exigir(
            ocorrencias == 0,
            f"nao eram esperadas ocorrencias, encontradas {ocorrencias}"
        )

    print()
    print("==========================================")
    print("PASS - regressao da entrada SQLite validada")
    print("==========================================")
    print("Total:                 10")
    print("VALIDO:                10")
    print("VALIDO_COM_ALERTA:      0")
    print("REJEITADO:              0")
    print("Estrutura divergente:   0")
    print("CSV de auditoria:       OK")
    print("SQLite de auditoria:    OK")


if __name__ == "__main__":
    try:
        main()
    except Exception as erro:
        print()
        print(f"FAIL - {erro}", file=sys.stderr)
        sys.exit(1)