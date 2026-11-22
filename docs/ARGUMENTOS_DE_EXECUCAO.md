# Argumentos de execução — arquivos TISS

Use uma linha por execução. Os caminhos são relativos ao diretório de trabalho
`LEGATECH_Core` configurado no Code::Blocks. Cada comando recebe o CSV de entrada,
os catálogos TUSS 22 e 63, e uma pasta-base para exportação. O programa acrescenta
automaticamente o timestamp à pasta e aos arquivos gerados.

Os arquivos grandes ficam em `data/entrada_tiss/` no computador local; não são
versionados. As referências abaixo são as usadas no teste do projeto
(`tuss22_202501.csv` e `tuss63_202501.csv`).

Informe a versão das referências com `--catalogo-tuss AAAAMM`. Ela é independente
da competência do arquivo TISS. Se a opção for omitida, o programa usa `202501`.
Os caminhos de TUSS 20, 60 e 19 são montados a partir dessa versão; os arquivos
TUSS 22 e 63 continuam sendo informados na linha de execução. Para o TUSS 19,
as partes são localizadas pelo nome e precisam ser sequenciais; a conferência
de quantidade de registros está cadastrada para a versão `202501`.

O contrato `AMB_DET` a partir de 2023 reconhece o layout de 13 colunas, incluindo
`QT_ITEM_EVENTO_PAGO` e `VL_ITEM_EVENTO_PAGO`. O layout anterior de 11 colunas
continua aceito; nesses registros, os dois campos pagos ficam vazios nas saídas.
As regras atuais continuam validando os campos já implementados.

## PR — competência 202501

```text
../data/entrada_tiss/PR_202501_AMB_DET.csv ../data/reference/tuss22_202501.csv ../data/reference/tuss63_202501.csv --catalogo-tuss 202501 --export ../data/output/saida_codeblocks
```

## PR — competência 202502

```text
../data/entrada_tiss/PR_202502_AMB_DET.csv ../data/reference/tuss22_202501.csv ../data/reference/tuss63_202501.csv --catalogo-tuss 202501 --export ../data/output/saida_codeblocks
```

## PR — competência 202503

```text
../data/entrada_tiss/PR_202503_AMB_DET.csv ../data/reference/tuss22_202501.csv ../data/reference/tuss63_202501.csv --catalogo-tuss 202501 --export ../data/output/saida_codeblocks
```

## PR — competência 202504

```text
../data/entrada_tiss/PR_202504_AMB_DET.csv ../data/reference/tuss22_202501.csv ../data/reference/tuss63_202501.csv --catalogo-tuss 202501 --export ../data/output/saida_codeblocks
```

## PR — competência 202505

```text
../data/entrada_tiss/PR_202505_AMB_DET.csv ../data/reference/tuss22_202501.csv ../data/reference/tuss63_202501.csv --catalogo-tuss 202501 --export ../data/output/saida_codeblocks
```

Execute uma linha por vez. Antes de executar, confirme que os catálogos TUSS
referenciados existem em `data/reference/`. A pasta gerada terá o formato
`saida_codeblocks_AAAAMMDD_HHMMSS`; dentro dela, os arquivos terão o mesmo timestamp:
`aceitos_AAAAMMDD_HHMMSS.csv`, `rejeitados_AAAAMMDD_HHMMSS.csv`,
`contexto_AAAAMMDD_HHMMSS.csv` e `resultados_AAAAMMDD_HHMMSS.db`.


## SQLite — base de demonstração

```text
--sqlite ../data/sample/entrada_sqlite_demo.db 202501 ../data/reference/tuss22_202501.csv ../data/reference/tuss63_202501.csv --catalogo-tuss 202501 --export ../data/output/saida_codeblocks
```
