# Entrada por DAO no LEGATECH Core

O executável permite processar CSV ou SQLite. Cada implementação produz `RegistroTissBruto` e usa `ValidadorRegistroTiss` com catálogo e competência. A entrada SQLite usa a tabela relacional normalizada `registro_tiss`; não lê as tabelas de auditoria.

## Execução no Windows

Abrir `LEGATECH_Core/LEGATECH_Core.cbp` no Code::Blocks, selecionar Release e fazer Rebuild. Executar os comandos abaixo na raiz do repositório. Não é necessário adicionar arquivos ao projeto do Code::Blocks: esta mudança usa classes já existentes.

CSV mantém a sintaxe anterior, inclusive auditoria opcional. Criar `data/output` se ainda não existir:

```powershell
.\LEGATECH_Core\bin\Release\LEGATECH_Core.exe .\data\sample\PR_202501_AMB_DET_sample.csv .\data\reference\tuss22_202501.csv .\data\output\auditoria_tiss.db .\data\reference\tuss63_202501.csv
```

SQLite recebe banco existente e competência explícita em AAAAMM. Ajustar o caminho ao banco de entrada real; o comando não cria nem carrega esse banco:

```powershell
.\LEGATECH_Core\bin\Release\LEGATECH_Core.exe --sqlite .\data\legatech.db 202501 .\data\reference\tuss22_202501.csv .\data\reference\tuss63_202501.csv
```

O catálogo 63 é opcional no comando SQLite. Sem ele, procedimentos da tabela 63 podem receber alerta de referência não carregada. Ano e mês da competência são conferidos antes da leitura; a competência não é inferida do nome do banco. O catálogo continua na versão 202501, mesmo quando a competência informada é histórica.

## Contrato da entrada relacional

O SELECT exige estas onze colunas em `registro_tiss`:

```text
id_evento_atencao_saude
uf_prestador
dt_realizacao
cd_procedimento
cd_tabela_referencia
qt_item_evento_informado
vl_item_evento_informado
vl_item_pago_fornecedor
unidade_medida
ind_pacote
ind_tabela_propria
```

O SELECT nomeia as colunas: sua ordem física na tabela pode variar. Colunas adicionais não são utilizadas. Tabela ou coluna obrigatória ausente é falha de estrutura, com retorno 1. Tabela existente sem linhas é execução válida com contagens zeradas. O banco é aberto somente para leitura; um caminho inexistente falha sem criar arquivo. Falha durante o SELECT também retorna 1 e não imprime resumo de sucesso.

Para preservar zeros e representação original, carregar os campos como TEXT. O DAO não pode recuperar zeros já perdidos por uma carga anterior em coluna numérica. SQL NULL é apresentado ao modelo atual como string vazia. A adaptação de CSV histórico com dez campos permanece no contrato CSV; a tabela SQLite deve conter a representação comum de onze campos.

## Polimorfismo na prática

Em `DAOEntrada.h`, `virtual void processarArquivo() = 0` define o contrato. Em `CsvDAOEntrada.h` e `SQLiteDAOEntrada.h`, `override` confere sua implementação. Em `main.cpp`, cada caminho constrói o objeto concreto e usa:

```cpp
DAOEntrada* entrada = &sqlite; // No caminho CSV, o endereço é &csv.
entrada->processarArquivo();
```

O tipo do ponteiro é a base; o método virtual executa a implementação do objeto apontado. O ponteiro não possui o objeto nem precisa de `delete`: o objeto local continua vivo durante a chamada. A criação e escolha da fonte ficam no main; os detalhes de leitura ficam no DAO.

Em termos de estudo: `.h` declara o contrato e os atributos; `.cpp` implementa o comportamento. Nem todo comportamento é regra de negócio: abrir banco e preparar SELECT são operações técnicas; conferir procedimento e vigência pertence ao validador. CALL em COBOL ajuda a pensar em contrato de chamada, mas o despacho virtual de C++ tem mecanismo próprio.

## Verificação e limites desta etapa

```powershell
python tools\testar_tuss.py
```

A regressão compila em C++23, transforma as mesmas amostras CSV em bancos temporários de onze colunas TEXT e confere contagens iguais nas duas fontes, com um ou dois catálogos. Confere competência divergente, banco vazio, banco ausente, tabela/coluna ausente e argumentos inválidos. Os bancos temporários são fechados e removidos; a entrada não é alterada.

As duas entradas estão disponíveis e demonstram o despacho pelo ponteiro da base no executável. A exportação CSV/SQLite e a auditoria de resultados foram implementadas posteriormente e estão descritas em [SAIDA_DAO.md](SAIDA_DAO.md). Não passar um banco de auditoria como entrada `registro_tiss`. A suíte verifica cenários definidos e não mede o desempenho da massa completa; as execuções integrais e as validações nos ambientes estão registradas na [matriz de evidências](MATRIZ_EVIDENCIAS_CABS002.md).
