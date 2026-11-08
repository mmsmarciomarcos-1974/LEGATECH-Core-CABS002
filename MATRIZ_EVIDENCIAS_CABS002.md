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
| RF04 | 10% | Validar obrigatórios, tipos, faixas e domínios. | `ValidadorRegistroTiss.cpp`; regras em [VALIDACAO_CAMPOS.md](VALIDACAO_CAMPOS.md); catálogos TUSS 19, 20, 22, 60 e 63; regressão em `tests/test_catalogo_tuss.cpp` e `tools/testar_tuss.py`. | TUSS 19 integrada com 1.292.182 códigos no snapshot 202501; TUSS 60 integrada com 61 códigos únicos para `UNIDADE_MEDIDA`. A regressão agora cobre domínio, formato, campo condicional, vigência e paridade CSV/SQLite. O workflow [regressao-tuss.yml](.github/workflows/regressao-tuss.yml) executou a suíte em Linux e Windows; ambas as execuções foram aprovadas ([run no GitHub Actions](https://github.com/mmsmarciomarcos-1974/LEGATECH-Core-CABS002/actions/runs/42428579828)). A obrigatoriedade contextual é explicitada como regra operacional quando inferível; aplicabilidade histórica e confronto com a massa completa permanecem limites. A rastreabilidade por campo está detalhada abaixo. |
| RF05 | 10% | Validar datas e alertas de compatibilidade temporal. | `ValidadorRegistroTiss.cpp`; regras e limites em [VALIDACAO_TEMPORAL.md](VALIDACAO_TEMPORAL.md); cenários temporais em `tests/test_catalogo_tuss.cpp` e `tools/testar_tuss.py`. | **Suíte abrangente passou no Ubuntu e no Windows em 02/11/2026**, incluindo datas, rejeições e alertas Y2K38 exportados em CSV e SQLite. Y2K38 é cenário sintético preventivo; não houve integração com destino de 32 bits. |
| RF06 | 5% | Classificar em válido, válido com alerta e rejeitado. | `ResultadoValidacao`; regressões CSV e SQLite verificam estados, contagens e ocorrências. | **Demonstrado em Windows e Ubuntu.** No CSV controlado: 2 válidos, 1 com alerta e 5 rejeitados. |
| RF07 | 10% | Registrar motivo(s) de rejeição. | `ResultadoValidacao` acumula motivos e alertas; `tests/test_fluxo_validacao.py` confere motivo de duplicidade, divergência estrutural e preservação de alerta junto com motivo. | **Demonstrado em Windows e Ubuntu** na massa controlada; a auditoria registra as ocorrências. |
| RF08 | 10% | Transformar entradas válidas para representação comum. | CSV e SQLite produzem `RegistroTissBruto` e passam pelo mesmo validador; `tools/testar_tuss.py` confere campos e preservação de valores em cenários controlados. | **Suíte abrangente passou no Ubuntu e no Windows em 02/11/2026**, incluindo paridade CSV/SQLite, histórico, zeros e proteção de resultados. |
| RF09 | 10% | Apresentar resumo e tempo de execução. | `ResumoProcessamento`; os testes de fluxo conferem totais e a execução Code::Blocks mostra o resumo e a duração. | **Demonstrado manualmente em Windows e Ubuntu.** A suíte automatizada confere contagens, mas não valida um limite de duração. |
| RF10 | 15% | Exportar em pelo menos dois formatos por DAO. | `DAOSaidaValidacao`, `CsvSaidaValidacao`, `SQLiteAuditoriaValidacao` e `SaidasValidacao`; regressões conferem artefatos CSV e SQLite. | **Demonstrado em Windows e Ubuntu** nos fluxos controlados, com CSV de resultados e banco de auditoria. |


## Rastreabilidade detalhada do RF04

**Revisão:** 08/11/2026. A tabela abaixo relaciona cada uma das onze colunas de `AMB_DET` à referência disponível, à regra operacional documentada, à evidência automatizada e ao limite que impede extrapolar o resultado. O teste de referência é `tests/test_catalogo_tuss.cpp`; a suíte `tools/testar_tuss.py` verifica as referências, cenários de integração e paridade de processamento indicados. A regra de data está detalhada também em [VALIDACAO_TEMPORAL.md](VALIDACAO_TEMPORAL.md).

| Campo | Fonte e regra do Core | Evidência de teste disponível | Limite da evidência |
|---|---|---|---|
| `ID_EVENTO_ATENCAO_SAUDE` | Dicionário ANS consultado: NUMBER, tamanho 30. O perfil exige o identificador e limita a 30 dígitos. | `tests/test_catalogo_tuss.cpp`: limite de 30 aceito e 31 rejeitado. | O dicionário consultado é a publicação pública disponível na consulta, não prova sozinho a aplicabilidade histórica exata à competência 202501. |
| `UF_PRESTADOR` | Perfil local exige duas letras e uma das 27 siglas de UF. Minúsculas são aceitas com alerta de padronização. ANS descreve o campo como CHAR(2) em dicionário histórico; IBGE relaciona as siglas. | Teste cobre as 27 siglas, vazio, comprimento/caracteres inválidos, domínio desconhecido e alerta para caixa baixa. CI Linux/Windows: run [42437074476](https://github.com/mmsmarciomarcos-1974/LEGATECH-Core-CABS002/actions/runs/42437074476), commit `0df4a3b1d2bb07d0a48ba7b676f7ae30ac999f10`. | A obrigatoriedade é regra local do perfil; as fontes citadas não demonstram que a ANS a imponha ao extrato AMB_DET 202501. |
| `DT_REALIZACAO` | Competência no formato `AAAA-MM`; mês impossível é rejeitado. Mês válido a partir de 2038 recebe alerta `COMPATIBILIDADE_Y2K38`, sem rejeição. | Teste cobre formatos inválidos, mês válido, ano zero, 2037/2038, anos posteriores, divergência de competência e coexistência de alertas. | Y2K38 é cenário sintético de compatibilidade; não houve execução em destino de 32 bits nem prova de falha em produção. A regra temporal é requisito RF05 relacionado, não evidência de que a ANS proíba datas futuras. |
| `CD_PROCEDIMENTO` | Código textual de até 40 caracteres; validado junto com `CD_TABELA_REFERENCIA`. TUSS 19, 20, 22 e 63 carregadas conforme referências configuradas. Código ausente gera alerta, não rejeição automática. | Testes cobrem tamanho, formatos/domínios, consulta por par tabela-código, códigos localizados/ausentes e vigência; normalizadores conferem os snapshots. | Snapshots 202501 não demonstram validade histórica fora da competência nem cobrem necessariamente todo código próprio de operadora. Ausência em catálogo não prova invalidade. |
| `CD_TABELA_REFERENCIA` | Código textual de até 40 caracteres; tabela e procedimento são consultados como par. Referências próprias `00`, `90` e `98` têm as condições locais descritas para indicadores. | Testes cobrem comprimento, isolamento de pares e combinações com indicadores. | A semântica da referência provém do Componente Organizacional TISS; o mapeamento para indicadores no extrato é uma inferência operacional explicitada em `VALIDACAO_CAMPOS.md`. |
| `QT_ITEM_EVENTO_INFORMADO` | Número decimal opcional, não negativo, até seis dígitos de precisão total; sem escala fixa. Zero e vazio aceitos no perfil do extrato. | Testes cobrem vazio, zero, ponto/vírgula, limites, precisão excedida, sinais e formatos inadequados; a massa controlada CSV/SQLite mantém a regra comum. | A ANS não informa escala, mínimo ou obrigatoriedade no dicionário consultado para o extrato. O limite de seis dígitos é interpretação operacional do tamanho declarado, não uma escala normativa. |
| `VL_ITEM_EVENTO_INFORMADO` | Número com até 16 dígitos inteiros e duas casas decimais; obrigatório fora de componente de pacote identificado por `IND_PACOTE=1`; zero aceito e negativo rejeitado. | Testes cobrem precisão, casas excedentes, vazio fora/dentro de pacote, zero, negativo e formatos inválidos. | A regra de obrigatoriedade contextual é sustentada pela semântica das guias e pela interpretação do registro de pacote; não equivale a validar uma mensagem ou guia TISS completa. |
| `VL_ITEM_PAGO_FORNECEDOR` | Mesma precisão; obrigatório fora de componente de pacote; zero indica ausência de pagamento. Negativo é aceito com alerta de possível estorno. | Testes cobrem precisão, vazio/contexto de pacote, zero, negativo com alerta e precisão inválida. | O Core não verifica saldo acumulado, justificativa ou vínculo de estorno na guia. A aplicação da regra geral de estorno a este campo é interpretação conservadora documentada. |
| `UNIDADE_MEDIDA` | Opcional quando o extrato não permite inferir a condição de cobrança. Preenchida: código numérico TUSS 60 de três dígitos, existente e vigente na data do evento. | Testes cobrem formato, domínio, vigência e carga integral dos 61 códigos únicos do snapshot 202501. | Snapshot comprova o catálogo da competência analisada, não a condição de obrigatoriedade por item nem a validade em outra competência. |
| `IND_PACOTE` | Vazio ou `0`/`1`; exigido como `1` para referências `90`/`98`. Em TUSS, `1` é permitido para componente de pacote. | Testes percorrem combinações de `00`, `90`, `98`, TUSS 19/20/22/63 e rejeitam valor fora de `0`/`1`. | Obrigatoriedade e combinação são perfil operacional do extrato, baseado no significado de tabela de referência; não são apresentadas como matriz normativa universal da ANS para todas as guias. |
| `IND_TABELA_PROPRIA` | Vazio ou `0`/`1`; referência `00` exige `1`; em tabela TUSS, `1` é incompatível. Para `90`/`98`, não se infere obrigatoriedade adicional. | Testes cobrem combinações acima, valores ausentes/incompatíveis e domínio binário. | A interpretação contextual está delimitada em `VALIDACAO_CAMPOS.md`; os testes demonstram comportamento do Core, não conformidade integral com todo o universo TISS. |

### Leitura da cobertura e do Y2K38

A regressão evidencia o comportamento implementado para casos definidos, limites e combinações selecionados. Ela não substitui validação contra a massa pública completa, nem comprova por si só a aplicabilidade histórica de cada regra à competência 202501. A ausência no snapshot de procedimentos TUSS permanece alerta; para TUSS 60, o snapshot 202501 é tratado como domínio completo no escopo documentado.

O cenário Y2K38 pertence ao controle temporal associado a `DT_REALIZACAO`: datas sintaticamente válidas a partir de janeiro de 2038 mantêm-se válidas e recebem alerta de compatibilidade, enquanto datas inválidas são rejeitadas sem alerta Y2K38. A cobertura é sintética e preventiva; não afirma que haja risco observado no ambiente de execução nem que o projeto tenha testado uma plataforma de 32 bits.

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
