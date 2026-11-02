from pathlib import Path
from contextlib import closing
import csv
import json
import os
import shutil
import sqlite3
import subprocess
import sys
import tempfile


def exigir(condicao, mensagem):
    if not condicao:
        raise AssertionError(f"Teste falhou: {mensagem}")


ROOT = Path(__file__).resolve().parents[1]
CORE = ROOT / "LEGATECH_Core"

EXE = CORE / "bin" / "Release" / (
    "LEGATECH_Core.exe" if os.name == "nt" else "LEGATECH_Core"
)

ENTRADA_ORIGINAL = ROOT / "data" / "legatech.db"
TUSS22 = ROOT / "data" / "reference" / "tuss22_202501.csv"
TUSS63 = ROOT / "data" / "reference" / "tuss63_202501.csv"
TUSS20 = ROOT / "data" / "reference" / "tuss20_202501.csv"
PASTA_TEMPORARIA = ROOT / "data" / "output"


def ler_csv(caminho):
    with caminho.open("r", encoding="utf-8-sig", newline="") as arquivo:
        return list(csv.DictReader(arquivo, delimiter=";"))


def main():
    print("=== LEGATECH Core - teste SQLite de duplicidade ===")

    exigir(EXE.exists(), f"Executavel nao encontrado: {EXE}")
    exigir(
        ENTRADA_ORIGINAL.exists(),
        f"Banco original nao encontrado: {ENTRADA_ORIGINAL}"
    )
    exigir(TUSS22.exists(), f"TUSS 22 nao encontrada: {TUSS22}")
    exigir(TUSS63.exists(), f"TUSS 63 nao encontrada: {TUSS63}")
    exigir(TUSS20.exists(), f"TUSS 20 nao encontrada: {TUSS20}")

    PASTA_TEMPORARIA.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory(
        prefix="teste_sqlite_duplicidade_",
        dir=PASTA_TEMPORARIA
    ) as pasta:
        temporaria = Path(pasta)
        entrada_teste = temporaria / "legatech_duplicidade.db"
        saida = temporaria / "saida"

        shutil.copy2(ENTRADA_ORIGINAL, entrada_teste)

        with closing(sqlite3.connect(entrada_teste)) as banco:
            total_original = banco.execute(
                "select count(*) from registro_tiss"
            ).fetchone()[0]

            exigir(
                total_original == 10,
                f"esperados 10 registros originais, encontrados {total_original}"
            )

            banco.execute(
                "insert into registro_tiss select * from registro_tiss limit 1"
            )
            banco.commit()

            total_com_duplicidade = banco.execute(
                "select count(*) from registro_tiss"
            ).fetchone()[0]

            exigir(
                total_com_duplicidade == 11,
                f"esperados 11 registros na copia, encontrados {total_com_duplicidade}"
            )

        ambiente = os.environ.copy()

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
            str(entrada_teste),
            "202501",
            str(TUSS22),
            str(TUSS63),
            "--export",
            str(saida),
        ]

        processo = subprocess.run(
            comando,
            cwd=CORE,
            env=ambiente,
            text=True,
            capture_output=True,
            check=False
        )

        print(processo.stdout, end="")
        if processo.stderr:
            print(processo.stderr, file=sys.stderr, end="")

        exigir(
            processo.returncode == 0,
            f"Core retornou RC={processo.returncode}"
        )

        aceitos = saida / "aceitos.csv"
        rejeitados = saida / "rejeitados.csv"

        exigir(aceitos.exists(), "aceitos.csv nao foi criado")
        exigir(rejeitados.exists(), "rejeitados.csv nao foi criado")

        registros_aceitos = ler_csv(aceitos)
        registros_rejeitados = ler_csv(rejeitados)

        exigir(
            len(registros_aceitos) == 10,
            f"esperados 10 aceitos, encontrados {len(registros_aceitos)}"
        )
        exigir(
            len(registros_rejeitados) == 1,
            f"esperado 1 rejeitado, encontrados {len(registros_rejeitados)}"
        )

        rejeitado = registros_rejeitados[0]

        exigir(
            rejeitado["ESTADO"] == "REJEITADO",
            f"estado inesperado para duplicado: {rejeitado['ESTADO']}"
        )

        motivos = json.loads(rejeitado["MOTIVOS_JSON"])

        exigir(
            "DUPLICIDADE: registro repetido (11 campos identicos)" in motivos,
            f"motivo de duplicidade nao encontrado: {motivos}"
        )

    print()
    print("==========================================")
    print("PASS - duplicidade SQLite rejeitada")
    print("==========================================")
    print("Registros processados: 11")
    print("VALIDO:                10")
    print("REJEITADO:             1")
    print("Motivo DUPLICIDADE:    OK")
    print("Banco original:        preservado")


if __name__ == "__main__":
    try:
        main()
    except Exception as erro:
        print()
        print(f"FAIL - {erro}", file=sys.stderr)
        sys.exit(1)