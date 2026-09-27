#include "../include/SQLiteDAOEntrada.h"
#include "../include/sqlite3.h"
#include "../include/RegistroTissBruto.h"
#include "../include/ValidadorRegistroTiss.h"
#include "../include/ResultadoValidacao.h"

#include <iostream>
#include <string>

SQLiteDAOEntrada::SQLiteDAOEntrada(
    const std::string& caminhoBanco
)
    : caminhoBanco(caminhoBanco)
{
}

void SQLiteDAOEntrada::processarArquivo()
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


    // =====================================================
    // 2. PREPARACAO DO SELECT
    // =====================================================

    const char* sql =
        "SELECT "
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
        "ind_tabela_propria "
        "FROM registro_tiss;";

    sqlite3_stmt* consulta = nullptr;

    retorno = sqlite3_prepare_v2(
        banco,
        sql,
        -1,
        &consulta,
        nullptr
    );

    // =====================================================
    // 3. TRATAMENTO DA ESTRUTURA
    // =====================================================

    if (retorno != SQLITE_OK)
    {
        std::string erro = sqlite3_errmsg(banco);

        if (erro.find("no such table") != std::string::npos)
        {
            std::cout
                << "Tabela registro_tiss inexistente ou ainda nao carregada.\n";

            std::cout
                << "Nenhum registro para processamento.\n";

            sqlite3_close(banco);
            return;
        }

        std::cout << "ERRO ao preparar SELECT.\n";
        std::cout << "Detalhe: "
                  << erro
                  << '\n';

        sqlite3_close(banco);
        return;
    }


    // =====================================================
    // 4. CONTADORES E VALIDADOR
    // =====================================================

    std::size_t total = 0;
    std::size_t validos = 0;
    std::size_t validosComAlerta = 0;
    std::size_t rejeitados = 0;

    ValidadorRegistroTiss validador;


    // =====================================================
    // 5. CONVERSAO DOS CAMPOS TEXT
    // =====================================================

    auto obterTexto = [&](int coluna) -> std::string
    {
        const unsigned char* valor =
            sqlite3_column_text(consulta, coluna);

        if (valor == nullptr)
        {
            return "";
        }

        return reinterpret_cast<const char*>(valor);
    };


    // =====================================================
    // 6. LEITURA EM STREAMING DO SQLITE
    // =====================================================

    while ((retorno = sqlite3_step(consulta)) == SQLITE_ROW)
    {
        ++total;

        RegistroTissBruto registro(
            obterTexto(0),
            obterTexto(1),
            obterTexto(2),
            obterTexto(3),
            obterTexto(4),
            obterTexto(5),
            obterTexto(6),
            obterTexto(7),
            obterTexto(8),
            obterTexto(9),
            obterTexto(10)
        );

        ResultadoValidacao resultado =
            validador.validar(registro);

        if (resultado.getEstado() ==
            EstadoValidacao::VALIDO)
        {
            ++validos;
        }
        else if (resultado.getEstado() ==
                 EstadoValidacao::VALIDO_COM_ALERTA)
        {
            ++validosComAlerta;
        }
        else
        {
            ++rejeitados;
        }
    }


    // =====================================================
    // 7. FIM DO RESULTADO / "EQUIVALENTE AO +100"
    // =====================================================

    if (retorno == SQLITE_DONE)
    {
        if (total == 0)
        {
            std::cout
                << "Tabela registro_tiss sem registros para processamento.\n";
        }
    }
    else
    {
        std::cout
            << "ERRO durante leitura do SQLite.\n";

        std::cout
            << "Detalhe: "
            << sqlite3_errmsg(banco)
            << '\n';
    }


    // =====================================================
    // 8. LIBERACAO DOS RECURSOS
    // =====================================================

    sqlite3_finalize(consulta);
    sqlite3_close(banco);


    // =====================================================
    // 9. RESUMO
    // =====================================================

    std::cout
        << "\n===== RESUMO SQLITE -> VALIDACAO =====\n";

    std::cout
        << "Total lido:             "
        << total << '\n';

    std::cout
        << "Validos:                "
        << validos << '\n';

    std::cout
        << "Validos com alerta:     "
        << validosComAlerta << '\n';

    std::cout
        << "Rejeitados:             "
        << rejeitados << '\n';

    std::cout
        << "Banco SQLite fechado com sucesso.\n";
}
