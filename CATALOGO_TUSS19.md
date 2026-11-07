# Catálogo TUSS 19 — competência 202501

## Fonte oficial

O pacote foi obtido na página oficial da ANS [Padrão TISS — Janeiro/2025](https://www.gov.br/ans/pt-br/assuntos/prestadores/padrao-para-troca-de-informacao-de-saude-suplementar-2013-tiss/padrao-tiss-janeiro-2025), na linha **Componente de Representação de Conceitos em Saúde**, versão `202501`.

Arquivo de origem: `Padrao_TISS_Representacao_de_Conceitos_em_Saude_202501.zip`  
SHA-256 do pacote recebido: `e4f7ee54d0e06c6c7f5a3374fb8cd8bedd67069e6c44f1d05b03a89298f30275`

O pacote contém duas planilhas TUSS 19:

| Parte | SHA-256 do XLSX |
|---|---|
| Parte 1 | `83137a59bc54c4e9ac7f8cea1d77fa8c51cc4ffaabdc69149496affcab5580e6` |
| Parte 2 | `ec7d14026bc23c0cbcddff6b216f30ea4d224e397afebd287fe8a2f8efb8fa6a` |

## Normalização

O script `tools/normalizar_tuss19.py` lê as duas planilhas oficiais e gera a referência compacta em `data/reference/tuss19_202501/`:

```bash
python tools/normalizar_tuss19.py "Padrao_TISS_Representacao_de_Conceitos_em_Saude_202501.zip"
```

O script requer `openpyxl`, valida o identificador 202501, os cabeçalhos, códigos numéricos de 8 ou 9 dígitos, datas iniciais, duplicidades e quantidade total. A saída mantém somente código, início de vigência e fim de vigência, que são os dados usados pela consulta do Core. As descrições extensas não são carregadas.

O snapshot contém **1.292.182 códigos únicos**: 909.640 com 8 dígitos e 382.542 com 9 dígitos. Todos possuem início de vigência preenchido e fim de vigência vazio. Os registros estão divididos em sete CSVs para facilitar a distribuição; `manifest.json` registra a origem, o hash do pacote, a quantidade de cada parte e seu SHA-256.

## Uso pelo Core

O `main.cpp` procura as sete partes em `data/reference/tuss19_202501/` e carrega a TUSS 19 automaticamente junto às tabelas 20 e 22; a tabela 63 continua opcional conforme o comando. Se nenhuma parte estiver presente, o Core mantém o comportamento de catálogo não carregado. Se apenas parte do conjunto existir, a execução falha antes do processamento para impedir uma consulta parcial silenciosa.

O carregador confere o número esperado de registros de cada parte, a estrutura compacta, o tamanho do código, as datas e duplicidades. O validador aceita para TUSS 19 códigos numéricos com 8 ou 9 dígitos. Código ausente do snapshot gera `CODIGO_NAO_LOCALIZADO_NO_CATALOGO` como alerta; não é rejeitado só por não aparecer na referência.

## Testes

`tools/testar_tuss.py` verifica os hashes e as quantidades das sete partes. `tests/test_catalogo_tuss.cpp` carrega a referência completa e cobre código localizado e vigente, código ausente com alerta e formatos de código fora do padrão observado. A suíte exercita também a carga automática usada pelos fluxos CSV e SQLite.
