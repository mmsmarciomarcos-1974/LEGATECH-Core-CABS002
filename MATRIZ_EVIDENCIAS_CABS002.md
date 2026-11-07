# Matriz de evidências — CABS002

**Versão inicial:** 02/11/2026  
**Escopo:** rastrear os dez requisitos funcionais e separar execução comprovada, testes automatizados existentes e limitações.

## Baseline dos requisitos

Os identificadores, pesos e sínteses abaixo seguem o baseline aceito registrado em `LEGATECH_Core_Evolucao_Projeto_v0_2.docx`. Os pesos totalizam 100%.

| RF | Peso | Síntese do requisito | Implementação e evidência disponível | Situação |
|---|---:|---|---|---|
| RF01 | 5% | Selecionar arquivo local. | `main.cpp` recebe o caminho do arquivo por argumento. Code::Blocks Release executou a massa controlada nos dois sistemas com caminhos relativos. | **Demonstrado em Windows e Ubuntu.** Evidência manual: 8 linhas processadas e retorno 0 em ambos. |
| RF02 | 15% | Ler pelo menos dois formatos por DAO. | `DAOEntrada`, `CsvDAOEntrada` e `SQLiteDAOEntrada`. `tests/test_fluxo_validacao.py` exercita CSV; `tests/test_fluxo_sqlite.py` exercita SQLite. | **Demonstrado em Windows e Ubuntu.** CSV: 8 registros; SQLite: 10 registros. |
| RF03 | 10% | Validar estrutura mínima antes dos registros. | Contratos CSV e esquema relacional em `ContratoAmbDet2023Plus`, `CsvDAOEntrada` e `SQLiteDAOEntrada`. A massa CSV controlada inclui uma linha estruturalmente divergente. | **Suíte abrangente passou no Ubuntu e no Windows em 02/11/2026.** Inclui cenários adicionais de estrutura e esquema. |
| RF04 | 10% | Validar obrigatórios, tipos, faixas e domínios. | `ValidadorRegistroTiss.cpp`; regras em [VALIDACAO_CAMPOS.md](VALIDACAO_CAMPOS.md); catálogos TUSS 19, 20, 22, 60 e 63; casos existentes em `tests/test_catalogo_tuss.cpp` e `tools/testar_tuss.py`. | TUSS 19 integrada com 1.292.182 códigos no snapshot 202501; TUSS 60 integrada com 61 códigos únicos para `UNIDADE_MEDIDA`. Quantidade, valores, vigências e indicadores de `AMB_DET` têm regras documentadas; valores exigidos podem ficar vazios em componente de pacote, e valor pago negativo gera alerta de estorno/saldo não verificável. As combinações verificáveis de indicadores e referências foram fechadas. **Falta registrar regressão específica da TUSS 60 e execução pós-alteração no Windows.** Obrigatoriedade condicional dos demais campos, aplicabilidade histórica e confronto completo com a massa permanecem pendentes. |
| RF05 | 10% | Validar datas e alertas de compatibilidade temporal. | `ValidadorRegistroTiss.cpp`; regras e limites em [VALIDACAO_TEMPORAL.md](VALIDACAO_TEMPORAL.md); cenários temporais em `tests/test_catalogo_tuss.cpp` e `tools/testar_tuss.py`. | **Suíte abrangente passou no Ubuntu e no Windows em 02/11/2026**, incluindo datas, rejeições e alertas Y2K38 exportados em CSV e SQLite. Y2K38 é cenário sintético preventivo; não houve integração com destino de 32 bits. |
| RF06 | 5% | Classificar em válido, válido com alerta e rejeitado. | `ResultadoValidacao`; regressões CSV e SQLite verificam estados, contagens e ocorrências. | **Demonstrado em Windows e Ubuntu.** No CSV controlado: 2 válidos, 1 com alerta e 5 rejeitados. |
| RF07 | 10% | Registrar motivo(s) de rejeição. | `ResultadoValidacao` acumula motivos e alertas; `tests/test_fluxo_validacao.py` confere motivo de duplicidade, divergência estrutural e preservação de alerta junto com motivo. | **Demonstrado em Windows e Ubuntu** na massa controlada; a auditoria registra as ocorrências. |
| RF08 | 10% | Transformar entradas válidas para representação comum. | CSV e SQLite produzem `RegistroTissBruto` e passam pelo mesmo validador; `tools/testar_tuss.py` confere campos e preservação de valores em cenários controlados. | **Suíte abrangente passou no Ubuntu e no Windows em 02/11/2026**, incluindo paridade CSV/SQLite, histórico, zeros e proteção de resultados. |
| RF09 | 10% | Apresentar resumo e tempo de execução. | `ResumoProcessamento`; os testes de fluxo conferem totais e a execução Code::Blocks mostra o resumo e a duração. | **Demonstrado manualmente em Windows e Ubuntu.** A suíte automatizada confere contagens, mas não valida um limite de duração. |
| RF10 | 15% | Exportar em pelo menos dois formatos por DAO. | `DAOSaidaValidacao`, `CsvSaidaValidacao`, `SQLiteAuditoriaValidacao` e `SaidasValidacao`; regressões conferem artefatos CSV e SQLite. | **Demonstrado em Windows e Ubuntu** nos fluxos controlados, com CSV de resultados e banco de auditoria. |

## Regras complementares e nomenclatura

A rejeição de conteúdo duplicado compara os onze campos do registro. Essa regra complementar é identificada nos documentos do projeto como **DUP-01**. Os testes `tests/test_fluxo_validacao.py` e `tests/test_fluxo_sqlite_duplicidade.py` passaram nos dois sistemas; a documentação específica está em [VALIDACAO_DUPLICIDADE.md](VALIDACAO_DUPLICIDADE.md).

**Distinção em relação ao baseline:** RF07 significa registrar motivos de rejeição. DUP-01 identifica a detecção de registros duplicados, uma regra adicional, sem alterar a lista ou a numeração dos requisitos aceitos. Eventual incorporação de DUP-01 como novo requisito depende de aprovação do responsável pela disciplina.

## Evidência de execução cruzada

Em 02/11/2026, o Code::Blocks Release executou `data/sample/PR_202501_AMB_DET_validacao.csv` no Windows e no Ubuntu, usando caminhos relativos. Ambos retornaram 0, processaram 8 registros e exportaram em `../data/output/saida_codeblocks`. O [README](README.md) registra os totais e a delimitação dessa evidência.

As regressões `tests/test_fluxo_validacao.py`, `tests/test_fluxo_sqlite.py` e `tests/test_fluxo_sqlite_duplicidade.py` também passaram nos dois ambientes, conforme os resultados registrados na sessão de validação de 02/11/2026.

Em 02/11/2026, a suíte abrangente `tools/testar_tuss.py` passou no Ubuntu (`python3 tools/testar_tuss.py`) e no Windows (`py tools\testar_tuss.py`). No Windows, foi necessário incluir `C:\Program Files\CodeBlocks\MinGW\bin` no `PATH` para disponibilizar `gcc`, usado pelo script ao compilar seus testes. As duas execuções concluíram todos os cenários e a limpeza dos diretórios temporários.

## Catálogos e limites de cobertura

- As referências TUSS 19 e 20 têm origem, normalização e manifesto documentados em [CATALOGO_TUSS19.md](CATALOGO_TUSS19.md) e [CATALOGO_TUSS20.md](CATALOGO_TUSS20.md). A carga completa de 19, 20, 22 e 63 contém 1.339.601 termos.
- A cobertura de terminologias ainda não é completa. Para códigos de procedimentos nas tabelas 19, 20, 22 e 63, ausência no snapshot não prova invalidade e gera alerta. A TUSS 60 é uma tabela menor e completa no pacote 202501 (61 códigos únicos); `UNIDADE_MEDIDA` fora desse domínio é rejeitada.
- No Windows, uma amostra de 500.000 registros foi processada com DUP-01 ativa: 488.276 válidos, 296 válidos com alerta, 11.428 rejeitados e nenhuma divergência estrutural. Entre as rejeições, 11.347 foram por duplicidade. A execução levou 51,794 segundos; o pico de memória observado foi 173,2 MiB.
- Essa medição é de uma execução em uma máquina específica; não é um benchmark comparativo nem valida o arquivo integral. O consumo de memória da deduplicação cresce com a quantidade e o tamanho das chaves distintas, e a capacidade para processar a massa completa permanece sem validação.

## Próximos fechamentos

1. Confirmar com o responsável pela disciplina se a regra DUP-01 deve ser incorporada formalmente ao baseline como novo requisito.
2. Reconciliar esta matriz com eventual versão mais recente do parecer/requisitos aceitos pelo professor.
3. Se necessário para a entrega, planejar novo ensaio de escala com registro de ambiente e memória antes/depois; não extrapolar a amostra de 500 mil para o arquivo integral.
