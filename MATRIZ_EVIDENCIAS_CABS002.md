# Matriz de evidências — CABS002

**Versão inicial:** 02/11/2026  
**Escopo:** rastrear os dez requisitos funcionais e separar execução comprovada, testes automatizados existentes e limitações.

## Baseline dos requisitos

Os identificadores, pesos e sínteses abaixo seguem o baseline aceito registrado em `LEGATECH_Core_Evolucao_Projeto_v0_2.docx`. Os pesos totalizam 100%.

| RF | Peso | Síntese do requisito | Implementação e evidência disponível | Situação |
|---|---:|---|---|---|
| RF01 | 5% | Selecionar arquivo local. | `main.cpp` recebe o caminho do arquivo por argumento. Code::Blocks Release executou a massa controlada nos dois sistemas com caminhos relativos. | **Demonstrado em Windows e Ubuntu.** Evidência manual: 8 linhas processadas e retorno 0 em ambos. |
| RF02 | 15% | Ler pelo menos dois formatos por DAO. | `DAOEntrada`, `CsvDAOEntrada` e `SQLiteDAOEntrada`. `tests/test_fluxo_validacao.py` exercita CSV; `tests/test_fluxo_sqlite.py` exercita SQLite. | **Demonstrado em Windows e Ubuntu.** CSV: 8 registros; SQLite: 10 registros. |
| RF03 | 10% | Validar estrutura mínima antes dos registros. | Contratos CSV e esquema relacional em `ContratoAmbDet2023Plus`, `CsvDAOEntrada` e `SQLiteDAOEntrada`. A massa CSV controlada inclui uma linha estruturalmente divergente. | **Parcialmente demonstrado.** A regressão registra a linha divergente nos dois sistemas. A suíte `tools/testar_tuss.py` contém cenários adicionais de estrutura/esquema; falta registrar uma execução dessa suíte após RF07. |
| RF04 | 10% | Validar obrigatórios, tipos, faixas e domínios. | `ValidadorRegistroTiss.cpp`; regras e pendências em [VALIDACAO_CAMPOS.md](VALIDACAO_CAMPOS.md); casos em `tests/test_catalogo_tuss.cpp` e `tools/testar_tuss.py`. | **Implementado com limites declarados.** Obrigatoriedade condicional, TUSS 60, cobertura dos catálogos 19/20, aplicabilidade histórica e confronto completo com a massa permanecem pendentes. Reexecutar a suíte abrangente após RF07. |
| RF05 | 10% | Validar datas e alertas de compatibilidade temporal. | `ValidadorRegistroTiss.cpp`; regras e limites em [VALIDACAO_TEMPORAL.md](VALIDACAO_TEMPORAL.md); cenários temporais em `tests/test_catalogo_tuss.cpp` e `tools/testar_tuss.py`. | **Implementado; teste abrangente pós-RF07 pendente de registro.** Y2K38 é cenário sintético preventivo; não houve integração com destino de 32 bits. |
| RF06 | 5% | Classificar em válido, válido com alerta e rejeitado. | `ResultadoValidacao`; regressões CSV e SQLite verificam estados, contagens e ocorrências. | **Demonstrado em Windows e Ubuntu.** No CSV controlado: 2 válidos, 1 com alerta e 5 rejeitados. |
| RF07 | 10% | Registrar motivo(s) de rejeição. | `ResultadoValidacao` acumula motivos e alertas; `tests/test_fluxo_validacao.py` confere motivo de duplicidade, divergência estrutural e preservação de alerta junto com motivo. | **Demonstrado em Windows e Ubuntu** na massa controlada; a auditoria registra as ocorrências. |
| RF08 | 10% | Transformar entradas válidas para representação comum. | CSV e SQLite produzem `RegistroTissBruto` e passam pelo mesmo validador; `tools/testar_tuss.py` confere campos e preservação de valores em cenários controlados. | **Implementado e exercitado pelos dois fluxos.** A suíte abrangente precisa ser reexecutada após RF07 para fechar a evidência atual. |
| RF09 | 10% | Apresentar resumo e tempo de execução. | `ResumoProcessamento`; os testes de fluxo conferem totais e a execução Code::Blocks mostra o resumo e a duração. | **Demonstrado manualmente em Windows e Ubuntu.** A suíte automatizada confere contagens, mas não valida um limite de duração. |
| RF10 | 15% | Exportar em pelo menos dois formatos por DAO. | `DAOSaidaValidacao`, `CsvSaidaValidacao`, `SQLiteAuditoriaValidacao` e `SaidasValidacao`; regressões conferem artefatos CSV e SQLite. | **Demonstrado em Windows e Ubuntu** nos fluxos controlados, com CSV de resultados e banco de auditoria. |

## Regras complementares e nomenclatura

A rejeição de conteúdo duplicado compara os onze campos do registro. Os testes `tests/test_fluxo_validacao.py` e `tests/test_fluxo_sqlite_duplicidade.py` passaram nos dois sistemas; a documentação específica está em [VALIDACAO_DUPLICIDADE.md](VALIDACAO_DUPLICIDADE.md).

**Atenção à nomenclatura:** no baseline acima, RF07 significa registrar motivos de rejeição. A documentação atual da duplicidade também a chama RF07. Esta matriz conserva RF07 para o requisito do baseline e trata a detecção de duplicidade como regra complementar até que uma revisão aprovada do requisito confirme outra numeração. Não confundir “registrar motivos” com “detectar duplicidade”.

## Evidência de execução cruzada

Em 02/11/2026, o Code::Blocks Release executou `data/sample/PR_202501_AMB_DET_validacao.csv` no Windows e no Ubuntu, usando caminhos relativos. Ambos retornaram 0, processaram 8 registros e exportaram em `../data/output/saida_codeblocks`. O [README](README.md) registra os totais e a delimitação dessa evidência.

As regressões `tests/test_fluxo_validacao.py`, `tests/test_fluxo_sqlite.py` e `tests/test_fluxo_sqlite_duplicidade.py` também passaram nos dois ambientes, conforme os resultados registrados na sessão de validação de 02/11/2026.

## Catálogos e limites de cobertura

- A referência TUSS 20 tem origem, normalização e manifesto documentados em [CATALOGO_TUSS20.md](CATALOGO_TUSS20.md). A regressão SQLite confirmou o carregamento conjunto dos catálogos 20, 22 e 63, totalizando 47.411 termos.
- A cobertura não é completa: faltam catálogos necessários a alguns domínios, entre eles TUSS 19 e TUSS 60. Ausência no snapshot carregado não prova que o dado seja inválido.
- A execução de 500 mil registros e pico observado de 173,2 MiB foram medidos no Windows antes desta matriz. Não constituem benchmark comparável nem validação da massa integral após a inclusão da deduplicação em memória.
- O algoritmo de duplicidade mantém chaves dos registros vistos em memória. O consumo cresce com a quantidade e o tamanho das chaves distintas. A capacidade na massa integral permanece sem validação nesta versão.

## Próximos fechamentos

1. Executar `python3 tools/testar_tuss.py` no Ubuntu e `py tools\\testar_tuss.py` no Windows para confirmar a suíte abrangente no estado atual, especialmente após a deduplicação com estado interno.
2. Resolver a numeração divergente da regra de duplicidade em relação ao RF07 do baseline e ajustar os documentos relacionados.
3. Reconciliar esta matriz com eventual versão mais recente do parecer/requisitos aceitos pelo professor.
4. Manter processamento em escala como validação separada, planejada para ambiente com memória suficiente; não extrapolar os resultados controlados para a massa completa.
