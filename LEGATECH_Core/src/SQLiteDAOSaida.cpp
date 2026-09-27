#include "../include/SQLiteDAOSaida.h"
#include "../include/sqlite3.h"

#include <iostream>

SQLiteDAOSaida::SQLiteDAOSaida(
    const std::string& caminhoBanco
)
    : caminhoBanco(caminhoBanco)
{
}

void SQLiteDAOSaida::gravar(
    const RegistroTissBruto& registro
)
{
    sqlite3* banco = nullptr;

    // =====================================================
    // 1. ABERTURA DO BANCO
    // =====================================================

    int retorno = sqlite3_open(
        caminhoBanco.c_str(),
        &banco
    );

    if (retorno != SQLITE_OK)
    {
        std::cout << "ERRO ao abrir banco para gravacao.\n";

        if (banco != nullptr)
        {
            std::cout << "Detalhe: "
                      << sqlite3_errmsg(banco)
                      << '\n';

            sqlite3_close(banco);
        }

        return;
    }


    // =====================================================
    // 2. GARANTE A EXISTENCIA DA TABELA
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

    int retornoTabela = sqlite3_exec(
        banco,
        sqlCriarTabela,
        nullptr,
        nullptr,
        &mensagemErro
    );

    if (retornoTabela != SQLITE_OK)
    {
        std::cout
            << "ERRO ao criar/verificar tabela registro_tiss.\n";

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
    // 3. PREPARACAO DO INSERT
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
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* comando = nullptr;

    retorno = sqlite3_prepare_v2(
        banco,
        sqlInserir,
        -1,
        &comando,
        nullptr
    );

    if (retorno != SQLITE_OK)
    {
        std::cout << "ERRO ao preparar INSERT.\n";
        std::cout << "Detalhe: "
                  << sqlite3_errmsg(banco)
                  << '\n';

        sqlite3_close(banco);
        return;
    }


    // =====================================================
    // 4. ASSOCIACAO DOS CAMPOS DO OBJETO AO INSERT
    // =====================================================

    sqlite3_bind_text(
        comando,
        1,
        registro.getIdEventoAtencaoSaude().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        comando,
        2,
        registro.getUfPrestador().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        comando,
        3,
        registro.getDtRealizacao().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        comando,
        4,
        registro.getCdProcedimento().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        comando,
        5,
        registro.getCdTabelaReferencia().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        comando,
        6,
        registro.getQtItemEventoInformado().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        comando,
        7,
        registro.getVlItemEventoInformado().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        comando,
        8,
        registro.getVlItemPagoFornecedor().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        comando,
        9,
        registro.getUnidadeMedida().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        comando,
        10,
        registro.getIndPacote().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        comando,
        11,
        registro.getIndTabelaPropria().c_str(),
        -1,
        SQLITE_TRANSIENT
    );


    // =====================================================
    // 5. EXECUCAO DO INSERT
    // =====================================================

    retorno = sqlite3_step(comando);

    if (retorno == SQLITE_DONE)
    {
        std::cout
            << "Registro gravado no SQLite com sucesso.\n";
    }
    else
    {
        std::cout
            << "ERRO ao gravar registro no SQLite.\n";

        std::cout << "Detalhe: "
                  << sqlite3_errmsg(banco)
                  << '\n';
    }


    // =====================================================
    // 6. LIBERACAO DOS RECURSOS
    // =====================================================

    sqlite3_finalize(comando);
    sqlite3_close(banco);
}
