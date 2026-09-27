#include <iostream>

#include "include/CsvDAOEntrada.h"
#include "include/SQLiteDAOSaida.h"
#include "include/ProcessadorTiss.h"

int main()
{
    std::cout << "LEGATECH Core - CABS002\n\n";

    // Entrada: amostra real da ANS
    CsvDAOEntrada csv(
        "../data/sample/PR_202501_AMB_DET_sample.csv"
    );

    // Saida: banco SQLite recriado do zero
    SQLiteDAOSaida sqliteSaida(
        "../data/legatech.db"
    );

    // Polimorfismo de saida
    DAOSaida& saida = sqliteSaida;

    // Orquestra validacao + persistencia
    ProcessadorTiss processador(saida);

    // Processamento em streaming:
    // CSV -> RegistroTissBruto -> validacao -> SQLite
    csv.processarArquivo(processador);

    return 0;
}
