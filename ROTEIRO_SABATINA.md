# Roteiro de demonstração CABS002

Executar na raiz do repositório após rebuild Release no Code::Blocks. Usar uma pasta de destino nova em cada comando. Os catálogos 22 e 20 devem permanecer juntos em data/reference.

1. Mostrar DAOEntrada.h e os dois caminhos em main.cpp: DAOEntrada* chama processarArquivo() do objeto CSV ou SQLite. Explicar .h, .cpp e duração do objeto local.
2. Rodar a regressão: `python tools/testar_tuss.py`. Ela cria entradas SQLite temporárias, compara fontes e saídas, testa limites, datas, TUSS 20/22/63 e cancelamento. As massas originais não são alteradas.
3. Demonstrar uma exportação:

```bat
LEGATECH_Core\bin\Release\LEGATECH_Core.exe data\sample\PR_202501_AMB_DET_sample_rejeitado.csv data\reference\tuss22_202501.csv --export demonstracao_cabs002
```

Esperado nesta amostra: total 10; válidos 6; válidos com alerta 3; rejeitados 1. Sem catálogo 63 carregado, os três alertas são esperados.

4. Abrir aceitos.csv, rejeitados.csv, contexto.csv e resultados.db. Relacionar estado, motivos, alertas e linha de origem. A entrada SQLite é registro_tiss; resultados.db é auditoria e não serve diretamente como entrada.
5. Usar os cenários sintéticos da suíte para explicar: 2025-13 rejeita; 2038-01 recebe alerta preventivo; precisão excedida rejeita sem arredondar; código ausente no snapshot gera alerta. Não apresentar o alerta Y2K38 como estouro real em sistema legado.
6. Mostrar os testes de falha de gravação: rollback e ausência de publicação parcial. Não há garantia contra queda de energia.

## Antes da apresentação

- Rebuild e regressão com sqlite3.c versionado no Windows.
- Executar a massa completa PR_202501_AMB_DET.csv, se disponível, e registrar SHA do código, hash da massa, contagens, tempo e ambiente.
- Explicar que TUSS 60, catálogo 19, obrigatoriedade condicional e aplicabilidade histórica integral permanecem fora do que foi comprovado.
- Reexecutar o comando da demonstração com uma pasta nova; não sobrescrever exportações anteriores.
