"""Gera a referência compacta TUSS 19 a partir do ZIP oficial 202501.

Uso: python tools/normalizar_tuss19.py <arquivo-oficial.zip> [diretorio-de-saida]
Requer openpyxl. O resultado contém apenas código e datas de vigência usados pelo Core.
"""
import csv
import hashlib
import json
import sys
import zipfile
from datetime import date, datetime
from pathlib import Path

import openpyxl


VERSAO = "202501"
TABELA = "19"
CABECALHO = ["cd_procedimento", "inicio_vigencia", "fim_vigencia"]
TAMANHOS_PARTES = [184598, 184598, 184598, 184598, 184598, 184598, 184594]
REGISTROS_ESPERADOS = sum(TAMANHOS_PARTES)


def sha256_arquivo(path):
    digest = hashlib.sha256()
    with path.open("rb") as arquivo:
        for bloco in iter(lambda: arquivo.read(1024 * 1024), b""):
            digest.update(bloco)
    return digest.hexdigest()


def sha256_membro(zipfile_obj, nome):
    digest = hashlib.sha256()
    with zipfile_obj.open(nome) as arquivo:
        for bloco in iter(lambda: arquivo.read(1024 * 1024), b""):
            digest.update(bloco)
    return digest.hexdigest()


def codigo_texto(valor):
    if isinstance(valor, bool) or valor is None:
        raise ValueError("Código TUSS 19 ausente ou inválido")
    if isinstance(valor, (int, float)):
        if not float(valor).is_integer():
            raise ValueError(f"Código TUSS 19 não inteiro: {valor!r}")
        codigo = str(int(valor))
    else:
        codigo = str(valor).strip()
    if not codigo.isdigit() or len(codigo) not in (8, 9):
        raise ValueError(f"Código TUSS 19 fora do formato de 8 ou 9 dígitos: {codigo!r}")
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


def gerar(arquivo_zip, diretorio_saida):
    arquivo_zip = Path(arquivo_zip).resolve()
    diretorio_saida = Path(diretorio_saida).resolve()
    diretorio_saida.mkdir(parents=True, exist_ok=True)

    with zipfile.ZipFile(arquivo_zip) as pacote:
        membros = [
            nome for nome in pacote.namelist()
            if nome.lower().endswith(".xlsx")
            and "tuss 19 - materiais e opme - versão 202501_parte_" in nome.lower()
        ]
        membros.sort(key=lambda nome: 1 if "parte_1.xlsx" in nome.lower() else 2)
        if len(membros) != 2 or not all(
            "parte_1.xlsx" in nome.lower() or "parte_2.xlsx" in nome.lower()
            for nome in membros
        ):
            raise ValueError("O ZIP não contém exatamente as duas planilhas oficiais da TUSS 19 versão 202501")

        manifest = {
            "catalog_version": VERSAO,
            "table": TABELA,
            "schema": ";".join(CABECALHO),
            "delimiter": ";",
            "encoding": "UTF-8",
            "records": REGISTROS_ESPERADOS,
            "unique_codes": REGISTROS_ESPERADOS,
            "code_lengths": [8, 9],
            "source_archive": "Padrao_TISS_Representacao_de_Conceitos_em_Saude_202501.zip",
            "source_zip_sha256": sha256_arquivo(arquivo_zip),
            "source_workbooks": [
                {"file": Path(nome).name, "sha256": sha256_membro(pacote, nome)}
                for nome in membros
            ],
            "parts": [],
        }

        codigos = set()
        contagem_tamanhos = {8: 0, 9: 0}
        total = 0
        indice_parte = -1
        linhas_na_parte = 0
        saida = None
        writer = None

        def proxima_parte():
            nonlocal indice_parte, linhas_na_parte, saida, writer
            if saida:
                saida.close()
            indice_parte += 1
            linhas_na_parte = 0
            nome = f"tuss19_202501_part{indice_parte + 1:02d}.csv"
            path = diretorio_saida / nome
            saida = path.open("w", encoding="utf-8", newline="")
            writer = csv.writer(saida, delimiter=";", lineterminator="\n")
            writer.writerow(CABECALHO)

        try:
            for nome in membros:
                arquivo_temporario = diretorio_saida / Path(nome).name
                with pacote.open(nome) as origem, arquivo_temporario.open("wb") as destino:
                    for bloco in iter(lambda: origem.read(1024 * 1024), b""):
                        destino.write(bloco)
                workbook = openpyxl.load_workbook(arquivo_temporario, read_only=True, data_only=True)
                try:
                    planilha = workbook.worksheets[1]
                    titulo = str(planilha.cell(7, 1).value or "")
                    if "202501" not in titulo or "Tabela 19" not in titulo:
                        raise ValueError(f"Identificação da planilha divergente: {titulo!r}")
                    cabecalho_origem = [planilha.cell(9, i).value for i in range(1, 11)]
                    if cabecalho_origem[0] != "Código do Termo" or \
                       cabecalho_origem[4] != "Data de início de vigência" or \
                       cabecalho_origem[5] != "Data de fim de vigência":
                        raise ValueError("Cabeçalho oficial da TUSS 19 divergente")
                    for linha in planilha.iter_rows(min_row=10, values_only=True):
                        if linha[0] is None and linha[1] is None:
                            continue
                        codigo = codigo_texto(linha[0])
                        if codigo in codigos:
                            raise ValueError(f"Código TUSS 19 duplicado: {codigo}")
                        codigos.add(codigo)
                        contagem_tamanhos[len(codigo)] += 1
                        inicio = data_iso(linha[4], obrigatoria=True)
                        fim = data_iso(linha[5], obrigatoria=False)
                        if total >= REGISTROS_ESPERADOS:
                            raise ValueError("Planilhas excedem a quantidade esperada de códigos")
                        if indice_parte < 0 or linhas_na_parte == TAMANHOS_PARTES[indice_parte]:
                            proxima_parte()
                        writer.writerow([codigo, inicio, fim])
                        total += 1
                        linhas_na_parte += 1
                finally:
                    workbook.close()
                    arquivo_temporario.unlink(missing_ok=True)
            if total != REGISTROS_ESPERADOS or len(codigos) != REGISTROS_ESPERADOS:
                raise ValueError(f"Quantidade divergente: {total} registros, {len(codigos)} códigos únicos")
        finally:
            if saida:
                saida.close()

    for indice, esperado in enumerate(TAMANHOS_PARTES, start=1):
        path = diretorio_saida / f"tuss19_202501_part{indice:02d}.csv"
        with path.open(encoding="utf-8", newline="") as arquivo:
            contagem = sum(1 for _ in csv.reader(arquivo, delimiter=";")) - 1
        if contagem != esperado:
            raise ValueError(f"Parte {indice:02d}: esperados {esperado} registros, encontrados {contagem}")
        manifest["parts"].append({
            "file": path.name,
            "records": contagem,
            "bytes": path.stat().st_size,
            "sha256": sha256_arquivo(path),
        })
    manifest["code_length_counts"] = {str(k): v for k, v in contagem_tamanhos.items()}
    (diretorio_saida / "manifest.json").write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(f"TUSS 19 {VERSAO}: {total} registros; {len(codigos)} códigos únicos")
    print(f"Códigos com 8 dígitos: {contagem_tamanhos[8]}; com 9 dígitos: {contagem_tamanhos[9]}")
    print(f"Catálogo gerado em: {diretorio_saida}")


if __name__ == "__main__":
    if len(sys.argv) not in (2, 3):
        raise SystemExit(__doc__)
    destino = Path(sys.argv[2]) if len(sys.argv) == 3 else Path("data/reference/tuss19_202501")
    gerar(sys.argv[1], destino)
