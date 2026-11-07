# Catálogo TUSS 60 — competência 202501

## Fonte e recorte

Fonte: [Padrão TISS — Janeiro/2025, componente de Representação de Conceitos em Saúde](https://www.gov.br/ans/pt-br/assuntos/prestadores/padrao-para-troca-de-informacao-de-saude-suplementar-2013-tiss/padrao-tiss-janeiro-2025). A TUSS 60 é a terminologia de unidade de medida; a aba `Tab 60` da planilha oficial contém os campos de código, termo, descrição e vigência.

O pacote oficial `Padrao_TISS_Representacao_de_Conceitos_em_Saude_202501.zip` tem SHA-256 `e4f7ee54d0e06c6c7f5a3374fb8cd8bedd67069e6c44f1d05b03a89298f30275`, igual ao arquivo de origem já registrado para a TUSS 19. A planilha `TUSS - Demais terminologias - VERSÃO 202501.xlsx` tem SHA-256 `2608c3a4002b815c29a8e3ecfa15e91e045a10d11d5aa998e8899738927a0978`.

A aba contém **61 códigos únicos**, todos iniciados até a competência 202501 e sem data de fim de vigência. Os códigos são mantidos como texto de três dígitos para preservar zeros à esquerda. A data de fim de implantação é preservada como metadado; ela não é tratada como fim de vigência. O arquivo compacto e seu manifesto estão em `data/reference/tuss60_202501/`.

## Reprodução

Com `openpyxl` instalado, gere o CSV e o manifesto a partir do pacote oficial:

```bash
python tools/normalizar_tuss60.py "Padrao_TISS_Representacao_de_Conceitos_em_Saude_202501.zip"
```

O normalizador confere o hash do pacote, o nome e título da planilha, o cabeçalho, os códigos, datas e duplicidades, além da quantidade esperada de 61 termos. O manifesto registra hashes da fonte, planilha e saída.

## Uso pelo Core

O `main.cpp` carrega a TUSS 60 automaticamente junto aos catálogos 19, 20 e 22; a tabela 63 permanece opcional. Para `UNIDADE_MEDIDA`, o Core aceita vazio porque o extrato não permite determinar se cada item exige unidade. Quando preenchido, exige código numérico de três dígitos, valida o domínio e verifica a vigência em relação ao mês de realização. Código fora do domínio completo da tabela 60 é rejeitado; se o catálogo não estiver disponível em uso direto do validador, o resultado recebe alerta de catálogo não carregado.

A validação de obrigatoriedade condicional continua delimitada ao que o extrato público informa. A validação integral de mensagens e guias TISS fica fora deste escopo.
