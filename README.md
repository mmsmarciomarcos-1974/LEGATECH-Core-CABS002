# LEGATECH Core

Projeto final da disciplina CABS002 - Programação Avançada.

Implementação em C++23 para ingestão, validação e transformação de dados heterogêneos.

## Contexto

Universidade Tecnológica Federal do Paraná - UTFPR  
Programa de Pós-Graduação em Computação Aplicada - PPGCA  
Disciplina: CABS002 - Programação Avançada

## Catálogos TUSS 22 e 63

A carga conjunta opcional e os comandos de validação no Windows estão em [CATALOGOS_TUSS22_63.md](CATALOGOS_TUSS22_63.md). A tabela 63 usa uma referência externa versionada com 31 grupos, conferida no pacote oficial 202501.

### Catálogo TUSS 19

A referência oficial 202501, a normalização das duas planilhas e a carga automática das sete partes estão descritas em [CATALOGO_TUSS19.md](CATALOGO_TUSS19.md). O catálogo normalizado contém código e datas de vigência, sem as descrições extensas das planilhas originais. A TUSS 60 de unidade de medida e sua normalização estão documentadas em [CATALOGO_TUSS60.md](CATALOGO_TUSS60.md).

## Entrada CSV e SQLite por DAO

O executável aceita as duas fontes e chama `processarArquivo()` por um ponteiro de `DAOEntrada`. Comandos, formato relacional e explicação para a sabatina: [ENTRADA_DAO.md](ENTRADA_DAO.md).

### Resultados CSV e SQLite por DAO

Use `--export pasta_nova` para exportar classificacoes, rejeicoes e alertas nos dois formatos. Comandos, classes e limites estao em [SAIDA_DAO.md](SAIDA_DAO.md).

### Limites e precisao dos campos (RF04)

Regras implementadas, fonte consultada e pendencias: [VALIDACAO_CAMPOS.md](VALIDACAO_CAMPOS.md).

### Datas e compatibilidade temporal (RF05)

Regras, fronteira Y2K38 e roteiro da sabatina: [VALIDACAO_TEMPORAL.md](VALIDACAO_TEMPORAL.md).

### Regra complementar DUP-01 — duplicidade de registros

A duplicidade é um controle complementar e não altera a numeração do baseline aceito, no qual RF07 corresponde ao registro dos motivos de rejeição. Regra, evidências de teste e limite operacional: [VALIDACAO_DUPLICIDADE.md](VALIDACAO_DUPLICIDADE.md).

### Matriz de evidências CABS002

Rastreabilidade dos requisitos RF01–RF10, testes, resultados e limitações: [MATRIZ_EVIDENCIAS_CABS002.md](MATRIZ_EVIDENCIAS_CABS002.md).

O workflow [Regressão TUSS — Linux e Windows](.github/workflows/regressao-tuss.yml) executa `tools/testar_tuss.py` nas duas plataformas após alterações no branch `main` e em pull requests.

## Execução no Windows e no Ubuntu

Em 02/11/2026, o projeto foi executado pelo Code::Blocks, na configuração Release, nos dois sistemas. A configuração usa caminhos relativos e a mesma massa controlada `data/sample/PR_202501_AMB_DET_validacao.csv`, com as referências TUSS 22 e 63 de 202501.

| Ambiente | Registros | Válidos | Válidos com alerta | Rejeitados | Estrutura divergente | Retorno |
|---|---:|---:|---:|---:|---:|---:|
| Windows | 8 | 2 | 1 | 5 | 1 | 0 |
| Ubuntu | 8 | 2 | 1 | 5 | 1 | 0 |

Nos dois ambientes, a exportação foi confirmada em `../data/output/saida_codeblocks`. Esta evidência demonstra a compilação/execução e a resolução dos caminhos relativos com a massa controlada; não representa benchmark nem validação de processamento em escala.

A origem e a normalização das TUSS 20 e 60 estão descritas em [CATALOGO_TUSS20.md](CATALOGO_TUSS20.md) e [CATALOGO_TUSS60.md](CATALOGO_TUSS60.md).
