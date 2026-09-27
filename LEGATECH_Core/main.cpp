#include <iostream>

#include "include/SQLiteDAOSaida.h"
#include "include/RegistroTissBruto.h"

int main()
{
    std::cout << "LEGATECH Core - CABS002\n\n";

    RegistroTissBruto registro(
        "6450733769",
        "PR",
        "2025-01",
        "10101012",
        "22",
        "1",
        "200",
        "0",
        "",
        "0",
        "0"
    );

    SQLiteDAOSaida sqliteSaida(
        "../data/legatech.db"
    );

    DAOSaida& saida = sqliteSaida;

    saida.gravar(registro);

    return 0;
}
