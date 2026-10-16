# Evidências de fechamento CABS002

Base inspecionada: ff20aff67fbe37e6830daa1221a492ffe0297aa4 (main, commit de 13/10/2026 UTC). A data civil desse commit em São Paulo é 12/10/2026. Este relatório acompanha as correções em branch; não declara a main validada.

## Correções

- TUSS 20 resolvida ao lado do caminho do catálogo 22 fornecido, nos modos CSV e SQLite. Os comandos explícitos passam a funcionar a partir da raiz e de outros diretórios com caminhos adequados.
- Metadados esperados na regressão atualizados para 22,20 e 22,20,63.
- Manifesto TUSS 20 corrigido de 41.424 para 41.416 registros lógicos. O CSV e seu SHA-256 foram preservados. Contagem com parser CSV e 41.416 códigos únicos; o arquivo contém quebras de linha dentro de campos citados, portanto contagem de linhas físicas não é contagem de registros.
- Hash e contagem dos três catálogos verificados. Testes reais TUSS 20: medicamento 90035593 vigente, anterior à vigência e código ausente, além da colisão sintética de pares já existente.
- Testes de NULL, colunas físicas invertidas/coluna extra e conteúdo de contexto.csv.
- Comparação dos resultados exportados de limites e TUSS 20 entre fontes, além dos cenários temporais existentes.

## Matriz de evidências

| Funcionalidade | Evidência automatizada | Limite da conclusão |
|---|---|---|
| DAOEntrada e polimorfismo | main.cpp usa DAOEntrada* nas duas fontes; integração executa ambas | Demonstração oral deve explicar objeto e ponteiro não proprietário |
| CSV histórico | Normalização 10→11 campos, execução e exportação de cenário 2022 | Não comprova adequação histórica integral das regras |
| Entrada SQLite | Paridade, somente leitura, ausente/vazio, esquema inválido, competência, NULL e ordem das colunas | Tabela deve seguir contrato de onze campos |
| Catálogos | Hashes/contagens 20,22,63, pares isolados, zeros, vigência e falhas | Snapshot 202501 não cobre todo histórico |
| RF04 limites/precisão | Fronteiras, vazios e formatos, paridade de motivos exportados | Quantidade: interpretação operacional; TUSS 60 e obrigatoriedade condicional pendentes |
| RF05 temporal | Ano/mês inválido, 2037/2038, extremos, alertas simultâneos e paridade | Alerta preventivo; sem integração real com timestamp 32 bits |
| Exportação DAO | Estados, identificadores, originais, ocorrências e contexto CSV/SQLite | Linha SQLite é serializada; ordinal não é identificador permanente |
| Auditoria | Integridade SQLite, histórico, rollback, reexecução, falha de gravação e publicação | Sem garantia contra interrupção abrupta/queda de energia |
| Massa completa | Não executada: ausente no repositório | Não declarar desempenho ou contagens integrais |
| Windows/Code::Blocks | Projeto lista os fontes e C++23; sem execução neste ambiente | Rebuild e regressão Windows pendentes |

## Ambiente e reprodução

Linux x86_64; g++ 13.3.0; Python 3.12.14; SQLite do sistema 3.45.1. Comando utilizado:

```sh
LEGATECH_SQLITE_SYSTEM=1 python tools/testar_tuss.py
```

O modo explícito acima liga libsqlite3.so.0 do Linux. A conexão não disponibilizou a amalgamação sqlite3.c de aproximadamente 9,5 MB; por isso esta execução não valida a biblioteca SQLite versionada. O comando padrão continua compilando sqlite3.c e deve ser executado em checkout completo. Não substituir essa pendência por um resultado presumido.

A recuperação dos demais arquivos usou a conexão GitHub fixada no SHA base; data/legatech.db não foi usado. Os testes geram bancos temporários. A biblioteca de sistema e a ausência da massa completa limitam a reprodução local.

## Critérios ainda necessários para fechar a entrega

1. Executar `python tools/testar_tuss.py` com sqlite3.c versionado, sem a variável de sistema.
2. Fazer rebuild Release e demonstração no Windows/Code::Blocks; registrar compilador e resultado.
3. Executar PR_202501_AMB_DET.csv completo e registrar hash, duração, totais e amostras de cada classificação.
4. Manter no escopo declarado as pendências de domínio/obrigatoriedade e aplicabilidade histórica.

Roteiro de apresentação: [ROTEIRO_SABATINA.md](ROTEIRO_SABATINA.md). A prática do Core na disciplina não constitui, isoladamente, evidência científica do projeto de pesquisa LEGATECH.AI.

## Resultado registrado

A regressão completa acima encerrou com retorno 0. [Log da execução](evidencias/regressao-linux-sqlite-sistema.txt). A execução inicial com o caminho fixo falhou ao abrir ../data/reference/tuss20_202501.csv. O resultado aprovado refere-se às alterações desta branch e ao modo SQLite do sistema, mantendo as pendências explicitadas.
