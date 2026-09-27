#include <iostream>

#include "include/SQLiteDAOEntrada.h"

int main()
{
    std::cout << "LEGATECH Core - CABS002\n\n";

    SQLiteDAOEntrada sqlite(
        "../data/legatech.db"
    );

    DAOEntrada& dao = sqlite;

    dao.processarArquivo();

    return 0;
}
