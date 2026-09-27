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

    // A leitura dos registros SQLite sera implementada aqui.
    // Este DAO nao cria nem grava registros.

    sqlite3_close(banco);

    std::cout << "Banco SQLite fechado com sucesso.\n";
}
