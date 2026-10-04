#ifndef PROCESSADORTISS_H
#define PROCESSADORTISS_H

#include "DAOSaida.h"
#include "ValidadorRegistroTiss.h"
#include "ResultadoValidacao.h"

class ProcessadorTiss
{
private:
    DAOSaida& saida;
    ValidadorRegistroTiss validador;

public:
    ProcessadorTiss(DAOSaida& saida, const CatalogoTuss* catalogo = nullptr);

    ResultadoValidacao processar(
        const RegistroTissBruto& registro
    );
};

#endif
