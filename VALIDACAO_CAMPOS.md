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
| VL_ITEM_EVENTO_INFORMADO | NUMBER, tamanho 18,2 | Ate 16 digitos inteiros e duas casas decimais |
| VL_ITEM_PAGO_FORNECEDOR | NUMBER, tamanho 18,2 | Ate 16 digitos inteiros e duas casas decimais |
| UNIDADE_MEDIDA | VARCHAR2(40), referencia TUSS 60 | Ate 40 caracteres; dominio ainda pendente |

O formato decimal exige digitos antes e depois do separador quando ele existe. Aceita ponto ou virgula, nunca ambos nem mais de um separador. Recusa `.5`, `5.`, notacao cientifica, NaN e sinais; a restricao de sinais ja existia. Nao aplica agrupamento de milhares, conversao binaria ou arredondamento silencioso. Zeros iniciais sao preservados, mas nao aumentam a precisao numerica. O limite textual conta caracteres UTF-8 em entradas bem formadas; nao constitui validacao completa de codificacao UTF-8.

### Regra fechada para `QT_ITEM_EVENTO_INFORMADO`

O escopo aqui e o extrato tabular publico `AMB_DET`, nao a validacao integral de uma mensagem TISS ou de uma guia. No dicionario consultado, o campo e `NUMBER`, tamanho 6; a fonte nao declara escala decimal fixa, faixa minima/maxima nem obrigatoriedade para esse campo. Portanto, o Core preserva valores vazios e nao importa para esse extrato regras de preenchimento de uma mensagem/guias especificas.

Quando preenchido, o valor deve ser decimal nao negativo com no maximo seis digitos de precisao total, contando os digitos fracionarios. A escala nao e fixa: `1234.56` e `0.123456` sao aceitos; `12345.67` e `0.1234567` excedem o limite. Zero e aceito porque o dicionario do extrato nao define minimo; sinais, notacao cientifica e formatos incompletos continuam rejeitados. O separador pode ser ponto ou virgula. Essa e uma interpretacao operacional explicita do tamanho 6, nao uma escala publicada pela ANS.

As mensagens TISS podem definir obrigatoriedade, tipo ou faixa diferentes para campos semanticamente relacionados conforme o componente e a guia. Por exemplo, o [modelo ANS do componente de utilizacao publicado em 2023](https://fhir-hm.ans.gov.br/componente_itens_assistenciais.html) declara quantidade inteira obrigatoria e maior que zero naquele caso de uso. Essa regra nao e transferida ao `AMB_DET` sem prova de equivalencia entre os campos e contextos. A validacao de mensagens/guias fica fora do escopo desta regra.

## Vazios e pendencias

A aceitacao de campo vazio no extrato e baseada na ausencia de regra de obrigatoriedade no dicionario consultado; nao e inferida apenas de `IND_PACOTE`. A mesma cautela vale para valores, unidade e indicadores: o dicionario nao descreve por si so todas as condicoes de preenchimento das mensagens/guias de origem.

Pendentes: obrigatoriedade condicional dos demais campos, consulta ao dominio TUSS 60, aplicabilidade historica e confronto com a massa completa. Para `QT_ITEM_EVENTO_INFORMADO`, a regra do extrato `AMB_DET` esta fechada nos limites acima; regras de mensagens/guias continuam fora deste escopo. Os catalogos TUSS 19, 20, 22 e 63 sao carregados pelo Core. Um codigo TUSS ausente do snapshot continua gerando alerta, nao rejeicao.

## Testes e explicacao para a sabatina

`python tools/testar_tuss.py` verifica limites exatos e excedidos, campos vazios, zeros iniciais, casas decimais e formatos inadequados. Uma massa controlada confere o mesmo resumo nas entradas CSV e SQLite. A regressao anterior continua obrigatoria.

Na pratica de COBOL, esta etapa corresponde a criticar tamanho, classe numerica e casas decimais antes de aceitar um registro. O Core preserva a representacao textual e verifica os digitos sem usar `double`; isso evita introduzir erro de ponto flutuante ou perda de zeros. Nao e uma implementacao de COMP-3. Os DAOs continuam responsaveis por acesso aos dados; o validador concentra as regras.
