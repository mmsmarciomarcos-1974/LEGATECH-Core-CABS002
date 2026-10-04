"""Compila em C++23 e executa regressao sem modificar massas ou bancos originais.

Uso na raiz do projeto: python tools/testar_tuss.py
Requer gcc e g++ no PATH (MinGW no Windows).
"""
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CORE = ROOT / "LEGATECH_Core"
CATALOG = ROOT / "data/reference/tuss22_202501.csv"


def run(args):
    return subprocess.run([str(a) for a in args], check=True, capture_output=True, text=True)


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
    print(run([tests, CATALOG]).stdout.strip())
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
