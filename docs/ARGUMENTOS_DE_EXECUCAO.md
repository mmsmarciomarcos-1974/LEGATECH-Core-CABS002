# Argumentos de execução — arquivos TISS

Use uma linha por execução. Os caminhos são relativos ao diretório de trabalho
`LEGATECH_Core` configurado no Code::Blocks. Cada comando recebe o CSV de entrada,
os catálogos TUSS 22 e 63, e uma pasta-base para exportação. O programa acrescenta
automaticamente o timestamp à pasta de saída.

Os arquivos grandes ficam em `data/entrada_tiss/` no computador local; não são
versionados. As referências abaixo são as usadas no teste do projeto
(`tuss22_202501.csv` e `tuss63_202501.csv`).

## PR — competência 202501

```text
../data/entrada_tiss/PR_202501_AMB_DET.csv ../data/reference/tuss22_202501.csv ../data/reference/tuss63_202501.csv --export ../data/output/saida_codeblocks
```

## PR — competência 202502

```text
../data/entrada_tiss/PR_202502_AMB_DET.csv ../data/reference/tuss22_202501.csv ../data/reference/tuss63_202501.csv --export ../data/output/saida_codeblocks
```

## PR — competência 202503

```text
../data/entrada_tiss/PR_202503_AMB_DET.csv ../data/reference/tuss22_202501.csv ../data/reference/tuss63_202501.csv --export ../data/output/saida_codeblocks
```

## PR — competência 202504

```text
../data/entrada_tiss/PR_202504_AMB_DET.csv ../data/reference/tuss22_202501.csv ../data/reference/tuss63_202501.csv --export ../data/output/saida_codeblocks
```

## PR — competência 202505

```text
../data/entrada_tiss/PR_202505_AMB_DET.csv ../data/reference/tuss22_202501.csv ../data/reference/tuss63_202501.csv --export ../data/output/saida_codeblocks
```

Execute uma linha por vez. Antes de executar, confirme que os catálogos TUSS
referenciados existem em `data/reference/`. A pasta gerada terá o formato
`saida_codeblocks_AAAAMMDD_HHMMSS`.
