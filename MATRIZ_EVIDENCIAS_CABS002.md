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
| RF04 | 10% | Validar obrigatórios, tipos, faixas e domínios. | `ValidadorRegistroTiss.cpp`; regras em [VALIDACAO_CAMPOS.md](VALIDACAO_CAMPOS.md); catálogos TUSS 19, 20, 22, 60 e 63; regressão em `tests/test_catalogo_tuss.cpp` e `tools/testar_tuss.py`. | TUSS 19 integrada com 1.292.182 códigos no snapshot 202501; TUSS 60 integrada com 61 códigos únicos para `UNIDADE_MEDIDA`. A regressão agora cobre domínio, formato, campo condicional, vigência e paridade CSV/SQLite. O workflow [regressao-tuss.yml](.github/workflows/regressao-tuss.yml) executou a suíte em Linux e Windows; ambas as execuções foram aprovadas ([run no GitHub Actions](https://github.com/mmsmarciomarcos-1974/LEGATECH-Core-CABS002/actions/runs/42428579828)). A obrigatoriedade contextual é explicitada como regra operacional quando inferível. Foi perfilado o arquivo público integral do Paraná na competência 202501; a cobertura nacional e a execução do Core sobre essa massa permanecem pendentes. A rastreabilidade por campo e o perfil agregado estão detalhados abaixo. |
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

### Verificação da referência temporal do dicionário

Na consulta de 08/11/2026, o [índice oficial do diretório de dicionários da ANS](https://dadosabertos.ans.gov.br/FTP/PDA/TISS/DICIONARIO/) lista o dicionário de variáveis “a partir de 2023”, atualizado em 25/08/2026, e a versão “até 2022”, atualizada em 15/10/2024. O [índice do diretório TISS](https://dadosabertos.ans.gov.br/FTP/PDA/TISS/) também mostra o diretório de dicionários atualizado em 15/10/2024. A página oficial do [Padrão TISS de janeiro/2025](https://www.gov.br/ans/pt-br/assuntos/prestadores/padrao-para-troca-de-informacao-de-saude-suplementar-2013-tiss/padrao-tiss-janeiro-2025) identifica as versões dos cinco componentes; ela lista Componente Organizacional 202501, Conteúdo e Estrutura 202211 e Representação de Conceitos em Saúde 202501, sem apresentar um arquivo de dicionário de variáveis específico daquela competência.

O rótulo “a partir de 2023” torna o dicionário consultado temporalmente relevante ao conjunto de dados de 202501. Porém, o arquivo disponível foi atualizado em 2026 e não foi localizada, nesses índices oficiais, uma cópia arquivada identificada como congelada em janeiro/2025. Assim, a referência sustenta a descrição operacional dos campos, mas não prova que cada definição permaneceu inalterada na data de competência. Mantemos esse limite na matriz e não o tratamos como divergência comprovada nem como conformidade histórica definitiva.

### Leitura da cobertura e do Y2K38

A regressão evidencia o comportamento implementado para casos definidos, limites e combinações selecionados. O perfil integral de PR/202501, registrado abaixo, complementa esses testes; ele não substitui a análise de outros estados nem comprova por si só a aplicabilidade histórica de cada regra à competência 202501. A ausência no snapshot de procedimentos TUSS permanece alerta; para TUSS 60, o snapshot 202501 é tratado como domínio completo no escopo documentado.

O cenário Y2K38 pertence ao controle temporal associado a `DT_REALIZACAO`: datas sintaticamente válidas a partir de janeiro de 2038 mantêm-se válidas e recebem alerta de compatibilidade, enquanto datas inválidas são rejeitadas sem alerta Y2K38. A cobertura é sintética e preventiva; não afirma que haja risco observado no ambiente de execução nem que o projeto tenha testado uma plataforma de 32 bits.

## Perfil agregado da massa pública PR/202501

### Fonte e método

Arquivo recebido da ANS: `PR_202501_AMB_DET.zip`; CSV interno `PR_202501_AMB_DET.csv`. O arquivo contém 7.855.323 registros e 462.698.039 bytes descompactados. O SHA-256 do ZIP analisado é `ad166e72447f3ea43a461bc89abf720ff4196268b0906029860c5c4035de203b`. As referências de terminologia foram lidas do pacote TISS 202501, SHA-256 `e4f7ee54d0e06c6c7f5a3374fb8cd8bedd67069e6c44f1d05b03a89298f30275`.

O perfil foi feito em Linux x86_64, Python 3.12.14, por leitura sequencial do CSV e comparação com as tabelas oficiais 19, 20, 22, 60 e 63. Foram preservados apenas contagens agregadas; nenhum registro individual foi copiado para o relatório ou para o repositório. O arquivo medido representa **todo o recorte do Paraná em 202501**, não a base nacional. O executável do Core não foi processado contra essa massa; os números abaixo são perfil externo confrontado com as regras e o código-fonte atuais, não saída do sistema.

### Estrutura e campos

| Campo ou controle | Resultado agregado no arquivo |
|---|---|
| Estrutura | Cabeçalho exatamente igual às 11 colunas previstas; 7.855.323 registros; zero linhas com quantidade de colunas divergente. |
| `ID_EVENTO_ATENCAO_SAUDE` | Nenhum vazio ou formato localmente inválido; comprimento máximo observado: 10 caracteres. |
| `UF_PRESTADOR` | Nenhum vazio ou sigla fora do domínio; nenhuma ocorrência em caixa baixa. O recorte do Paraná não exercita as 27 siglas, já cobertas por teste unitário. |
| `DT_REALIZACAO` | Os 7.855.323 registros têm competência `2025-01`; nenhum formato ou mês inválido e nenhum caso em 2038 ou posterior. |
| `CD_PROCEDIMENTO` + `CD_TABELA_REFERENCIA` | Nenhum formato ou comprimento local inválido. As frequências das referências e o cruzamento de vigência estão na tabela seguinte. |
| `QT_ITEM_EVENTO_INFORMADO` | 346.531 vazios, aceitos pelo perfil do extrato; dois preenchimentos excedem a precisão operacional de seis dígitos. Ambos têm precisão observada de sete dígitos. |
| `VL_ITEM_EVENTO_INFORMADO` e `VL_ITEM_PAGO_FORNECEDOR` | 346.531 vazios em cada campo; todos ocorrem em registros com `IND_PACOTE=1). Nenhum formato/precisão inválido, sinal negativo em valor pago ou ocorrência negativa fora da regra local. |
| `UNIDADE_MEDIDA` | 7.746.248 vazios; 109.075 preenchidos. Todos os preenchidos correspondem a código vigente no domínio completo TUSS 60 (61 códigos). |
| `IND_PACOTE` e `IND_TABELA_PROPRIA` | Nenhum valor fora de vazio/`0`/`1) ou combinação divergente das regras locais documentadas. |

### Referências TUSS de procedimento

| Tabela | Registros no recorte | Códigos distintos usados | Cruzamento com snapshot 202501 |
|---|---:|---:|---|
| 19 | 131 | 95 | Todos localizados e dentro da vigência em janeiro/2025, após conferência da coluna oficial “Data de fim de vigência”. A indicação anterior de 3 ocorrências fora da vigência foi corrigida. |
| 20 | 68.139 | 1.384 | 68.138 registros localizados e vigentes; 1 ocorrência de código ausente do snapshot, tratada pelo Core como alerta. |
| 22 | 6.132.916 | 1.964 | 6.128.311 registros dentro da vigência; 1.262 ocorrências fora da vigência, em 2 códigos com início em 01/02/2025; 3.343 ocorrências correspondem a 14 códigos ausentes do snapshot, tratadas pelo Core como alertas. |
| 63 | 1.235.717 | 31 | Todos os 31 códigos localizados e vigentes. |

As referências proprietárias apareceram com estas frequências: `00` 225.072; `90` 67.414; `98` 125.934 registros. O código atual classifica a ausência no snapshot como alerta e a incompatibilidade comprovada de vigência como motivo de rejeição. As contagens acima são por ocorrência de cada condição; não foram somadas para produzir um total de registros rejeitados porque condições de campos diferentes podem coexistir no mesmo registro.

### Revisão do cruzamento de vigência

A recontagem usou o intervalo de competência inclusivo de 01/01/2025 a 31/01/2025 e comparou separadamente as colunas oficiais de início e fim de vigência. Na TUSS 19, os 95 códigos distintos usados (131 ocorrências) estão localizados e vigentes no mês; a contagem anterior de 128 dentro e 3 fora foi substituída por esse resultado, após conferência da coluna “Data de fim de vigência” nas planilhas oficiais. Na TUSS 22, as 1.262 ocorrências fora do período são de dois códigos: `41001141` (1.240 ocorrências) e `40314430` (22), ambos com início de vigência em 01/02/2025. Não houve ocorrência conhecida com vigência encerrada antes de janeiro/2025 no cruzamento TUSS 22. Pelo comportamento documentado no Core, esses códigos comprovadamente ainda não vigentes são rejeitados; códigos ausentes do snapshot permanecem alertas. As contagens são ocorrências de códigos e não identificam pessoas ou registros individuais.

### Investigação dos códigos ausentes dos snapshots

O cruzamento foi repetido contra as abas oficiais TUSS 20 (41.416 códigos únicos) e TUSS 22 (5.964 códigos únicos) do pacote 202501, mantendo tabela e código como chave composta. Os códigos abaixo não aparecem na aba correspondente. Isso confirma ausência no snapshot analisado, mas não prova que sejam inválidos na data do evento ou que a operadora tenha reportado incorretamente; por isso, o Core mantém o resultado como alerta.

| Tabela | Código ausente | Ocorrências no recorte |
|---|---:|---:|
| 20 | 90603303 | 1 |
| 22 | 30101077 | 423 |
| 22 | 30801095 | 1 |
| 22 | 30911117 | 2 |
| 22 | 30912164 | 4 |
| 22 | 40403300 | 3 |
| 22 | 40403866 | 11 |
| 22 | 50000462 | 2.402 |
| 22 | 50000721 | 261 |
| 22 | 50001060 | 212 |
| 22 | 81000383 | 10 |
| 22 | 82000239 | 2 |
| 22 | 82000255 | 3 |
| 22 | 82000263 | 1 |
| 22 | 84000244 | 8 |

As frequências somam 1 ocorrência ausente na TUSS 20 e 3.343 na TUSS 22, sem expor identificadores de registros ou beneficiários. Para análise científica, este achado pode ser descrito como divergência observada entre códigos reportados no recorte público PR/202501 e as abas TUSS 20/22 do pacote oficial 202501; a causa não foi determinada e não deve ser apresentada como erro confirmado do arquivo ou do catálogo.

### Interpretação e limites

No recorte analisado, os dados confirmam que as regras condicionais de pacote explicam os vazios dos dois campos de valor. Também mostram ocorrências fora da vigência nos snapshots TUSS 19 e 22, dois valores de quantidade acima da precisão operacional adotada e códigos de procedimento ausentes dos snapshots 20 e 22. Ausência no snapshot continua sem provar invalidade histórica; por isso, o Core gera alerta nesse caso.

Este perfil não valida o resultado completo de processamento/exportação do Core, não mede tempo ou memória de execução do Core e não é benchmark. Também não cobre outras UFs, consistências entre itens de uma guia nem a validade das interpretações locais para toda versão histórica do TISS. Para uso científico, registrar como perfil de um recorte público estadual completo, com fonte, hashes, ambiente e limites, sem generalizar para a base nacional.

## Execução integral do Core no recorte PR/202501

Em 08/11/2026, o Core processou o CSV integral com 7.855.323 registros, extraído do ZIP identificado na seção anterior. A compilação foi feita em Ubuntu 24.04.3 LTS x86_64, com g++ 13.3.0, usando o código do repositório no commit `6a63514312482df1439a76d20d4b30b633ba39cd` e as opções `-std=c++23 -O2 -Wall -Wextra`. Para incluir TUSS 63 sem ativar a auditoria SQLite por registro, a cópia temporária de execução acrescentou esse catálogo à lista inicial de referências; o validador, o DAO CSV, os contratos e os demais catálogos foram usados sem alteração. Nenhum arquivo do repositório foi modificado por essa adaptação.

Os CSVs TUSS 20, 22 e 63 usados nesta execução correspondem aos arquivos versionados e seus hashes SHA-256 conferem com os manifestos: `3f2f2238c3c0e65927f62e50a3723aadd5cc33584b1b65b7ee4d114f210fcad0`, `112a1fe0cc9adc313eea9931c894d9a252d13ad8a52a4102d15830ec80eba33c` e `007954dc5546a941a4da75c8b33c50fb578d0c333d8483cc32234fe34a21574d`, respectivamente. O CSV de entrada extraído tem SHA-256 `4eef163cc0b7f3d6d09e41821fa2dfb2912dd0a13f160425c008b233323ae096`. O binário produzido para a execução tem SHA-256 `e7a7d97d1a05f63a94e1ce35116a10c9e1db9c27f0d1789fe1ae46a9c1d274a7`. Foram carregados 1.339.654 termos de referência. A auditoria SQLite e as exportações CSV/SQLite ficaram desativadas; portanto, esta execução complementa as evidências de classificação e desempenho, mas não comprova RF10 para a massa integral.

| Resultado de estado | Registros |
|---|---:|
| Total | 7.855.323 |
| Válidos | 7.609.841 |
| Válidos com alerta | 3.343 |
| Rejeitados | 242.139 |
| Estrutura divergente | 0 |

Os motivos de rejeição registrados foram: 240.875 por duplicidade; 1.262 por procedimento fora da vigência (1.240 do código `41001141` e 22 do código `40314430`); e 2 por precisão excedida em `QT_ITEM_EVENTO_INFORMADO`. As ocorrências de alerta por código ausente somaram 3.344: 3.343 na TUSS 22 e 1 na TUSS 20. A ocorrência da TUSS 20 também foi rejeitada por duplicidade; por isso, foi registrada como alerta, mas seu estado final é rejeitado. Essa sobreposição explica por que existem 3.344 ocorrências de alerta e 3.343 registros classificados como válidos com alerta.

O Core informou 11,894 segundos de processamento; o tempo externo medido entre início e término do processo foi 13,579 segundos, com pico de memória residente de 3.092.572 KiB (aproximadamente 3,0 GiB). É uma única execução no ambiente registrado, não um benchmark nem uma medida generalizável. O resultado cobre o recorte estadual público do Paraná e não a base nacional. Para a análise científica, deve ser descrito como execução integral do Core nesse recorte, com versão, hashes, ambiente e a adaptação temporária de carregamento de TUSS 63 explicitados.

### Execução integral histórica no Windows

A preparação para a sabatina registra uma execução anterior da massa integral no Compaq Windows, em 09/10/2026: 7.855.323 registros processados pelo CSV com `--export`, usando os catálogos TUSS 22 e 63. O resultado foi 7.782.446 válidos, 71.613 válidos com alerta, 1.264 rejeitados e zero divergências estruturais; o tempo interno foi 514,783 s e o tempo total exibido pelo Code::Blocks foi 515,020 s. Essa evidência confirma que o equipamento concluiu uma execução integral naquela versão e configuração. O registro também aponta que a TUSS 20 não estava carregada, portanto não representa a cobertura do catálogo nem as regras da versão atual. A preparação para sabatina está registrada no documento de evidências do projeto.

### Execução integral atual no Windows (Compaq)

Em 08/11/2026, o Core processou no Compaq Windows o CSV integral do recorte PR/202501: 7.855.323 registros, 462.698.039 bytes, 11 colunas, competência janeiro/2025. O executável Release foi recompilado no Code::Blocks com GCC 14.2.0 a partir do código vigente no commit `e303f23`; o Build terminou com zero erros e um aviso sobre `-std=c++23` aplicado à unidade C do SQLite. A chamada incluiu TUSS 22 e 63 por argumento e carregou também as referências integradas TUSS 19, 20 e 60. A execução foi iniciada com `--export` para `data/sample/saida_full_windows_20261108_172622`; a verificação posterior dos arquivos exportados ainda precisa ser concluída.

| Resultado de estado | Registros |
|---|---:|
| Total | 7.855.323 |
| Válidos | 7.609.841 |
| Válidos com alerta | 3.343 |
| Rejeitados | 242.139 |
| Estrutura divergente | 0 |

Os motivos de rejeição no resumo foram: 240.875 por duplicidade; 1.262 por procedimento fora da vigência; e 2 por precisão excedida em `QT_ITEM_EVENTO_INFORMADO`. Os alertas de códigos não localizados nos snapshots TUSS 20/22 somaram 3.344 ocorrências; uma dessas ocorrências também foi rejeitada por duplicidade, por isso o total de registros válidos com alerta é 3.343.

O tempo interno informado pelo Core foi 963,538 segundos (16 min 3,538 s), aproximadamente 8152.58 registros por segundo nessa execução. As contagens de classificação coincidem com a execução integral atual no Ubuntu. Os tempos não são comparáveis como benchmark: o Windows executou com `--export`, enquanto o ensaio Ubuntu desativou as exportações e a auditoria por registro; os ambientes e condições também diferem. Foi uma execução por ambiente, sem repetição estatística.

O equipamento tem 4 GB de RAM instalada, sem slot para expansão; o Windows reconhece 3,6 GiB utilizáveis. A leitura pré-execução após reinicialização e fechamento do Copilot indicou 0,39 GiB livres. O pico de memória do processo e o uso de arquivo de paginação durante esta execução não foram medidos. Também não houve medição de energia. Para discussão de engenharia verde, os dados permitem descrever volumetria, tempo e limite do hardware; tempo de processamento isolado não demonstra consumo energético nem economia. A avaliação energética exigiria instrumentação e protocolo controlado, com medições repetidas.

A execução confirma o processamento integral do recorte estadual público PR/202501 pelo Core atual no Windows e complementa a execução atual no Ubuntu. O recorte não representa a base nacional; os resultados dependem das regras operacionais e dos snapshots TUSS versionados descritos nesta matriz.
## Regras complementares e nomenclatura

A rejeição de conteúdo duplicado compara os onze campos do registro. Essa regra complementar é identificada nos documentos do projeto como **DUP-01**. Os testes `tests/test_fluxo_validacao.py` e `tests/test_fluxo_sqlite_duplicidade.py` passaram nos dois sistemas; a documentação específica está em [VALIDACAO_DUPLICIDADE.md](VALIDACAO_DUPLICIDADE.md).

**Distinção em relação ao baseline:** RF07 significa registrar motivos de rejeição. DUP-01 identifica a detecção de registros duplicados, uma regra adicional, sem alterar a lista ou a numeração dos requisitos aceitos. Eventual incorporação de DUP-01 como novo requisito depende de aprovação do responsável pela disciplina.

## Evidência de execução cruzada

Em 02/11/2026, o Code::Blocks Release executou `data/sample/PR_202501_AMB_DET_validacao.csv` no Windows e no Ubuntu, usando caminhos relativos. Ambos retornaram 0, processaram 8 registros e exportaram em `../data/output/saida_codeblocks`. O [README](README.md) registra os totais e a delimitação dessa evidência.

As regressões `tests/test_fluxo_validacao.py`, `tests/test_fluxo_sqlite.py` e `tests/test_fluxo_sqlite_duplicidade.py` também passaram nos dois ambientes, conforme os resultados registrados na sessão de validação de 02/11/2026.

Em 08/11/2026, após os commits de documentação do perfil PR/202501, o workflow [Regressão TUSS — Linux e Windows](.github/workflows/regressao-tuss.yml) voltou a passar em `ubuntu-latest` e `windows-latest` (run [42512371868](https://github.com/mmsmarciomarcos-1974/LEGATECH-Core-CABS002/actions/runs/42512371868)). A execução rodou `tools/testar_tuss.py`; ela confirma a regressão automatizada nas duas plataformas, mas não processa a massa PR/202501 integral no Windows.

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
