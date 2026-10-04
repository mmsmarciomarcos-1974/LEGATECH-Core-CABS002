# Validação externa da TUSS 22 — 03/10/2026

Implementação sobre o ZIP fornecido de `LEGATECH-Core-CABS002`, branch indicado pelo nome do arquivo: `main`. Não foi possível verificar o commit remoto nem enviar alterações ao GitHub.

## O que mudou

`CatalogoTuss` carrega os 5.964 termos externos uma vez, antes da leitura da massa. A versão `202501` e a tabela `22` são verificadas no cabeçalho de cada registro normalizado. As consultas usam `std::unordered_map`; os códigos continuam como texto, preservando zeros. Não há lista de procedimentos no C++.

`ValidadorRegistroTiss` recebe um ponteiro constante ao catálogo. A mesma dependência chega a `CsvDAOEntrada`, `SQLiteDAOEntrada` e `ProcessadorTiss`. O catálogo deve sobreviver a esses objetos. Chamadas antigas permanecem possíveis, mas uma tabela TUSS sem catálogo gera alerta explícito.

O domínio de tabelas e as regras existentes de `00`, `90`, `98`, `IND_PACOTE` e `IND_TABELA_PROPRIA` foram preservados. Para tabela padronizada com indicação de tabela própria, registra-se alerta e não se presume que o código seja TUSS.

Na tabela 22, código localizado permite conferir as datas de vigência; código não localizado gera alerta, porque um snapshot não comprova cobertura de todo o histórico. Nas tabelas 19, 20 e 63, registra-se ausência de catálogo específico. Passar nesta regra não significa validação integral do TISS.

O período vem de `DT_REALIZACAO`, no formato `AAAA-MM` efetivamente usado pelo código. Se a vigência fica totalmente fora desse mês, o registro é rejeitado. Sobreposição parcial gera alerta. Fim de vigência no último dia do mês também gera alerta: a inclusão da fronteira não foi presumida. A data de fim de implantação não é usada como fim de vigência.

Os contratos históricos (10 campos até 2022) e atuais (11 campos a partir de 2023) mantêm a normalização existente. O catálogo 202501 é uma referência explicitamente selecionada; não vira automaticamente uma versão histórica correspondente a cada arquivo.

## Arquivos de referência

`data/reference/tuss22_202501.csv`: UTF-8, ponto e vírgula, oito colunas e datas ISO. É diferente do CSV AMB_DET e tem leitor próprio, incluindo aspas duplicadas e campos multilinha. Cabeçalho divergente, arquivo vazio, data inválida, duplicidade e intervalo invertido interrompem a carga.

`data/reference/manifest.json`: origem, contagem e hashes SHA-256 do arquivo original e do normalizado. Os hashes documentam a transformação; não são uma assinatura da ANS e não são verificados automaticamente em runtime. Para auditoria de uma execução, preservar também esse manifesto. O aplicativo informa versão, tabela e contagem; alertas de consulta/vigência incluem versão e código.

`tools/normalizar_tuss22.py`: conversor específico da edição 202501, com versão fixa deliberada. Novas edições exigem revisar a versão e a fonte, sem sobrescrever silenciosamente o histórico.

## Como executar

Abra `LEGATECH_Core/LEGATECH_Core.cbp` no Code::Blocks e recompile. A lista de fontes agora inclui o catálogo, os contratos históricos/atuais e o seletor de dicionário. Foi adicionada a opção `-std=c++23`.

Com diretório de execução `LEGATECH_Core`, o padrão é a amostra original e `../data/reference/tuss22_202501.csv`. Também aceita caminhos explícitos:

```sh
legatech caminho/PR_202501_AMB_DET.csv caminho/tuss22_202501.csv
```

Para regressão reproduzível, na raiz do projeto:

```sh
python tools/testar_tuss.py
```

Requer Python, GCC e G++ no PATH. O script usa diretórios temporários e não altera as massas nem o banco original.

## Verificação e limites

Compilação e regressão realizadas em Linux/G++ 13.3.0, com C++23. A amostra original retorna 10 processados, 7 válidos, 3 com alerta (tabela 63 sem catálogo), 0 rejeitados. A amostra de UF vazia retorna 6 válidos, 3 com alerta e 1 rejeitado. A antiga contagem de 10 válidos muda porque agora a cobertura incompleta é visível.

Os testes também cobrem código ausente, preservação de zeros, início futuro, mês inicial completo, fim de vigência, outros catálogos, pacotes, tabela própria, períodos inválidos, falhas de carga, CSV citado/multilinha, processador, releitura SQLite e contrato histórico em CSV. A base de 7.855.323 linhas não estava no ZIP e não foi reprocessada. A execução Windows/Code::Blocks precisa ser confirmada no ambiente do usuário.

O comportamento já existente de `ProcessadorTiss` grava somente `VALIDO`; registros com alerta continuam fora da saída. Portanto, ao usar persistência, os alertas de catálogo incompleto terão impacto na quantidade gravada. Uma política de persistir registros com alerta deve ser decidida separadamente.

## Aplicar no Git

O ZIP contém o projeto atualizado e `alteracoes_tuss22.patch`. Em um checkout limpo correspondente ao ZIP de origem, faça `git apply --check` antes de aplicar o patch. Se o branch avançou, revise os conflitos em vez de substituir o projeto inteiro. Nenhum push ou commit remoto foi feito.
