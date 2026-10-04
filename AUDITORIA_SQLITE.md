# Auditoria de validacao TISS em SQLite

A validacao de `CD_TABELA_REFERENCIA + CD_PROCEDIMENTO` continua usando o
catalogo externo TUSS 22 / 202501, carregado uma vez. A auditoria passa a
preservar os resultados que antes apareciam apenas no console.

## Executar no Windows

Na raiz do repositorio, apos **Rebuild** no Code::Blocks:

```bat
if not exist data\output mkdir data\output
LEGATECH_Core\bin\Debug\LEGATECH_Core.exe data\sample\PR_202501_AMB_DET_sample.csv data\reference\tuss22_202501.csv data\output\auditoria_tiss.db
```

O terceiro argumento e o banco de auditoria. O diretorio precisa existir;
o arquivo `.db` e criado se necessario. Sem esse argumento, a execucao
continua exibindo somente a validacao no console.
`data/output/` ja esta ignorado pelo Git; mantenha os bancos gerados nessa pasta.

Uma execucao confirmada exibe `Auditoria confirmada: execucao ...`.
Falhas de abertura, cabecalho, leitura ou gravacao retornam codigo 1.

## Dados preservados

| Tabela | Conteudo |
| --- | --- |
| `auditoria_execucao` | Identificador, horarios UTC, caminho do CSV, competencia, nome do contrato, versao/tabela do catalogo carregado, cabecalho e contagens finais. |
| `auditoria_resultado` | Uma linha por registro nao vazio: numero fisico, texto recebido, ID do evento, tabela, procedimento, estado e indicador de estrutura divergente. |
| `auditoria_ocorrencia` | Todos os motivos e alertas, ordenados por tipo e vinculados ao resultado. Um rejeitado pode ter ambos. |

O numero fisico conta o cabecalho como linha 1 e inclui linhas vazias na
numeracao, embora elas nao sejam registros processados. O texto original
preserva aspas e o `CR` de arquivos CRLF; o delimitador final `LF` nao e
armazenado. A validacao usa uma copia sem o `CR` final. Codigos sao textos,
preservando zeros a esquerda.

Linhas com quantidade incorreta de campos recebem `REJEITADO` e motivo
`ESTRUTURA_DIVERGENTE_DO_CONTRATO`. Nesse caso, ID, tabela e procedimento
ficam `NULL`, pois a estrutura nao permite atribuir esses campos com
seguranca; a linha original permanece disponivel para investigacao.

O contrato historico de 10 campos e o atual de 11 campos usam o mesmo fluxo.
`ProcessadorTiss` tambem passa a receber a competencia quando chamado pelo
CSV. A interface de auditoria e opcional e independente da saida operacional.

## Consultar

Abra o banco em um cliente SQLite e execute:

```sql
SELECT id, competencia, versao_catalogo, tabela_catalogo,
       total, validos, alertas, rejeitados, estrutura_divergente
FROM auditoria_execucao
ORDER BY id;

SELECT r.execucao_id, r.linha_fisica, r.id_evento,
       r.cd_tabela_referencia, r.cd_procedimento, r.estado,
       o.tipo, o.ordem, o.mensagem
FROM auditoria_resultado AS r
LEFT JOIN auditoria_ocorrencia AS o ON o.resultado_id = r.id
WHERE r.execucao_id = (SELECT MAX(id) FROM auditoria_execucao)
ORDER BY r.linha_fisica, o.tipo, o.ordem;
```

Na amostra positiva, o banco deve registrar **10 / 7 / 3 / 0 / 0**
(total / validos / alertas / rejeitados / estrutura divergente).
Na amostra com UF vazia, deve registrar **10 / 6 / 3 / 1 / 0**.
Cada reexecucao acrescenta outro identificador; nao substitui resultados.

## Garantias e limites

- Uma conexao e comandos preparados sao reutilizados. O processamento
  continua lendo uma linha por vez, sem acumular a massa em memoria.
- A auditoria usa uma transacao por arquivo. Erro de leitura ou de gravacao
  cancela os resultados daquela execucao; execucoes confirmadas permanecem.
  Uma tentativa cancelada nao deixa uma execucao registrada no banco.
- O resumo e a mensagem de confirmacao so aparecem depois do `COMMIT`.
  O esquema vazio pode existir mesmo quando a entrada e rejeitada.
- As tabelas de auditoria nao inserem dados em `registro_tiss`. O DAO
  operacional legado continua gravando apenas `VALIDO`. Se ele for usado
  junto com a auditoria, suas gravacoes nao participam desta transacao.
- A versao/tabela registrada identifica o catalogo carregado; nao afirma
  que outras tabelas, como 63, tenham sido verificadas nesse catalogo.
- Ainda nao ha hash do arquivo/catalogo nem identificacao automatica do
  commit do motor no banco. Os metadados nao comprovam identidade binaria
  dos arquivos usados. A ausencia no snapshot continua sendo alerta.
- A leitura AMB_DET permanece baseada em linhas fisicas e campos separados
  por ponto e virgula. Campos CSV com ponto e virgula interno ou quebra de
  linha dentro de aspas exigem uma evolucao especifica do parser de entrada.
- Uma transacao extensa ocupa disco e mantem bloqueio de escrita durante
  a carga. O desempenho e o tamanho do banco para a massa completa de
  7.855.323 registros precisam ser medidos no ambiente do usuario.

## Verificar

```bat
python tools\testar_tuss.py
```

O teste usa bancos temporarios e verifica os tres estados, pares tabela/codigo,
metadados, motivos/alertas, reexecucao, contrato historico, CRLF, linhas vazias,
quarentena estrutural, preservacao de zeros e rejeitados com alerta.
Uma falha SQLite e provocada depois de gravacoes para verificar o rollback;
tambem sao exercitados reuso da conexao e cancelamento pelo destrutor.
Os bancos e as massas originais nao sao modificados.
