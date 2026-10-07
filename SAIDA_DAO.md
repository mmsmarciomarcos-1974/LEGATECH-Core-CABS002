# Exportacao dos resultados por DAO

O Core aceita `--export pasta_nova` ao final dos argumentos. A pasta pai deve existir; a pasta de resultados deve ser nova. Os formatos sao gravados durante a mesma leitura e publicados juntos ao concluir o processamento. As sintaxes anteriores, inclusive auditoria SQLite isolada, continuam disponiveis.

Na raiz do repositorio, apos compilar:

```bat
LEGATECH_Core\bin\Release\LEGATECH_Core.exe data\sample\PR_202501_AMB_DET_sample_rejeitado.csv data\reference\tuss22_202501.csv --export resultado_csv
```

Para carregar tambem os grupos da tabela 63:

```bat
LEGATECH_Core\bin\Release\LEGATECH_Core.exe data\sample\PR_202501_AMB_DET_sample_rejeitado.csv data\reference\tuss22_202501.csv data\reference\tuss63_202501.csv --export resultado_22_63
```

Entrada relacional existente com onze colunas TEXT em `registro_tiss`:

```bat
LEGATECH_Core\bin\Release\LEGATECH_Core.exe --sqlite entrada.db 202501 data\reference\tuss22_202501.csv --export resultado_sqlite
```

Cada pasta contem:

| Arquivo | Conteudo |
|---|---|
| aceitos.csv | Registros validos e validos com alerta; estado e alertas explicitos |
| rejeitados.csv | Registros rejeitados, todos os motivos e eventuais alertas |
| contexto.csv | Origem, competencia, contrato, versao/tabelas do catalogo e cabecalho |
| resultados.db | Auditoria da mesma execucao, resumo, registros e ocorrencias |

Os CSVs usam UTF-8, separador ponto e virgula e aspas duplicadas para escapar aspas. Os onze campos normalizados permanecem textuais, com zeros iniciais preservados. As colunas MOTIVOS_JSON e ALERTAS_JSON contem arrays JSON, preservando ordem e limites de cada mensagem. LINHA_ORIGINAL preserva a linha fisica CSV (inclusive CR quando presente). Uma estrutura rejeitada recebe campos normalizados vazios; a linha original e os motivos continuam disponiveis.

Na entrada SQLite, LINHA_ORIGINAL e uma serializacao CSV dos onze campos lidos, pois nao existe uma linha fisica CSV original. LINHA_ORIGEM corresponde ao ordinal da consulta mais um, compativel com o esquema de auditoria existente; nao e chave primaria nem identificador permanente. A consulta nao promete ordem entre execucoes. SQL NULL segue a normalizacao existente para texto vazio.

No SQLite de saida, `auditoria_resultado.estado='REJEITADO'` separa rejeitados; `auditoria_ocorrencia` guarda motivos e alertas associados. `linha_original` permite recuperar todos os campos da origem; as colunas de identificacao permitem localizar os resultados. O banco de saida e de auditoria, nao deve ser usado diretamente como entrada `registro_tiss`.

## Classes e polimorfismo para a sabatina

`DAOSaidaValidacao` declara o contrato de iniciar, gravar resultado, finalizar e cancelar. `AuditoriaValidacao` e um alias de compatibilidade para esse mesmo contrato. `CsvSaidaValidacao` implementa CSV; `SQLiteAuditoriaValidacao` implementa o banco; `SaidasValidacao` coordena ambas e publica a pasta. A interface `DAOSaida` anterior continua sendo usada pelo fluxo operacional de registros brutos; o novo contrato recebe tambem classificacao e ocorrencias.

O ponteiro `DAOSaidaValidacao*` chama metodos virtuais de objetos concretos distintos. Na analogia com COBOL, o chamador depende de um contrato, como ao usar CALL e um leiaute de parametros, mas o despacho virtual e um mecanismo proprio do C++. O `.h` declara atributos e metodos; o `.cpp` implementa cada responsabilidade. As regras de negocio continuam no validador, nao sao repetidas pelos DAOs.

## Falhas e verificacao

Os formatos sao preparados numa pasta temporaria irma do destino. Os arquivos e o banco sao fechados antes da publicacao por rename. Falhas capturadas removem essa pasta e nao publicam resultados. Destinos existentes sao recusados. Uma interrupcao abrupta do processo pode deixar uma pasta `.tmp-*`, que nao representa uma exportacao confirmada; nao ha promessa de durabilidade contra queda de energia.

```bat
python tools\testar_tuss.py
```

A regressao compara estados, codigos, linhas originais e todas as ocorrencias nos dois formatos. Inclui contrato historico, estrutura divergente, rejeitado com alerta, banco vazio, zeros, caracteres especiais, protecao das entradas/destinos e cancelamento apos falha real de gravacao SQLite. A entrada continua processada em streaming; nao acumula a massa inteira em memoria.

Esta etapa atende a exportacao funcional em dois formatos por DAO. A conclusao integral da entrega depende ainda da revisao dos demais requisitos e da validacao da massa completa. A compilacao Windows/Code::Blocks desta alteracao deve ser confirmada antes do merge.
