#ifndef CSVDAOENTRADA_H
#define CSVDAOENTRADA_H
#include "../include/ResultadoValidacao.h"
#include <string>

class CsvDAOEntrada
{
private:
    std::string caminhoArquivo;

public:
    CsvDAOEntrada(const std::string& caminhoArquivo);

    void lerPrimeiroRegistro();
    void processarArquivo();

    bool validarCabecalho(const std::string& cabecalho);
};

#endif
