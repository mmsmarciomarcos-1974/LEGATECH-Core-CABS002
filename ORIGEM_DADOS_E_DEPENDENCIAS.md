# Origem dos dados e componentes externos

## Fontes documentadas

| Material | Origem registrada | Limite da conferência |
|---|---|---|
| Catálogos TUSS 19 e 60 | Pacote de Representação de Conceitos em Saúde 202501 da ANS; normalizadores e manifestos próprios | Reprodução e hashes documentados; termos de redistribuição das representações derivadas ainda precisam ser conferidos |
| TUSS 63 | Mesmo pacote; aba Tab 63; conferência adicional com PDF documentada | Preservar códigos de três dígitos e distinguir fim de implantação de fim de vigência |
| TUSS 22 | CSV de referência identificado no manifesto e documentação do pacote | Falta consolidar a cadeia exata de obtenção do CSV de origem |
| TUSS 20 | CSV de origem preservado e normalizador na raiz | A documentação aponta o diretório geral ANS/TISS; falta identificar precisamente o arquivo oficial de aquisição |
| Amostras CSV e banco de dez linhas | Exemplos e variantes controladas presentes no repositório | A inspeção não comprovou que todas as amostras sejam inteiramente sintéticas; confirmar a origem com o autor |
| SQLite | Código de terceiro em sqlite3.c e sqlite3.h | Manter avisos originais e conferir versão e termos antes da distribuição |

Página oficial conferida em 14/11/2026:
https://www.gov.br/ans/pt-br/assuntos/prestadores/padrao-para-troca-de-informacao-de-saude-suplementar-2013-tiss/padrao-tiss-janeiro-2025

A página identifica o componente de Representação de Conceitos em Saúde como versão 202501. Seu rodapé informa Creative Commons Atribuição-SemDerivações 3.0 Não Adaptada para o conteúdo do site. Esse aviso, isoladamente, não foi interpretado aqui como autorização específica para redistribuir cada anexo ou catálogo normalizado.

Consultar também CATALOGO_TUSS19.md, CATALOGO_TUSS20.md, CATALOGOS_TUSS22_63.md, CATALOGO_TUSS60.md e os manifestos em data/reference.

## Tratamento editorial das amostras

Não apresentar identificadores de evento como se fossem necessariamente identificadores diretos de pacientes. Também não afirmar anonimização apenas porque não há nomes no leiaute. O banco inspecionado contém onze campos TEXT e dez linhas; isso caracteriza sua estrutura, não certifica a origem.

As variantes com campos inválidos e a massa controlada devem ser descritas como dados preparados para testes. Para rotulá-las como totalmente sintéticas é necessário confirmar sua geração. Eventual substituição deverá preservar os cenários, as expectativas dos testes e o registro das evidências anteriores.

## Licença do projeto

A licença do código próprio ainda será escolhida pelo autor. Este registro não concede novos direitos sobre o código, os dados ou os componentes de terceiros. Não foi criado LICENSE nem aplicada uma licença por inferência.

A escolha deve considerar se o autor deseja permitir utilização e modificação por terceiros, inclusive comercial, e quais obrigações deseja estabelecer. A licença escolhida para o código próprio não altera os termos dos catálogos ou do SQLite.
