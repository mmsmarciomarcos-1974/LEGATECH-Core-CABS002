#ifndef CSVDAOENTRADA_H
#define CSVDAOENTRADA_H

#include <string>

class CsvDAOEntrada
{
private:
    std::string caminhoArquivo;

public:
    CsvDAOEntrada(const std::string& caminhoArquivo);

    void lerPrimeiroRegistro();
};

#endif
