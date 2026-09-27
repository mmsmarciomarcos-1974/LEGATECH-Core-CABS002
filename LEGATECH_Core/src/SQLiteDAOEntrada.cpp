#include "../include/SQLiteDAOEntrada.h"
#include "../include/sqlite3.h"

#include <iostream>

SQLiteDAOEntrada::SQLiteDAOEntrada(
    const std::string& caminhoBanco
)
    : caminhoBanco(caminhoBanco)
{
}

void SQLiteDAOEntrada::processarArquivo()
{
    sqlite3* banco = nullptr;

    int retorno = sqlite3_open(
        caminhoBanco.c_str(),
        &banco
    );

    if (retorno != SQLITE_OK)
    {
        std::cout << "ERRO ao abrir banco SQLite.\n";

        if (banco != nullptr)
        {
            std::cout << "Detalhe: "
                      << sqlite3_errmsg(banco)
                      << '\n';

            sqlite3_close(banco);
        }

        return;
    }

    std::cout << "Banco SQLite aberto com sucesso.\n";
    std::cout << "Banco: " << caminhoBanco << '\n';

    // Cria a primeira tabela do LEGATECH.
    // Os campos permanecem TEXT nesta etapa
    // para preservar o dado bruto antes da validacao.
    const char* sqlCriarTabela =
        "CREATE TABLE IF NOT EXISTS registro_tiss ("
        "id_evento_atencao_saude TEXT,"
        "uf_prestador TEXT,"
        "dt_realizacao TEXT,"
        "cd_procedimento TEXT,"
        "cd_tabela_referencia TEXT,"
        "qt_item_evento_informado TEXT,"
        "vl_item_evento_informado TEXT,"
        "vl_item_pago_fornecedor TEXT,"
        "unidade_medida TEXT,"
        "ind_pacote TEXT,"
        "ind_tabela_propria TEXT"
        ");";

    char* mensagemErro = nullptr;

    int retornoSql = sqlite3_exec(
        banco,
        sqlCriarTabela,
        nullptr,
        nullptr,
        &mensagemErro
    );

    if (retornoSql != SQLITE_OK)
    {
        std::cout << "ERRO ao criar tabela registro_tiss.\n";

        if (mensagemErro != nullptr)
        {
            std::cout << "Detalhe: "
                      << mensagemErro
                      << '\n';

            sqlite3_free(mensagemErro);
        }

        sqlite3_close(banco);
        return;
    }

    std::cout
        << "Tabela registro_tiss criada/verificada com sucesso.\n";

    sqlite3_close(banco);

    std::cout << "Banco SQLite fechado com sucesso.\n";
}
