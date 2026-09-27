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


    // =====================================================
    // 1. CRIACAO DA TABELA
    // =====================================================

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


    // =====================================================
    // 2. INSERT DE TESTE
    // =====================================================

    const char* sqlInserir =
        "INSERT INTO registro_tiss ("
        "id_evento_atencao_saude,"
        "uf_prestador,"
        "dt_realizacao,"
        "cd_procedimento,"
        "cd_tabela_referencia,"
        "qt_item_evento_informado,"
        "vl_item_evento_informado,"
        "vl_item_pago_fornecedor,"
        "unidade_medida,"
        "ind_pacote,"
        "ind_tabela_propria"
        ") VALUES ("
        "'6450733769',"
        "'PR',"
        "'2025-01',"
        "'10101012',"
        "'22',"
        "'1',"
        "'200',"
        "'0',"
        "'',"
        "'0',"
        "'0'"
        ");";

    mensagemErro = nullptr;

    retornoSql = sqlite3_exec(
        banco,
        sqlInserir,
        nullptr,
        nullptr,
        &mensagemErro
    );

    if (retornoSql != SQLITE_OK)
    {
        std::cout << "ERRO ao inserir registro de teste.\n";

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

    std::cout << "Registro de teste inserido com sucesso.\n";


    // =====================================================
    // 3. SELECT COUNT(*)
    // =====================================================

    sqlite3_stmt* consulta = nullptr;

    const char* sqlContar =
        "SELECT COUNT(*) FROM registro_tiss;";

    int retornoConsulta = sqlite3_prepare_v2(
        banco,
        sqlContar,
        -1,
        &consulta,
        nullptr
    );

    if (retornoConsulta != SQLITE_OK)
    {
        std::cout << "ERRO ao preparar SELECT COUNT(*).\n";
        std::cout << "Detalhe: "
                  << sqlite3_errmsg(banco)
                  << '\n';

        sqlite3_close(banco);
        return;
    }

    if (sqlite3_step(consulta) == SQLITE_ROW)
    {
        int quantidade = sqlite3_column_int(
            consulta,
            0
        );

        std::cout << "Registros na tabela: "
                  << quantidade
                  << '\n';
    }

    sqlite3_finalize(consulta);


    // =====================================================
    // 4. ENCERRAMENTO
    // =====================================================

    sqlite3_close(banco);

    std::cout << "Banco SQLite fechado com sucesso.\n";
}
