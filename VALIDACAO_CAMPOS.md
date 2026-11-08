# RF04: validacao dos campos do AMB_DET

Esta etapa acrescenta limites textuais e precisao ao ValidadorRegistroTiss, compartilhado pelos DAOs CSV e SQLite. Nao conclui toda a obrigatoriedade condicional nem todos os dominios.

## Fonte consultada

ANS, Dicionario_de_variaveis_a_partir_de_2023.ods, abas Ambulatorial_DET e Informacoes_gerais, consultado em 07/10/2026:
https://dadosabertos.ans.gov.br/FTP/PDA/TISS/DICIONARIO/Dicionario_de_variaveis_a_partir_de_2023.ods

SHA-256 do arquivo consultado: `5711a4c1c9598f40488301e6f603cde066bb59be16e158615aa8ef8e5260eb76`.

O indice oficial informa atualizacao em 25/08/2026. Essa consulta nao comprova que o arquivo seja identico ao dicionario publicado na competencia 202501. A aplicabilidade historica e a escala da quantidade devem ser confrontadas com a documentacao preservada e a massa real antes de declarar conformidade integral.

## Regras acrescentadas

| Campo | Referencia do dicionario | Regra desta etapa |
|---|---|---|
| ID_EVENTO_ATENCAO_SAUDE | NUMBER, tamanho 30 | Ate 30 digitos; obrigatoriedade e formato anteriores preservados |
| CD_PROCEDIMENTO | VARCHAR2(40) | Ate 40 caracteres; consulta ao par TUSS anterior preservada |
| CD_TABELA_REFERENCIA | VARCHAR2(40) | Ate 40 caracteres; dominio anterior preservado |
| QT_ITEM_EVENTO_INFORMADO | NUMBER, tamanho 6 | Ate seis digitos de precisao, mantendo decimais sem presumir escala fixa |
| VL_ITEM_EVENTO_INFORMADO | NUMBER, tamanho 18,2 | Obrigatorio, exceto componente de pacote; nao negativo; ate 16 digitos inteiros e duas casas decimais |
| VL_ITEM_PAGO_FORNECEDOR | NUMBER, tamanho 18,2 | Obrigatorio, exceto componente de pacote; zero sem pagamento; negativo permitido para estorno, com alerta |
| UNIDADE_MEDIDA | VARCHAR2(40), referencia TUSS 60 | Opcional no extrato; quando preenchida, ate 40 caracteres e codigo TUSS 60 numerico de tres digitos, validado por dominio e vigencia |

O formato decimal exige digitos antes e depois do separador quando ele existe. Aceita ponto ou virgula, nunca ambos nem mais de um separador. Recusa `.5`, `5.`, notacao cientifica e NaN. Sinais sao recusados, exceto o sinal negativo de `VL_ITEM_PAGO_FORNECEDOR`, que pode representar estorno. Nao aplica agrupamento de milhares, conversao binaria ou arredondamento silencioso. Zeros iniciais sao preservados, mas nao aumentam a precisao numerica. O limite textual conta caracteres UTF-8 em entradas bem formadas; nao constitui validacao completa de codificacao UTF-8.

### Regra fechada para `QT_ITEM_EVENTO_INFORMADO`

O escopo aqui e o extrato tabular publico `AMB_DET`, nao a validacao integral de uma mensagem TISS ou de uma guia. No dicionario consultado, o campo e `NUMBER`, tamanho 6; a fonte nao declara escala decimal fixa, faixa minima/maxima nem obrigatoriedade para esse campo. Portanto, o Core preserva valores vazios e nao importa para esse extrato regras de preenchimento de uma mensagem/guias especificas.

Quando preenchido, o valor deve ser decimal nao negativo com no maximo seis digitos de precisao total, contando os digitos fracionarios. A escala nao e fixa: `1234.56` e `0.123456` sao aceitos; `12345.67` e `0.1234567` excedem o limite. Zero e aceito porque o dicionario do extrato nao define minimo; sinais, notacao cientifica e formatos incompletos continuam rejeitados. O separador pode ser ponto ou virgula. Essa e uma interpretacao operacional explicita do tamanho 6, nao uma escala publicada pela ANS.

As mensagens TISS podem definir obrigatoriedade, tipo ou faixa diferentes para campos semanticamente relacionados conforme o componente e a guia. Por exemplo, o [modelo ANS do componente de utilizacao publicado em 2023](https://fhir-hm.ans.gov.br/componente_itens_assistenciais.html) declara quantidade inteira obrigatoria e maior que zero naquele caso de uso. Essa regra nao e transferida ao `AMB_DET` sem prova de equivalencia entre os campos e contextos. A validacao de mensagens/guias fica fora do escopo desta regra.

### Regras fechadas para os valores do item

Para os campos do item do extrato `AMB_DET`, a semantica dos nomes corresponde aos campos de valor por item das guias TISS. O [modelo ANS de SPSADT, baseado no CCE 202207 e publicado no guia 202309](https://fhir-hm.ans.gov.br/guia_spsadt.html), define `Valor informado de procedimentos ou itens assistenciais` como obrigatorio e maior ou igual a zero. Define `Valor pago diretamente ao fornecedor` como obrigatorio; quando nao houver pagamento, deve ser informado zero. O [Componente Organizacional da ANS](https://www.gov.br/ans/pt-br/arquivos/assuntos/prestadores/padrao-para-troca-de-informacao-de-saude-suplementar-tiss/padrao-tiss/PadroTISS_ComponenteOrganizacional_202203.pdf), no exemplo de envio de pacotes (Quadro 18), mostra componentes com valores informados vazios. No Core, esses vazios sao aceitos quando `IND_PACOTE=1`, que identifica o contexto de componente de pacote no extrato; fora dele, os dois valores sao obrigatorios.

A precisao vem do dicionario do extrato (`NUMBER(18,2)`): ate 16 digitos inteiros e duas casas decimais. Zero e valido para ambos os campos. Valor informado negativo e rejeitado. A regra 234 do Componente Organizacional da ANS permite valor negativo no campo pago para registrar estorno, desde que a soma dos valores pagos do item ou da guia nao fique negativa. A regra nao nomeia especificamente o campo de pagamento direto ao fornecedor; por isso, aceitar sinal negativo em `VL_ITEM_PAGO_FORNECEDOR` e uma interpretacao conservadora dessa regra geral de estorno, marcada com alerta. O saldo acumulado e a justificativa do estorno nao podem ser confirmados em uma linha isolada. A validacao local tambem nao verifica somatorios ou igualdade entre lancamentos da mesma guia. O CCE 202207 e a referencia semantica consultada; nao declaramos conformidade integral de cada mensagem/guia ou de todas as versoes do TISS.

### Regras fechadas para os indicadores de pacote e tabela propria

Os indicadores continuam opcionais quando o registro nao permite inferir seu contexto. As combinacoes verificaveis no extrato sao:

| `CD_TABELA_REFERENCIA` | Condicao aplicada |
|---|---|
| `00` | Exige `IND_TABELA_PROPRIA=1`. Pode ter `IND_PACOTE=1` quando o codigo proprio identifica componente de pacote. |
| `90` ou `98` | Exige `IND_PACOTE=1`, pois a referencia identifica tabela propria de pacote. O valor de `IND_TABELA_PROPRIA` permanece opcional: nao se infere sua obrigatoriedade adicional para essas referencias. |
| TUSS suportada (`19`, `20`, `22` ou `63`) | `IND_TABELA_PROPRIA=1` e rejeitado por contradizer a referencia padronizada. `IND_PACOTE=1` e permitido para componente TUSS de pacote; `0` ou vazio tambem sao aceitos. |

A ANS define `00` como tabela propria das operadoras e `90`/`98` como tabelas proprias de pacotes (item 84). O item 81 estabelece que os itens de um pacote devem possuir codigo TUSS ou codigo de tabela propria; o Quadro 18 exemplifica um pacote `98` com um componente em TUSS `22`, inclusive com valores vazios no componente. Portanto, nao se exige que todo componente de pacote use `90`/`98`. Fonte: [Componente Organizacional do Padrao TISS, janeiro de 2025, itens 81 e 84 e Quadro 18](https://www.gov.br/ans/pt-br/assuntos/prestadores/padrao-para-troca-de-informacao-de-saude-suplementar-2013-tiss/PadroTISS_ComponenteOrganizacional_202501.pdf). A associacao desses codigos aos indicadores `IND_PACOTE` e `IND_TABELA_PROPRIA` no extrato `AMB_DET` continua sendo uma inferencia operacional, nao uma regra explicita de preenchimento dos indicadores nesse documento; a validacao integral de mensagens/guias permanece fora do escopo.

## Vazios e pendencias

A aceitacao de campo vazio no extrato segue a regra especifica de cada campo: quantidade, unidade e indicadores permanecem opcionais nesta validacao, exceto quando a referencia permite verificar uma combinacao obrigatoria (`00` exige tabela propria e `90`/`98` exigem pacote); os valores do item sao obrigatorios, exceto em componente de pacote identificado por `IND_PACOTE=1`. O [guia ANS do componente de utilizacao](https://fhir-hm.ans.gov.br/componente_itens_assistenciais.html) define unidade como condicional, preenchida quando o item cobrado possui unidade; o extrato `AMB_DET`, isoladamente, nao permite decidir essa condicao por item. Unidade preenchida deve pertencer a TUSS 60 da competencia 202501 e ser vigente na data do evento. Valor negativo e aceito apenas em `VL_ITEM_PAGO_FORNECEDOR`, com alerta para verificacao do estorno e do saldo da guia.

Pendentes: obrigatoriedade condicional dos demais campos, consistencias entre itens da mesma guia, aplicabilidade historica e confronto com a massa completa. Para `QT_ITEM_EVENTO_INFORMADO` e os dois valores do item, as regras locais do extrato estao documentadas acima; a validacao integral de mensagens/guias continua fora deste escopo. Os catalogos TUSS 19, 20, 22, 60 e 63 sao carregados pelo Core. Codigo de procedimento ausente dos snapshots 19/20/22/63 continua gerando alerta, nao rejeicao; para a TUSS 60, a referencia 202501 contem os 61 codigos unicos, e um codigo preenchido ausente dessa tabela e rejeitado.

## Testes e explicacao para a sabatina

A regressao em `tests/test_catalogo_tuss.cpp` cobre a matriz contextual dos indicadores: tabela 00 com tabela própria obrigatória; referências 90/98 com pacote obrigatório e indicador de tabela própria opcional; TUSS 19/20/22/63 com tabela própria incompatível e indicador de pacote permitido; e valores de indicador fora de `0`/`1`. A regra de obrigatoriedade dos indicadores é uma decisão local do extrato AMB_DET baseada no significado das referências; não se declara que a ANS publique essa matriz para todo contexto de mensagem TISS.



`python tools/testar_tuss.py` verifica limites exatos e excedidos, campos vazios, zeros iniciais, casas decimais e formatos inadequados. Uma massa controlada confere o mesmo resumo nas entradas CSV e SQLite. A regressao anterior continua obrigatoria.

Na pratica de COBOL, esta etapa corresponde a criticar tamanho, classe numerica e casas decimais antes de aceitar um registro. O Core preserva a representacao textual e verifica os digitos sem usar `double`; isso evita introduzir erro de ponto flutuante ou perda de zeros. Nao e uma implementacao de COMP-3. Os DAOs continuam responsaveis por acesso aos dados; o validador concentra as regras.
