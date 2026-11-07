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

Para quantidade, o tamanho 6 do dicionario nao esclarece uma escala fixa. O limite de seis digitos de precisao, contando a parte fracionaria, e uma interpretacao operacional explicita do projeto; nao deve ser apresentado como regra de escala confirmada pela ANS. Por exemplo, 1234.56 e aceito; 12345.67 excede essa precisao. Valores monetarios com mais de duas casas sao rejeitados, sem arredondamento.

## Vazios e pendencias

A documentacao explica que componentes de pacotes podem vir sem quantidade e valores, e que novos campos podem nao estar presentes em todos os eventos. Por isso, esta etapa preserva a aceitacao de vazios de quantidade, valores, unidade e indicadores. Nao usa apenas IND_PACOTE para presumir toda a obrigatoriedade: o contexto completo deve ser estabelecido.

Pendentes: obrigatoriedade condicional, consulta ao dominio TUSS 60, revisao das regras de sinais/faixas conforme a origem, aplicabilidade aos contratos historicos e confronto com a massa completa. Os catalogos TUSS 19, 20, 22 e 63 sao carregados pelo Core. Um codigo TUSS ausente do snapshot continua gerando alerta, nao rejeicao.

## Testes e explicacao para a sabatina

`python tools/testar_tuss.py` verifica limites exatos e excedidos, campos vazios, zeros iniciais, casas decimais e formatos inadequados. Uma massa controlada confere o mesmo resumo nas entradas CSV e SQLite. A regressao anterior continua obrigatoria.

Na pratica de COBOL, esta etapa corresponde a criticar tamanho, classe numerica e casas decimais antes de aceitar um registro. O Core preserva a representacao textual e verifica os digitos sem usar `double`; isso evita introduzir erro de ponto flutuante ou perda de zeros. Nao e uma implementacao de COMP-3. Os DAOs continuam responsaveis por acesso aos dados; o validador concentra as regras.
