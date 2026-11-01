# Catálogo TUSS 20 — competência 202501

## Origem

A referência oficial para os dados TISS é o [Portal de Dados Abertos da ANS](https://dadosabertos.ans.gov.br/FTP/PDA/TISS/).

O arquivo de origem preservado no repositório é `data/reference/tuss20_202501_origem.csv`. A versão normalizada usada pelo LEGATECH Core é `data/reference/tuss20_202501.csv`.

## Normalização

A transformação é feita pelo script `normalizar_tuss20.py`, na raiz do repositório. Para reproduzir:

`py normalizar_tuss20.py data\reference\tuss20_202501_origem.csv data\reference\tuss20_202501.csv`

O script valida a quantidade esperada de 41.416 registros, códigos duplicados, datas e estrutura do CSV produzido. Ao final, apresenta os hashes SHA-256 dos arquivos de origem e de saída.

## Uso e evidência

O Core carrega a TUSS 20 junto com as tabelas 22 e 63. A regressão SQLite executada no Windows confirmou o carregamento conjunto dos catálogos, com 47.411 termos.

A execução dos testes Python está registrada em [AUDITORIA_SQLITE.md](AUDITORIA_SQLITE.md).

## Reprodução validada no Windows

A reprodução do CSV gerou 41.416 registros de dados, além do cabeçalho, com oito campos. A comparação dos arquivos como CSV confirmou equivalência após uniformizar as quebras de linha internas dos campos.

Em quatro registros, a referência contém quebras internas CRLF e a saída reproduzida contém LF. Por isso, os hashes SHA-256 dos arquivos completos diferem: eles identificam os bytes, não apenas os valores dos campos.
