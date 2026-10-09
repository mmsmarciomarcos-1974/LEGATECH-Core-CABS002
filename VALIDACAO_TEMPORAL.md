# RF05 — Datas e compatibilidade temporal

Registro para a sabatina, 08/10/2026. O campo DT_REALIZACAO do AMB_DET tem granularidade mensal: AAAA-MM. O Core nao inventa dia, horario ou fuso para esse campo.

## Regras implementadas

| Situacao | Resultado temporal | Explicacao |
| --- | --- | --- |
| Vazio | Rejeicao | Campo obrigatorio |
| Formato diferente de AAAA-MM, ano 0000 ou mes fora de 01–12 | Rejeicao | Perfil do projeto aceita anos civis 0001–9999 |
| Mes valido igual a competencia do arquivo | Sem alerta de competencia | Outras regras ainda podem alterar a classificacao |
| Mes valido diferente da competencia | Alerta | Divergencia contextual, preservando o dado original |
| Mes valido a partir de 2038-01 | Alerta COMPATIBILIDADE_Y2K38 | Possivel incompatibilidade ao integrar com um destino que use timestamp Unix de 32 bits com sinal |

Janeiro de 2038 inclui a fronteira de representacao; sem dia e horario nao se pode afirmar que um evento desse mes ultrapassou o limite. Meses posteriores estao depois da fronteira. O alerta e preventivo e independe da arquitetura da maquina onde o Core roda. Nao declara que o Core ou todo sistema de 32 bits apresenta falha.

Referencia tecnica consultada em 08/10/2026: [GNU Gnulib — Avoiding the year 2038 problem](https://www.gnu.org/software/gnulib/manual/html_node/Avoiding-the-year-2038-problem.html). Um timestamp Unix de 32 bits com sinal nao representa instantes a partir de 2038-01-19 03:14:08 UTC.

O Core valida o texto mensal e compara ano/mes; nao converte DT_REALIZACAO para time_t. A avaliacao da vigencia de procedimentos usa std::chrono com a janela do mes. Mes totalmente fora da vigencia conhecida gera rejeicao; vigencia que cobre apenas parte do mes gera alerta, pois o dia do evento nao esta disponivel.

## Demonstracao e limites

Testes verificam vazio, ano zero, meses 00/13, formato incorreto, anos 0001/9999, fevereiro, dezembro de 2037, janeiro/fevereiro de 2038 e coexistencia de alertas. A massa controlada compara entradas CSV/SQLite, totais e exportacoes com motivos e alertas iguais. Execute `python tools/testar_tuss.py`.

A massa AMB_DET 2025 nao demonstra sozinha Y2K38. O cenario futuro e sintetico e identificado como teste de compatibilidade. Nao se aplica validacao de dia ou ano bissexto a um campo que nao possui dia. Nao foi executada integracao com um sistema legado que use timestamp de 32 bits; o teste demonstra a regra preventiva do Core, nao um estouro real. Tambem nao declara compatibilidade de todos os anos com todo destino externo.

## Resposta para a sabatina

**Como distingue data invalida de data valida com alerta?**

“2025-13 e invalido porque o mes nao existe. 2038-02 e um mes valido, mas recebe alerta se o dado for destinado a uma tecnologia que use timestamp Unix de 32 bits com sinal. O dado permanece valido. Janeiro de 2038 recebe alerta preventivo porque a base nao informa o dia.”

**Onde fica a regra?**

Em ValidadorRegistroTiss.cpp, compartilhada pelos DAOs CSV e SQLite. ResultadoValidacao acumula motivos e alertas; os DAOs de saida preservam essas ocorrencias. O .h declara o contrato da classe; o .cpp implementa seu comportamento.

**Como relacionar com COBOL?**

Uma PIC com ano e mes pode preservar o valor sem usa-lo como contador de segundos. A analogia ajuda a separar representacao do dado e limites da tecnologia que o recebera. Assim como em COBOL, validar formato e dominio nao substitui verificar compatibilidade na conversao para outro formato.
