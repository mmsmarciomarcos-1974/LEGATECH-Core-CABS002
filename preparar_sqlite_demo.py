"""Create or validate a small SQLite input from the controlled TISS CSV sample."""

import csv
import sqlite3
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent
CSV_PATH = ROOT / "data" / "sample" / "PR_202501_AMB_DET_sample.csv"
DB_PATH = ROOT / "data" / "sample" / "entrada_sqlite_demo.db"
EXPECTED_COLUMNS = [
    "id_evento_atencao_saude",
    "uf_prestador",
    "dt_realizacao",
    "cd_procedimento",
    "cd_tabela_referencia",
    "qt_item_evento_informado",
    "vl_item_evento_informado",
    "vl_item_pago_fornecedor",
    "unidade_medida",
    "ind_pacote",
    "ind_tabela_propria",
]


def main() -> int:
    if not CSV_PATH.is_file():
        print(f"CSV de amostra nao encontrado: {CSV_PATH}", file=sys.stderr)
        return 1

    with CSV_PATH.open("r", encoding="utf-8-sig", newline="") as source:
        reader = csv.reader(source, delimiter=";")
        try:
            header = next(reader)
        except StopIteration:
            print("CSV de amostra vazio.", file=sys.stderr)
            return 1
        columns = [name.strip().lower() for name in header]
        if columns != EXPECTED_COLUMNS:
            print("Cabecalho do CSV de amostra inesperado.", file=sys.stderr)
            return 1
        rows = list(reader)

    if not rows or any(len(row) != len(EXPECTED_COLUMNS) for row in rows):
        print(
            "CSV vazio ou com registro fora da estrutura de 11 colunas.",
            file=sys.stderr,
        )
        return 1

    if DB_PATH.exists():
        try:
            with sqlite3.connect(DB_PATH) as connection:
                columns = [
                    row[1].lower()
                    for row in connection.execute("PRAGMA table_info(registro_tiss)")
                ]
                count = connection.execute(
                    "SELECT COUNT(*) FROM registro_tiss"
                ).fetchone()[0]
                stored_rows = connection.execute(
                    "SELECT " + ", ".join(EXPECTED_COLUMNS)
                    + " FROM registro_tiss ORDER BY rowid"
                ).fetchall()
            expected_rows = [tuple(row) for row in rows]
            if columns != EXPECTED_COLUMNS or stored_rows != expected_rows:
                print(
                    "Base SQLite existente difere da amostra esperada; preservada.",
                    file=sys.stderr,
                )
                return 1
            print(f"Base SQLite existente validada: {DB_PATH}")
            print(f"Registros carregados: {count}")
            return 0
        except sqlite3.Error as error:
            print(
                f"Base SQLite existente invalida; preservada: {error}",
                file=sys.stderr,
            )
            return 1

    try:
        with sqlite3.connect(DB_PATH) as connection:
            definitions = ", ".join(f'"{name}" TEXT' for name in EXPECTED_COLUMNS)
            connection.execute(f"CREATE TABLE registro_tiss ({definitions})")
            placeholders = ", ".join("?" for _ in EXPECTED_COLUMNS)
            connection.executemany(
                f"INSERT INTO registro_tiss VALUES ({placeholders})", rows
            )
    except (OSError, sqlite3.Error) as error:
        DB_PATH.unlink(missing_ok=True)
        print(f"Falha ao criar a base SQLite: {error}", file=sys.stderr)
        return 1

    print(f"Base SQLite criada: {DB_PATH}")
    print(f"Registros carregados: {len(rows)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
