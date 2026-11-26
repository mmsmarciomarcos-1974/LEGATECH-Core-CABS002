# Roteiro de reprodução e citação

## Ambiente e regressão

Executar a partir da raiz do repositório. Requisitos do harness: Python 3 e gcc/g++ no PATH, com suporte a C++23; no Windows, utilizar a instalação MinGW correspondente ao ambiente de desenvolvimento. O harness compila o programa em diretório temporário e utiliza o SQLite incluído no projeto.

```text
python tools/testar_tuss.py
```

As referências versionadas devem estar presentes nos caminhos esperados. A suíte verifica seus manifestos e hashes, com o tratamento de quebras de linha explicitado no código. O script de regressão utiliza a biblioteca padrão Python; openpyxl é necessário para reproduzir os catálogos 19 e 60 a partir das planilhas, conforme seus documentos próprios.

O término com sucesso da regressão não executa a massa integral e não mede consumo energético.

## Execução no Code::Blocks

Abrir LEGATECH_Core/LEGATECH_Core.cbp, selecionar Release e executar Rebuild. Na raiz do repositório, usar o executável produzido e a sintaxe documentada em ENTRADA_DAO.md e SAIDA_DAO.md. Exemplo Windows com exportação e catálogos 22 e 63:

```text
LEGATECH_Core\bin\Release\LEGATECH_Core.exe data\sample\PR_202501_AMB_DET_sample.csv data\reference\tuss22_202501.csv data\reference\tuss63_202501.csv --export resultado_exemplo
```

A pasta resultado_exemplo deve ser nova. O carregamento automático das outras referências depende dos caminhos descritos nos documentos dos catálogos. Conferir todos os catálogos efetivamente carregados na saída.

## Execução integral registrada

A massa integral não está incluída nesta proposta. O ensaio Windows de 08/11/2026 usou código no commit e303f2306bb3fd1155c973edaccd365babc21562, em Release/GCC 14.2.0, com exportação CSV e SQLite habilitada. Usar a MATRIZ_EVIDENCIAS_CABS002.md para conferir o recorte, hashes, totais, tempo e limitações.

Antes de declarar reprodução integral independente, completar o registro do comando exato, versões do sistema e configuração do equipamento. A duração histórica de 963,538 s não é um resultado esperado obrigatório em outro ambiente.

A execução Ubuntu foi associada ao commit 6a63514312482df1439a76d20d4b30b633ba39cd e teve adaptação temporária do harness para TUSS 63. Não apresentar um checkout sem essa adaptação como reprodução exata. Exportação e auditoria estavam desativadas; os tempos não devem ser comparados diretamente com o ensaio Windows.

## Citação

CITATION.cff identifica o software e o autor. Ainda não contém versão, data de release, DOI ou licença, pois esses elementos não foram definidos nesta preparação. Ao referenciar resultados, acrescentar o commit efetivamente utilizado, o ano e a data real de acesso. A futura release documental não substitui a identificação do código histórico executado.

## Conferência desta atualização

Esta atualização acrescenta documentação e metadados de citação. Não representa nova execução dos testes ou da massa integral.

## Conferência da contagem TUSS 20

O manifesto anterior informava 41.424 e o normalizador/documentação informavam 41.416 registros. A contagem local confirmou 41.416 e a correção foi enviada à main no commit 0b53db1. O roteiro abaixo permanece útil para novas conferências. Para conferir o arquivo local sem modificá-lo, executar na raiz:

```python
import csv
import hashlib
import json
from pathlib import Path

p = Path("data/reference/tuss20_202501.csv")
with p.open(encoding="utf-8-sig", newline="") as stream:
    reader = csv.reader(stream, delimiter=";")
    header = next(reader)
    records = 0
    codes = set()
    for row in reader:
        if len(row) != len(header):
            raise ValueError(f"Estrutura divergente no registro {records + 1}")
        records += 1
        codes.add(row[2])
manifest = json.loads(Path("data/reference/manifest_tuss20_202501.json").read_text(encoding="utf-8"))
print("Registros de dados:", records)
print("Códigos únicos:", len(codes))
print("Quantidade no manifesto:", manifest["records"])
print("SHA-256 bytes locais:", hashlib.sha256(p.read_bytes()).hexdigest())
print("SHA-256 no manifesto:", manifest["catalog_sha256"])
```

A comparação de hashes considera bytes locais; CRLF/LF pode gerar diferenças, conforme CATALOGO_TUSS20.md. Esse roteiro é de conferência e não foi executado nesta atualização por indisponibilidade dos bytes integrais no acesso utilizado. Não modifica catálogo nem manifesto.

## Resolução da contagem TUSS 20

Em 14/11/2026, o autor executou a leitura local com csv.reader, separador ponto e vírgula, codificação utf-8-sig e newline vazio. Após excluir o cabeçalho e ignorar linhas vazias, foram contados 41.416 registros. O diff mostrou somente records: 41424 → 41416 no manifesto, preservando o SHA-256 registrado. A correção foi enviada à main no commit 0b53db1. O catálogo não foi alterado nessa correção; o hash não foi recalculado por esse procedimento. A procedência exata da exportação e os termos de distribuição continuam como verificações independentes.
