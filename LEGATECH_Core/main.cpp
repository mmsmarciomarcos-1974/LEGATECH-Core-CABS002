#include <iostream>
#include "include/CsvDAOEntrada.h"

int main()
{
    std::cout << "LEGATECH Core - CABS002\n\n";


    CsvDAOEntrada csv(
    "../data/sample/PR_202501_AMB_DET_sample.csv"
);

    DAOEntrada& dao = csv;

    dao.processarArquivo();
    return 0;
}
