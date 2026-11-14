# LEGATECH Core

Projeto final da disciplina CABS002 - Programação Avançada.

Implementação em C++23 para ingestão, validação e transformação de dados heterogêneos.

## Contexto

Universidade Tecnológica Federal do Paraná - UTFPR  
Programa de Pós-Graduação em Computação Aplicada - PPGCA  
Disciplina: CABS002 - Programação Avançada

## Catálogos TUSS 22 e 63

A carga conjunta opcional e os comandos de validação no Windows estão em [CATALOGOS_TUSS22_63.md](CATALOGOS_TUSS22_63.md). A tabela 63 usa uma referência externa versionada com 31 grupos, conferida no pacote oficial 202501.

### Catálogo TUSS 19

A referência oficial 202501, a normalização das duas planilhas e a carga automática das sete partes estão descritas em [CATALOGO_TUSS19.md](CATALOGO_TUSS19.md). O catálogo normalizado contém código e datas de vigência, sem as descrições extensas das planilhas originais. A TUSS 60 de unidade de medida e sua normalização estão documentadas em [CATALOGO_TUSS60.md](CATALOGO_TUSS60.md).

## Entrada CSV e SQLite por DAO

O executável aceita as duas fontes e chama `processarArquivo()` por um ponteiro de `DAOEntrada`. Comandos, formato relacional e explicação para a sabatina: [ENTRADA_DAO.md](ENTRADA_DAO.md).

### Resultados CSV e SQLite por DAO

Use `--export pasta_nova` para exportar classificacoes, rejeicoes e alertas nos dois formatos. Comandos, classes e limites estao em [SAIDA_DAO.md](SAIDA_DAO.md).

### Limites e precisao dos campos (RF04)

Regras implementadas, fonte consultada e pendencias: [VALIDACAO_CAMPOS.md](VALIDACAO_CAMPOS.md).

### Datas e compatibilidade temporal (RF05)

Regras, fronteira Y2K38 e roteiro da sabatina: [VALIDACAO_TEMPORAL.md](VALIDACAO_TEMPORAL.md).

### Regra complementar DUP-01 — duplicidade de registros

A duplicidade é um controle complementar e não altera a numeração do baseline aceito, no qual RF07 corresponde ao registro dos motivos de rejeição. Regra, evidências de teste e limite operacional: [VALIDACAO_DUPLICIDADE.md](VALIDACAO_DUPLICIDADE.md).

### Matriz de evidências CABS002

Rastreabilidade dos requisitos RF01–RF10, testes, resultados e limitações: [MATRIZ_EVIDENCIAS_CABS002.md](MATRIZ_EVIDENCIAS_CABS002.md).

O workflow [Regressão TUSS — Linux e Windows](.github/workflows/regressao-tuss.yml) executa `tools/testar_tuss.py` nas duas plataformas após alterações no branch `main` e em pull requests.

## Execução no Windows e no Ubuntu

Em 02/11/2026, o projeto foi executado pelo Code::Blocks, na configuração Release, nos dois sistemas, com caminhos relativos e a mesma massa controlada `data/sample/PR_202501_AMB_DET_validacao.csv`, usando as referências TUSS 22 e 63 de 202501. Este ensaio controlado processou oito registros:

| Ambiente | Registros | Válidos | Válidos com alerta | Rejeitados | Estrutura divergente | Retorno |
|---|---:|---:|---:|---:|---:|---:|
| Windows | 8 | 2 | 1 | 5 | 1 | 0 |
| Ubuntu | 8 | 2 | 1 | 5 | 1 | 0 |

Nos dois ambientes, a exportação foi confirmada em `../data/output/saida_codeblocks`. Esta evidência demonstra a compilação/execução e a resolução dos caminhos relativos com a massa controlada; não representa benchmark nem validação de processamento em escala.

### Execução integral no Compaq Windows — 08/11/2026

O arquivo público `PR_202501_AMB_DET.csv`, recorte do Paraná na competência janeiro de 2025, tem 462.698.039 bytes e 7.855.323 registros. No notebook Compaq com 4 GB de RAM, a execução Release terminou com código 0 e exportou resultados em CSV e SQLite:

| Resultado | Registros |
|---|---:|
| Válidos | 7.609.841 |
| Válidos com alerta | 3.343 |
| Rejeitados | 242.139 |
| Divergências estruturais | 0 |
| Total processado | 7.855.323 |

O tempo interno informado pelo programa foi 963,538 s (cerca de 16 minutos). Foram gerados `aceitos.csv` (1.291.810.378 bytes), `contexto.csv` (394 bytes), `rejeitados.csv` (55.606.708 bytes) e `resultados.db` (970.088.448 bytes), totalizando 2.317.505.928 bytes, aproximadamente 5,01 vezes o tamanho da entrada. Essa razão descreve o volume armazenado na saída; não mede consumo de energia. O pico de memória do processo e o uso de paginação não foram medidos durante essa execução. Trata-se de uma execução em um equipamento e configuração; os números não são um benchmark nem sustentam generalização de desempenho ou eficiência energética.

A origem e a normalização das TUSS 20 e 60 estão descritas em [CATALOGO_TUSS20.md](CATALOGO_TUSS20.md) e [CATALOGO_TUSS60.md](CATALOGO_TUSS60.md).

## Referências da disciplina para consulta futura

As obras abaixo constam no plano de ensino de CABS002 e ficam registradas como fontes possíveis de consulta e aprofundamento. Sua inclusão não significa que todas tenham sido consultadas ou fundamentem diretamente uma implementação específica do Core. Quando forem usadas em relatório, apresentação ou artigo, relacionar cada citação ao conceito efetivamente discutido.

### Referências básicas

- DEITEL, H. M.; DEITEL, P. J. *C++ Como Programar*. 3. ed. Bookman, 2001.
- STROUSTRUP, B. *The C++ Programming Language*. 3rd ed. Addison-Wesley, 1997.
- PRESSMAN, R. S. *Software Engineering: A Practitioner’s Approach*. 6th ed. McGraw-Hill Higher Education, 2005. ISBN 0-07-285318-2.
- GAMMA, E.; HELM, R.; JOHNSON, R.; VLISSIDES, J. *Design Patterns: Elements of Reusable Object-Oriented Software*. Addison-Wesley Longman, 1995.
- SIMÃO, J. M. *Fundamentos de Programação 2 — C++*. Disponível em: <http://www.pessoal.utfpr.edu.br/jeansimao/Fundamentos2/Fundamentos2.htm>.

### Referências complementares

- SEBESTA, R. W. *Conceitos de Linguagens de Programação*. 5. ed. Bookman, 2003.
- HORSTMANN, C. *Conceitos de Computação com o Essencial de C++*. 3. ed. Bookman, 2003. ISBN 0-471-16437-2.
- RUMBAUGH, J.; JACOBSON, I.; BOOCH, G. *The Unified Modeling Language Reference Manual*. 2nd ed. Addison-Wesley, 2005. ISBN 0-321-26797-4.
- RUMBAUGH, J.; JACOBSON, I.; BOOCH, G. *The Unified Software Development Process*. 1st ed. Addison-Wesley, 2005. ISBN 0-201-57169-2.
- LARMAN, C. *Applying UML and Patterns: An Introduction to Object-Oriented Analysis and Design*. Prentice Hall, 1998. ISBN 0-13-748880-7.
- BEZERRA, E. *Princípios de Análise e Projeto de Sistemas com UML*. Editora Campus, 2003. ISBN 85-352-1032-6.

## Preparação para disponibilização junto ao artigo

O escopo da revisão inicial, os limites da inspeção e as pendências de publicação estão em [PREPARACAO_PUBLICACAO.md](PREPARACAO_PUBLICACAO.md). Os resultados históricos devem ser citados com os commits das respectivas execuções, sem atribuí-los automaticamente à versão mais recente.
