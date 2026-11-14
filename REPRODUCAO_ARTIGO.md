# Roteiro de reprodução e citação

## Ambiente e regressão

Executar a partir da raiz do repositório. Requisitos do harness: Python 3 e gcc/g++ no PATH, com suporte a C++23; no Windows, utilizar a instalação MinGW correspondente ao ambiente de desenvolvimento. O harness compila o programa em diretório temporário e utiliza o SQLite incluído no projeto.

```text
python tools/testar_tuss.py
```

As referências versionadas devem estar presentes nos caminhos esperados. A suíte verifica seus manifestos e hashes, com o tratamento de quebras de linha explicitado no código. O script de regressão utiliza a biblioteca padrão Python; openpyxl é necessário para reproduzir os catálogos 19 e 60 a partir das planilhas, conforme seus documentos próprios.

O término com sucesso da regressão não executa a massa integral e não mede consumo energético.

## Execução no Code::Blocks

Abrir LEGATECH_Core/LEGATECH_Core.cbp, selecionar Release e executar Rebuild. Na raiz do repositório, usar o executável produzido e a sintaxe documentada em ENTRADA_DAO.md e SAIDA_DAO.md. Exemplo Windows com exportação e catálogos 22 e 63:

```text
LEGATECH_Core\bin\Release\LEGATECH_Core.exe data\sample\PR_202501_AMB_DET_sample.csv data\reference\tuss22_202501.csv data\reference\tuss63_202501.csv --export resultado_exemplo
```

A pasta resultado_exemplo deve ser nova. O carregamento automático das outras referências depende dos caminhos descritos nos documentos dos catálogos. Conferir todos os catálogos efetivamente carregados na saída.

## Execução integral registrada

A massa integral não está incluída nesta proposta. O ensaio Windows de 08/11/2026 usou código no commit e303f2306bb3fd1155c973edaccd365babc21562, em Release/GCC 14.2.0, com exportação CSV e SQLite habilitada. Usar a MATRIZ_EVIDENCIAS_CABS002.md para conferir o recorte, hashes, totais, tempo e limitações.

Antes de declarar reprodução integral independente, completar o registro do comando exato, versões do sistema e configuração do equipamento. A duração histórica de 963,538 s não é um resultado esperado obrigatório em outro ambiente.

A execução Ubuntu foi associada ao commit 6a63514312482df1439a76d20d4b30b633ba39cd e teve adaptação temporária do harness para TUSS 63. Não apresentar um checkout sem essa adaptação como reprodução exata. Exportação e auditoria estavam desativadas; os tempos não devem ser comparados diretamente com o ensaio Windows.

## Citação

CITATION.cff identifica o software e o autor. Ainda não contém versão, data de release, DOI ou licença, pois esses elementos não foram definidos nesta preparação. Ao referenciar resultados, acrescentar o commit efetivamente utilizado, o ano e a data real de acesso. A futura release documental não substitui a identificação do código histórico executado.

## Conferência desta atualização

Esta atualização acrescenta documentação e metadados de citação. Não representa nova execução dos testes ou da massa integral.
