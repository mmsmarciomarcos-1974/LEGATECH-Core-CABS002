#!/usr/bin/env python3
"""Normalize the ANS TUSS 20 CSV export to LEGATECH CatalogoTuss schema."""
import argparse
import csv
import hashlib
import re
from collections import Counter
from datetime import datetime
from pathlib import Path

HEADER = [
    "versao_catalogo", "cd_tabela_referencia", "cd_procedimento", "termo",
    "descricao_detalhada", "inicio_vigencia", "fim_vigencia", "fim_implantacao",
]
VERSION = "202501"
TABLE = "20"
SPECIAL_SPLIT_PRESENTATION = {
    "90287991", "90447107", "90447115", "90447158", "90468023", "90553080"
}
SPECIAL_MALFORMED_QUOTE = "90518721"

def sha256(path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def date_iso(value, code, field):
    value = value.strip()
    if not value:
        return ""
    if re.fullmatch(r"\d{4}-\d{2}-\d{2}", value):
        datetime.strptime(value, "%Y-%m-%d")
        return value
    # One ANS row in the spreadsheet export was emitted as MM-DD-YY.
    if code == SPECIAL_MALFORMED_QUOTE and re.fullmatch(r"\d{2}-\d{2}-\d{2}", value):
        return datetime.strptime(value, "%m-%d-%y").strftime("%Y-%m-%d")
    raise ValueError(f"Data inesperada em {field}, código {code}: {value!r}")

def clean_source_row(row, logical_line):
    if not row or not row[0].strip().isdigit():
        return None
    original = list(row)
    code = row[0].strip()
    if len(row) == 10 and code == "90553080" and row[-1] == "":
        row = row[:-1]
    elif len(row) == 9 and row[-1] == "" and code not in SPECIAL_SPLIT_PRESENTATION | {SPECIAL_MALFORMED_QUOTE}:
        row = row[:-1]
    if code == "90301145" and len(row) == 9 and row[3] == "":
        row.pop(3)  # blank stray delimiter between presentation and laboratory
    elif code in SPECIAL_SPLIT_PRESENTATION and len(row) == 9:
        row[2] = row[2] + ";" + row[3]
        row.pop(3)  # presentation text was split by an unquoted semicolon
    elif code == SPECIAL_MALFORMED_QUOTE and len(row) == 9:
        left = row[2].lstrip('"')
        right = row[3].rstrip('"')
        row[2] = left + ";" + right
        row.pop(3)  # broken quoting split one presentation into two fields
    if len(row) != 8:
        raise ValueError(f"Linha lógica {logical_line}, código {code}: esperados 8 campos após reparo, vieram {len(row)}: {original!r}")
    return code, row

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("origem", type=Path)
    ap.add_argument("destino", type=Path)
    args = ap.parse_args()
    src, dst = args.origem, args.destino
    codes = []
    records = []
    repairs = Counter()
    with src.open("r", encoding="utf-8-sig", newline="") as f:
        for logical_line, row in enumerate(csv.reader(f, delimiter=";"), start=1):
            if not row or not row[0].strip().isdigit():
                continue
            code = row[0].strip()
            n = len(row)
            normalized = clean_source_row(row, logical_line)
            if n == 10:
                repairs["campo_vazio_final_removido"] += 1
            elif n == 9 and row[-1] == "" and code not in SPECIAL_SPLIT_PRESENTATION | {SPECIAL_MALFORMED_QUOTE}:
                repairs["campo_vazio_final_removido"] += 1
            if code == "90301145": repairs["delimitador_vazio_entre_apresentacao_laboratorio"] += 1
            if code in SPECIAL_SPLIT_PRESENTATION: repairs["separador_interno_da_apresentacao_reunido"] += 1
            if code == SPECIAL_MALFORMED_QUOTE: repairs["aspas_quebradas_apresentacao_reconstruida"] += 1
            if normalized is None:
                continue
            code, row = normalized
            _, term, presentation, lab, start, end, implant, anvisa = row
            if not re.fullmatch(r"\d{8}", code):
                raise ValueError(f"Código inválido na linha {logical_line}: {code!r}")
            start = date_iso(start, code, "inicio_vigencia")
            end = date_iso(end, code, "fim_vigencia")
            implant = date_iso(implant, code, "fim_implantacao")
            detail = f"Apresentação: {presentation}; Laboratório: {lab}; Registro ANVISA: {anvisa}"
            records.append([VERSION, TABLE, code, term, detail, start, end, implant])
            codes.append(code)
    duplicate_count = sum(n - 1 for n in Counter(codes).values() if n > 1)
    if duplicate_count:
        raise ValueError(f"Foram encontrados {duplicate_count} códigos duplicados")
    if len(records) != 41416:
        raise ValueError(f"Quantidade de registros inesperada: {len(records)} (esperado 41416)")
    dst.parent.mkdir(parents=True, exist_ok=True)
    with dst.open("w", encoding="utf-8", newline="") as f:
        writer = csv.writer(f, delimiter=";", lineterminator="\n", quoting=csv.QUOTE_MINIMAL)
        writer.writerow(HEADER)
        writer.writerows(records)
    # Read back the produced file and assert schema, row count, keys, and dates.
    with dst.open("r", encoding="utf-8", newline="") as f:
        check = list(csv.reader(f, delimiter=";"))
    if check[0] != HEADER or len(check) != len(records) + 1 or any(len(r) != 8 for r in check[1:]):
        raise ValueError("Falha na validação estrutural da saída")
    if len({r[2] for r in check[1:]}) != len(records) or any(r[0] != VERSION or r[1] != TABLE for r in check[1:]):
        raise ValueError("Falha na validação de códigos ou tabela/versão")
    print(f"Registros: {len(records)}")
    print(f"Códigos únicos: {len(set(codes))}; duplicados: {duplicate_count}")
    print(f"Reparos: {dict(repairs)}")
    print(f"SHA-256 origem: {sha256(src)}")
    print(f"SHA-256 saída:  {sha256(dst)}")

if __name__ == "__main__":
    main()
