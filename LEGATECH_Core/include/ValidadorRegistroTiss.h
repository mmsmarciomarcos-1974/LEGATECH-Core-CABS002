#ifndef VALIDADORREGISTROTISS_H
#define VALIDADORREGISTROTISS_H

#include "RegistroTissBruto.h"
#include "ResultadoValidacao.h"

class ValidadorRegistroTiss
{
public:
    ResultadoValidacao validar(
        const RegistroTissBruto& registro
    ) const;
};

#endif
