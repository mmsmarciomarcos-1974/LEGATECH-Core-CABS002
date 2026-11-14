# Preparação do Core para disponibilização junto ao artigo

## Escopo da revisão de 14 de novembro de 2026

Referência examinada: commit `9e669b1bb72c9feda4491fe340b1270d2a95d83f`, branch main. A revisão é preliminar e não constitui certificação de segurança, anonimização ou direitos de distribuição.

Foram examinados 79 arquivos textuais com menos de 100.000 bytes, incluindo código próprio, testes, documentação, manifestos e amostras pequenas. A busca por padrões de chaves privadas, tokens, atribuições de credenciais e referências corporativas selecionadas não encontrou correspondências. Essa busca não detecta todos os formatos de segredo nem substitui a revisão humana.

Foram consultados os 102 commits retornados na paginação do histórico de main e examinados 334 patches disponíveis. Dezessete entradas de arquivos não ofereceram patch; seus conteúdos históricos não foram examinados por esse procedimento. Outras branches e objetos não alcançáveis a partir de main não foram auditados. Os catálogos grandes e os arquivos completos da distribuição SQLite ficaram fora da varredura textual atual.

## Dados e amostras

O banco `data/legatech.db` foi aberto somente para leitura. Contém uma tabela `registro_tiss`, onze colunas TEXT e dez linhas, com nove identificadores de evento distintos. O tamanho pequeno e a estrutura não comprovam anonimização nem origem sintética.

As amostras CSV contêm identificadores de eventos e variantes com campos alterados para testes. A massa `PR_202501_AMB_DET_validacao.csv` utiliza identificadores controlados e inclui um registro repetido e um registro com campo extra. A procedência de cada amostra e sua adequação à redistribuição precisam ser documentadas antes da abertura. Não presumir que todos os exemplos são sintéticos.

Os manifestos registram referências TUSS e hashes. A existência desses manifestos não comprova, por si só, permissão de redistribuição dos catálogos.

## Correções preparadas

- Separação da entrada malformada `Thumbs.dbdata/sample/PR_201501_AMB_DET.csv` no .gitignore, mantendo a regra própria para a massa histórica.
- Atualização do trecho de ENTRADA_DAO.md que ainda apresentava exportação e auditoria como pendentes.
- Ligação deste registro no README.

Não foram modificados algoritmos, massas, classificações ou resultados históricos.

## Versões associadas aos resultados

| Ensaio | Commit documentado |
|---|---|
| Integral Windows de 08/11/2026 | `e303f2306bb3fd1155c973edaccd365babc21562` |
| Integral Ubuntu documentada | `6a63514312482df1439a76d20d4b30b633ba39cd` |

Consultar a matriz de evidências para configurações e limites. A execução Ubuntu envolveu uma adaptação temporária do harness para carregar TUSS 63, que deve ser explicitada no roteiro de reprodução. As opções de exportação/auditoria diferem; os tempos não são comparação direta entre plataformas.

## Pendências para publicação

1. Completar a revisão dos dados, catálogos, arquivos históricos sem patch e demais branches que serão expostas.
2. Documentar a origem das amostras e decidir se os exemplos distribuídos devem ser substituídos por massas inteiramente sintéticas, preservando as evidências históricas.
3. Definir uma licença para o código próprio e separar os termos das dependências e dos dados externos.
4. Consolidar um roteiro reproduzível de compilação, teste e execução, incluindo as condições do ensaio integral.
5. Preparar CITATION.cff e uma release com versão e commit definidos após o fechamento.
6. Autorizar explicitamente a mudança de visibilidade. A preparação documental não altera a privacidade do repositório.

## Verificação automatizada da preparação

A execução GitHub Actions 43603140452, associada ao commit d6c548427962040a09c0eb7ed96e2426933f2ad9, concluiu com sucesso os jobs Regressão em ubuntu-latest e Regressão em windows-latest. Trata-se da suíte automatizada; não foi uma execução integral dos 7.855.323 registros.

Link: https://github.com/mmsmarciomarcos-1974/LEGATECH-Core-CABS002/actions/runs/43603140452

A leitura alternativa do blob da TUSS 20 falhou por encerramento do transporte. O acesso por fetch_file também retornou conteúdo vazio. A divergência foi resolvida posteriormente pela contagem local e pelo commit 0b53db1 na main, conforme atualização abaixo. A regressão aprovada não resolve essa divergência: o harness consultado não contém uma conferência explícita da quantidade 41.416 versus o campo records desse manifesto.

Para conferência local, usar o trecho somente de leitura em REPRODUCAO_ARTIGO.md. Não atualizar hashes ou contagens por suposição e não alterar resultados históricos. Esta atualização é documental.

## Resolução da contagem TUSS 20

Em 14/11/2026, o autor executou a leitura local com csv.reader, separador ponto e vírgula, codificação utf-8-sig e newline vazio. Após excluir o cabeçalho e ignorar linhas vazias, foram contados 41.416 registros. O diff mostrou somente records: 41424 → 41416 no manifesto, preservando o SHA-256 registrado. A correção foi enviada à main no commit 0b53db1. O catálogo não foi alterado nessa correção; o hash não foi recalculado por esse procedimento. A procedência exata da exportação e os termos de distribuição continuam como verificações independentes.
