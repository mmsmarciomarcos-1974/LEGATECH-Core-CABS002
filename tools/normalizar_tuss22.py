"""Prepara catálogo externo; não altera nem valida a massa AMB_DET.

Uso: python3 normalizar_tuss22.py origem.csv diretorio_saida
"""
import csv
import datetime as dt
import hashlib
import json
import pathlib
import sys

HEADER = ["Código do Termo", "Termo", "Descrição Detalhada",
          "Data de início de vigência", "Data de fim de vigência",
          "Data de fim de implantação"]


def iso(value):
    return dt.datetime.strptime(value, "%d/%m/%Y").date().isoformat() if value else ""


def main(source, destination):
    source, destination = pathlib.Path(source), pathlib.Path(destination)
    raw = source.read_bytes()
    # newline='' permite campos CSV citados, inclusive quebras de linha internas.
    with source.open(encoding="utf-8-sig", newline="") as stream:
        reader = csv.reader(stream, delimiter=",")
        for row in reader:
            if row == HEADER:
                break
        else:
            raise ValueError("Cabeçalho TUSS 22 esperado não encontrado")
        records, seen = [], set()
        for row in reader:
            if not any(row):
                continue
            if len(row) != 6:
                raise ValueError(f"Linha CSV {reader.line_num}: esperado 6 campos")
            code, term, description, start, end, implementation = row
            if len(code) != 8 or not code.isascii() or not code.isdigit():
                raise ValueError(f"Código TUSS 22 inválido: {code!r}")
            if code in seen:
                raise ValueError(f"Código duplicado: {code}; revisar intervalos antes de importar")
            seen.add(code)
            start, end, implementation = iso(start), iso(end), iso(implementation)
            if not start or (end and end < start):
                raise ValueError(f"Intervalo inválido: {code}")
            records.append(["202501", "22", code, term, description, start, end, implementation])
    if not records:
        raise ValueError("Catálogo vazio")
    destination.mkdir(parents=True, exist_ok=True)
    catalog = destination / "tuss22_202501.csv"
    with catalog.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, delimiter=";", lineterminator="\n")
        writer.writerow(["versao_catalogo", "cd_tabela_referencia", "cd_procedimento",
                         "termo", "descricao_detalhada", "inicio_vigencia",
                         "fim_vigencia", "fim_implantacao"])
        writer.writerows(records)
    manifest = {"source_filename": source.name,
                "source_sha256": hashlib.sha256(raw).hexdigest(),
                "catalog_sha256": hashlib.sha256(catalog.read_bytes()).hexdigest(),
                "catalog_version": "202501", "table": "22", "records": len(records),
                "encoding": "UTF-8", "delimiter": ";", "date_format": "YYYY-MM-DD",
                "scope": "Snapshot fornecido; não comprova cobertura histórica completa"}
    (destination / "manifest.json").write_text(json.dumps(manifest, ensure_ascii=False, indent=2)+"\n", encoding="utf-8")
    print(json.dumps(manifest, ensure_ascii=False))


if __name__ == "__main__":
    main(*sys.argv[1:])
