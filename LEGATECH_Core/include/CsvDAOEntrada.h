#ifndef CSVDAOENTRADA_H
#define CSVDAOENTRADA_H
#include "../include/ResultadoValidacao.h"
#include <string>
#include "DAOEntrada.h"
#include "ProcessadorTiss.h"

/*class CsvDAOEntrada*/
class CsvDAOEntrada : public DAOEntrada
{
private:
    std::string caminhoArquivo;

public:
    CsvDAOEntrada(const std::string& caminhoArquivo);

    void lerPrimeiroRegistro();
    /*void processarArquivo();*/
    /*class CsvDAOEntrada : public DAOEntrada*/
    void processarArquivo() override;
        void processarArquivo(
            ProcessadorTiss& processador
);

    bool validarCabecalho(const std::string& cabecalho);
};

#endif
