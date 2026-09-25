#include "../include/SQLiteDAOEntrada.h"

#include <iostream>

SQLiteDAOEntrada::SQLiteDAOEntrada(
    const std::string& caminhoBanco
)
    : caminhoBanco(caminhoBanco)
{
}

void SQLiteDAOEntrada::processarArquivo()
{
    std::cout << "Processamento SQLite iniciado.\n";
    std::cout << "Banco: " << caminhoBanco << '\n';
}
