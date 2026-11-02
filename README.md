# LEGATECH Core

Projeto final da disciplina CABS002 - Programação Avançada.

Implementação em C++23 para ingestão, validação e transformação de dados heterogêneos.

## Contexto

Universidade Tecnológica Federal do Paraná - UTFPR  
Programa de Pós-Graduação em Computação Aplicada - PPGCA  
Disciplina: CABS002 - Programação Avançada
## Catálogos TUSS 22 e 63

A carga conjunta opcional e os comandos de validação no Windows estão em [CATALOGOS_TUSS22_63.md](CATALOGOS_TUSS22_63.md). A tabela 63 usa uma referência externa versionada com 31 grupos, conferida no pacote oficial 202501.

## Entrada CSV e SQLite por DAO

O executável aceita as duas fontes e chama `processarArquivo()` por um ponteiro de `DAOEntrada`. Comandos, formato relacional e explicação para a sabatina: [ENTRADA_DAO.md](ENTRADA_DAO.md).

### Resultados CSV e SQLite por DAO

Use `--export pasta_nova` para exportar classificacoes, rejeicoes e alertas nos dois formatos. Comandos, classes e limites estao em [SAIDA_DAO.md](SAIDA_DAO.md).

### Limites e precisao dos campos (RF04)

Regras implementadas, fonte consultada e pendencias: [VALIDACAO_CAMPOS.md](VALIDACAO_CAMPOS.md).

### Datas e compatibilidade temporal (RF05)

Regras, fronteira Y2K38 e roteiro da sabatina: [VALIDACAO_TEMPORAL.md](VALIDACAO_TEMPORAL.md).

### Duplicidade de registros (RF07)

Regra implementada, evidências de teste e limite operacional: [VALIDACAO_DUPLICIDADE.md](VALIDACAO_DUPLICIDADE.md).

A origem e a normalização da TUSS 20 estão descritas em [CATALOGO_TUSS20.md](CATALOGO_TUSS20.md).