#include <iostream>
#include "include/CsvDAOEntrada.h"

int main()
{
    std::cout << "LEGATECH Core - CABS002\n\n";

    CsvDAOEntrada dao(
        "../data/sample/PR_202501_AMB_DET_sample.csv"
    );

    dao.lerPrimeiroRegistro();

    return 0;
}
