#ifndef CSVDAOENTRADA_H
#define CSVDAOENTRADA_H

#include <string>
#include <vector>

#include "DAOEntrada.h"
#include "ProcessadorTiss.h"
#include "ContratoTiss.h"

class CsvDAOEntrada : public DAOEntrada
{
private:
    std::string caminhoArquivo;
    const ContratoTiss& contrato;

public:
CsvDAOEntrada(
    const std::string& caminhoArquivo,
    const ContratoTiss& contrato
);

    void lerPrimeiroRegistro();

    void processarArquivo() override;

    void processarArquivo(
        ProcessadorTiss& processador
    );

    bool validarCabecalho(
        const std::string& cabecalho,
        const ContratoTiss& contrato
    );

    std::vector<std::string> normalizarCampos(
        const std::vector<std::string>& campos,
        const ContratoTiss& contrato
    ) const;
};

#endif
