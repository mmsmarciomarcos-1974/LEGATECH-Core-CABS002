# RF07: rejeicao de registros duplicados

## Regra implementada

A detecção compara os 11 valores textuais que chegam ao `RegistroTissBruto`:

`ID_EVENTO_ATENCAO_SAUDE`, `UF_PRESTADOR`, `DT_REALIZACAO`, `CD_PROCEDIMENTO`, `CD_TABELA_REFERENCIA`, `QT_ITEM_EVENTO_INFORMADO`, `VL_ITEM_EVENTO_INFORMADO`, `VL_ITEM_PAGO_FORNECEDOR`, `UNIDADE_MEDIDA`, `IND_PACOTE` e `IND_TABELA_PROPRIA`.

A comparação é feita após a leitura e normalização de cada entrada. A primeira ocorrência segue as demais validações. Uma ocorrência posterior com os mesmos 11 valores é rejeitada com o motivo:

`DUPLICIDADE: registro repetido (11 campos identicos)`

A regra vale durante o processamento de cada entrada CSV ou SQLite. A repetição isolada de `ID_EVENTO_ATENCAO_SAUDE` não caracteriza duplicidade.

## Evidências de teste

- `py tests\test_fluxo_validacao.py`: passou com o caso duplicado na massa controlada; a auditoria registra o motivo.
- `py tests\test_fluxo_sqlite.py`: passou com os 10 registros originais.
- `py tests\test_fluxo_sqlite_duplicidade.py`: passou; uma cópia temporária do banco recebeu uma linha repetida, que foi rejeitada. O banco original foi preservado.

## Limite operacional

O validador mantém em memória as chaves dos registros já processados. O consumo de memória cresce conforme a quantidade e o tamanho dos registros distintos. Os testes controlados confirmam a regra, mas ainda não demonstram o consumo de memória na massa integral. Essa capacidade precisa ser avaliada antes de afirmar que o processamento em escala está validado.