# Catálogos TUSS 22 e 63 — versão 202501

A execução pode carregar as duas referências uma vez e consultar cada par
`CD_TABELA_REFERENCIA + CD_PROCEDIMENTO`. Os códigos permanecem strings: `018`
e `18` são chaves diferentes. O motor não contém uma lista de grupos ou procedimentos.

## Fonte e conferência

Fonte: [Padrão TISS — Janeiro/2025, ANS](https://www.gov.br/ans/pt-br/assuntos/prestadores/padrao-para-troca-de-informacao-de-saude-suplementar-2013-tiss/padrao-tiss-janeiro-2025).
[Pacote oficial](https://www.ans.gov.br/arquivos/extras/tiss/Padrao_TISS_Representacao_de_Conceitos_em_Saude_202501.zip).

Arquivo `TUSS - Demais terminologias - VERSÃO 202501.xlsx`, aba `Tab 63`.
Conferência independente com o PDF de mesmo nome, páginas impressas 78–79:
31 códigos, termos e datas reconciliados. A tabela 63 identifica grupos para
envio de dados à ANS. A tabela 64 define a forma de envio e exige outra análise.

| Campo | Resultado do snapshot 63 |
| --- | --- |
| Códigos | 31 grupos, `001` a `031`, três dígitos preservados |
| Início de vigência | `2012-10-10` nos 31 grupos |
| Fim de vigência | Vazio nos 31 grupos |
| Fim de implantação | `2014-08-31` nos 31 grupos |
| `018` | PROCEDIMENTOS EM FISIOTERAPIA |
| `004` | DIAGNÓSTICO POR IMAGEM - RX |

Fim de implantação não é fim de vigência. Não foi criada uma data de término.
O CSV normalizado e seu manifesto estão em `data/reference/`.

SHA-256 do ZIP oficial: `e4f7ee54d0e06c6c7f5a3374fb8cd8bedd67069e6c44f1d05b03a89298f30275`.
SHA-256 do PDF conferido: `11789c1bfbc6cf1ebc27b5ef678c2eb35047e2df0f85fd25bdb857f0c34b9df2`.
Os hashes do XLSX e do catálogo constam no manifesto. Eles identificam arquivos
conferidos; não constituem assinatura digital da ANS.

## Execução no Windows

Os três argumentos anteriores continuam aceitos. Sem o quarto argumento,
a tabela 63 continua gerando `CATALOGO_NAO_CARREGADO`.
Para carregar os dois catálogos, após compilar Release:

```powershell
New-Item -ItemType Directory -Force .\data\output | Out-Null
.\LEGATECH_Core\bin\Release\LEGATECH_Core.exe .\data\sample\PR_202501_AMB_DET_sample.csv .\data\reference\tuss22_202501.csv .\data\output\auditoria_tiss.db .\data\reference\tuss63_202501.csv
```

Resultado esperado da amostra positiva: **10 / 10 / 0 / 0 / 0**
(total / válidos / alertas / rejeitados / estrutura divergente).
Com `PR_202501_AMB_DET_sample_rejeitado.csv`: **10 / 9 / 0 / 1 / 0**.
O motivo de UF ausente permanece. As execuções anteriores permanecem no banco.
A nova execução grava `versao_catalogo=202501` e `tabela_catalogo=22,63`.
O esquema SQLite permanece na versão 1: o campo textual passa a admitir uma
lista explícita das tabelas carregadas, na ordem informada ao construtor.

Para conferir, consultando o banco somente para leitura:

```powershell
@'
import sqlite3
from contextlib import closing
with closing(sqlite3.connect("file:data/output/auditoria_tiss.db?mode=ro", uri=True)) as db:
    print(db.execute("SELECT id,tabela_catalogo,total,validos,alertas,rejeitados FROM auditoria_execucao ORDER BY id").fetchall())
'@ | python -
```

## Reproduzir e testar

Depois de extrair o XLSX do pacote oficial, sem editar ou salvar novamente:

```powershell
python tools\normalizar_tuss63.py "C:\caminho\TUSS - Demais terminologias - VERSÃO 202501.xlsx" data\reference
python tools\testar_tuss.py
```

O importador usa somente a biblioteca padrão do Python e verifica o hash do
XLSX conferido antes de extrair. Outro arquivo não pode ser rotulado como
202501 por esse importador. O formato de saída é o mesmo CSV de oito colunas
usado para a tabela 22; descrição detalhada fica vazia porque não existe na aba 63.

A suíte verifica carga integral (5.964 + 31), isolamento dos pares, zeros,
vigência de grupos em referência sintética, tabela própria/pacotes,
catálogo ausente/incorreto/repetido, auditoria com os dois catálogos e rollback.
Sem o catálogo 63, os resultados anteriores continuam 10/7/3/0/0 e 10/6/3/1/0.

Validação automatizada realizada em Linux. A mudança ainda precisa da execução
no Windows/Code::Blocks. Não foi medida a massa completa de 7.855.323 registros.
Código ausente no snapshot continua alerta; ausência não prova invalidez histórica.
A carga conjunta falha integralmente antes da leitura da massa se uma referência
estiver incorreta. Os hashes ainda não são registrados por execução no banco.
