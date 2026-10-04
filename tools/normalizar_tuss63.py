"""Extrai somente Tab 63 do XLSX oficial Demais terminologias, versao 202501.

Uso: python tools/normalizar_tuss63.py origem.xlsx diretorio_saida
Biblioteca padrao; nao instala dependencias nem altera a massa AMB_DET.
"""
import csv
import datetime as dt
import hashlib
import io
import json
from pathlib import Path
import posixpath
import sys
import xml.etree.ElementTree as ET
import zipfile

NS = {"m": "http://schemas.openxmlformats.org/spreadsheetml/2006/main"}
RID = "{http://schemas.openxmlformats.org/officeDocument/2006/relationships}id"
HEADER = ["Código", "Grupo", "Data de início de vigência",
          "Data de fim de vigência", "Data de fim de implantação"]
OUTPUT_HEADER = ["versao_catalogo", "cd_tabela_referencia", "cd_procedimento",
                 "termo", "descricao_detalhada", "inicio_vigencia",
                 "fim_vigencia", "fim_implantacao"]
SOURCE_URL = "https://www.ans.gov.br/arquivos/extras/tiss/Padrao_TISS_Representacao_de_Conceitos_em_Saude_202501.zip"


def cell_text(cell, strings):
    if cell.find("m:f", NS) is not None:
        raise ValueError("Formula inesperada na tabela de referencia")
    if cell.get("t") == "inlineStr":
        return "".join(t.text or "" for t in cell.findall("m:is//m:t", NS))
    value = cell.find("m:v", NS)
    text = "" if value is None else value.text or ""
    if cell.get("t") == "s":
        return strings[int(text)]
    if cell.get("t") not in (None, "n", "str", "d"):
        raise ValueError("Tipo de celula nao suportado")
    return text


def iso_date(value):
    if not value:
        return ""
    # O XLSX oficial usa o calendario Excel 1900, datas inteiras sem horario.
    if value.isascii() and value.isdigit():
        serial = int(value)
        if serial <= 60:
            raise ValueError("Serial Excel fora do intervalo suportado")
        return (dt.date(1899, 12, 30) + dt.timedelta(days=serial)).isoformat()
    for fmt in ("%d/%m/%Y", "%Y-%m-%d"):
        try:
            return dt.datetime.strptime(value, fmt).date().isoformat()
        except ValueError:
            pass
    raise ValueError(f"Data invalida: {value!r}")


def extract(raw):
    with zipfile.ZipFile(io.BytesIO(raw)) as workbook:
        info = ET.fromstring(workbook.read("xl/workbook.xml"))
        properties = info.find("m:workbookPr", NS)
        if properties is not None and properties.get("date1904") in ("1", "true"):
            raise ValueError("Calendario Excel 1904 nao suportado")
        sheets = [s for s in info.findall("m:sheets/m:sheet", NS) if s.get("name") == "Tab 63"]
        if len(sheets) != 1:
            raise ValueError("Aba Tab 63 nao encontrada ou duplicada")
        relationships = ET.fromstring(workbook.read("xl/_rels/workbook.xml.rels"))
        matches = [r for r in relationships if r.get("Id") == sheets[0].get(RID)]
        if len(matches) != 1 or matches[0].get("TargetMode") == "External":
            raise ValueError("Referencia da aba invalida")
        target = matches[0].get("Target")
        sheet_path = target.lstrip("/") if target.startswith("/") else posixpath.normpath("xl/" + target)
        shared = ET.fromstring(workbook.read("xl/sharedStrings.xml"))
        strings = ["".join(t.text or "" for t in s.findall(".//m:t", NS)) for s in shared]
        sheet = ET.fromstring(workbook.read(sheet_path))
        header_found, title_found = False, False
        records, seen = [], set()
        for row in sheet.findall("m:sheetData/m:row", NS):
            cells = {}
            for cell in row.findall("m:c", NS):
                column = "".join(c for c in cell.get("r", "") if c.isalpha())
                value = cell_text(cell, strings)
                if value:
                    cells[column] = value
            values = [cells.get(column, "") for column in "ABCDE"]
            if not header_found:
                if values[0].startswith("Tabela 63 - Grupos de procedimentos"):
                    title_found = True
                if values == HEADER:
                    header_found = True
                continue
            if not cells:
                continue
            if any(column not in tuple("ABCDE") for column in cells):
                raise ValueError("Coluna inesperada apos cabecalho")
            code, term, start, end, implementation = values
            if len(code) != 3 or not code.isascii() or not code.isdigit() or not term:
                raise ValueError(f"Grupo invalido: {code!r}")
            if code in seen:
                raise ValueError(f"Grupo duplicado: {code}")
            seen.add(code)
            start, end, implementation = map(iso_date, (start, end, implementation))
            if not start or (end and end < start):
                raise ValueError(f"Intervalo invalido: {code}")
            records.append(["202501", "63", code, term, "", start, end, implementation])
        if not title_found or not header_found or not records:
            raise ValueError("Tabela 63 vazia ou estrutura divergente")
        return records


def main(source, destination):
    source, destination = Path(source), Path(destination)
    raw = source.read_bytes()
    # Este importador e vinculado ao snapshot oficial identificado por hash.
    # Outra versao nao deve ser rotulada silenciosamente como 202501.
    expected = "2608c3a4002b815c29a8e3ecfa15e91e045a10d11d5aa998e8899738927a0978"
    actual = hashlib.sha256(raw).hexdigest()
    if actual != expected:
        raise ValueError("XLSX diferente do snapshot oficial 202501 conferido; revisar origem antes de importar")
    records = extract(raw)
    destination.mkdir(parents=True, exist_ok=True)
    catalog = destination / "tuss63_202501.csv"
    with catalog.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, delimiter=";", lineterminator="\n")
        writer.writerow(OUTPUT_HEADER)
        writer.writerows(records)
    manifest = {"source_filename": source.name, "source_sha256": actual,
                "source_url": SOURCE_URL, "source_sheet": "Tab 63",
                "catalog_sha256": hashlib.sha256(catalog.read_bytes()).hexdigest(),
                "catalog_version": "202501", "table": "63", "records": len(records),
                "encoding": "UTF-8", "delimiter": ";", "date_format": "YYYY-MM-DD",
                "scope": "Snapshot oficial 202501; nao comprova cobertura historica completa"}
    (destination / "manifest_tuss63_202501.json").write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(manifest, ensure_ascii=False))


if __name__ == "__main__":
    main(*sys.argv[1:])
