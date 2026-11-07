"""Normaliza a TUSS 60 da representacao de conceitos TISS 202501.

Uso: python tools/normalizar_tuss60.py <arquivo-oficial.zip> [arquivo-de-saida.csv]
Requer openpyxl. O CSV preserva os campos usados pelo CatalogoTuss.
"""
import csv
import hashlib
import io
import json
import sys
import zipfile
from datetime import date, datetime
from pathlib import Path

import openpyxl


VERSAO = "202501"
TABELA = "60"
REGISTROS_ESPERADOS = 61
HASH_FONTE_ESPERADO = "e4f7ee54d0e06c6c7f5a3374fb8cd8bedd67069e6c44f1d05b03a89298f30275"
NOME_PLANILHA = "TUSS - Demais terminologias - VERSÃO 202501.xlsx"
TITULO = "Tabela 60 - Terminologia de unidade de medida"
CABECALHO_ORIGEM = [
    "Código do Termo", "Termo", "Descrição Detalhada",
    "Data de início de vigência", "Data de fim de vigência",
    "Data de fim de implantação",
]
CABECALHO_SAIDA = [
    "versao_catalogo", "cd_tabela_referencia", "cd_procedimento", "termo",
    "descricao_detalhada", "inicio_vigencia", "fim_vigencia", "fim_implantacao",
]


def sha256_arquivo(arquivo):
    digest = hashlib.sha256()
    with Path(arquivo).open("rb") as stream:
        for bloco in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(bloco)
    return digest.hexdigest()


def sha256_membro(pacote, nome):
    digest = hashlib.sha256()
    with pacote.open(nome) as stream:
        for bloco in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(bloco)
    return digest.hexdigest()


def texto(valor):
    return "" if valor is None else str(valor).strip()


def codigo_texto(valor):
    if isinstance(valor, bool) or valor is None:
        raise ValueError("Código TUSS 60 ausente ou inválido")
    if isinstance(valor, int):
        codigo = f"{valor:03d}"
    elif isinstance(valor, float) and valor.is_integer():
        codigo = f"{int(valor):03d}"
    else:
        codigo = str(valor).strip()
    if len(codigo) != 3 or not codigo.isdigit():
        raise ValueError(f"Código TUSS 60 fora do formato de três dígitos: {codigo!r}")
    return codigo


def data_iso(valor, obrigatoria):
    if valor is None or valor == "":
        if obrigatoria:
            raise ValueError("Data inicial de vigência ausente")
        return ""
    if isinstance(valor, datetime):
        return valor.date().isoformat()
    if isinstance(valor, date):
        return valor.isoformat()
    raise ValueError(f"Data de vigência em formato inesperado: {valor!r}")


def gerar(arquivo_zip, arquivo_saida):
    arquivo_zip = Path(arquivo_zip).resolve()
    arquivo_saida = Path(arquivo_saida).resolve()
    arquivo_saida.parent.mkdir(parents=True, exist_ok=True)

    hash_fonte = sha256_arquivo(arquivo_zip)
    if hash_fonte != HASH_FONTE_ESPERADO:
        raise ValueError(f"Hash do pacote 202501 divergente: {hash_fonte}")

    with zipfile.ZipFile(arquivo_zip) as pacote:
        membros = [
            nome for nome in pacote.namelist()
            if Path(nome).name == NOME_PLANILHA
        ]
        if len(membros) != 1:
            raise ValueError("O ZIP não contém exatamente a planilha oficial de demais terminologias 202501")
        nome_planilha = membros[0]
        hash_planilha = sha256_membro(pacote, nome_planilha)
        workbook_bytes = pacote.read(nome_planilha)

    workbook = openpyxl.load_workbook(
        io.BytesIO(workbook_bytes), read_only=True, data_only=True
    )
    if "Tab 60" not in workbook.sheetnames:
        workbook.close()
        raise ValueError("Planilha Tab 60 ausente no arquivo oficial")
    planilha = workbook["Tab 60"]
    if texto(planilha.cell(4, 1).value) != TITULO:
        workbook.close()
        raise ValueError("Identificação da TUSS 60 divergente")
    cabecalho = [planilha.cell(6, coluna).value for coluna in range(1, 7)]
    if cabecalho != CABECALHO_ORIGEM:
        workbook.close()
        raise ValueError("Cabeçalho oficial da TUSS 60 divergente")

    linhas = []
    codigos = set()
    for linha in planilha.iter_rows(min_row=7, max_col=6, values_only=True):
        if all(valor is None for valor in linha):
            continue
        codigo = codigo_texto(linha[0])
        if codigo in codigos:
            workbook.close()
            raise ValueError(f"Código TUSS 60 duplicado: {codigo}")
        codigos.add(codigo)
        inicio = data_iso(linha[3], obrigatoria=True)
        fim = data_iso(linha[4], obrigatoria=False)
        implantacao = data_iso(linha[5], obrigatoria=False)
        linhas.append([
            VERSAO, TABELA, codigo, texto(linha[1]), texto(linha[2]),
            inicio, fim, implantacao,
        ])
    workbook.close()

    if len(linhas) != REGISTROS_ESPERADOS or len(codigos) != REGISTROS_ESPERADOS:
        raise ValueError(
            f"Quantidade divergente: esperados {REGISTROS_ESPERADOS}, encontrados {len(linhas)}"
        )

    with arquivo_saida.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, delimiter=";", lineterminator="\n")
        writer.writerow(CABECALHO_SAIDA)
        writer.writerows(linhas)

    manifesto = {
        "catalog_version": VERSAO,
        "table": TABELA,
        "table_name": TITULO,
        "schema": ";".join(CABECALHO_SAIDA),
        "delimiter": ";",
        "encoding": "UTF-8",
        "records": len(linhas),
        "unique_codes": len(codigos),
        "code_length": 3,
        "source_archive": "Padrao_TISS_Representacao_de_Conceitos_em_Saude_202501.zip",
        "source_zip_sha256": hash_fonte,
        "source_workbook": Path(nome_planilha).name,
        "source_workbook_sha256": hash_planilha,
        "source_sheet": "Tab 60",
        "source_header_row": 6,
        "source_data_first_row": 7,
        "output_file": arquivo_saida.name,
        "output_bytes": arquivo_saida.stat().st_size,
        "output_sha256": sha256_arquivo(arquivo_saida),
    }
    manifesto_path = arquivo_saida.with_name("manifest.json")
    manifesto_path.write_text(
        json.dumps(manifesto, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(f"TUSS 60 {VERSAO}: {len(linhas)} códigos; arquivo {arquivo_saida}")
    print(f"Manifesto: {manifesto_path}")


if __name__ == "__main__":
    if len(sys.argv) not in (2, 3):
        raise SystemExit(__doc__)
    destino = Path(sys.argv[2]) if len(sys.argv) == 3 else Path(
        "data/reference/tuss60_202501/tuss60_202501.csv"
    )
    gerar(sys.argv[1], destino)
