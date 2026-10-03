#ifndef VALIDADORREGISTROTISS_H
#define VALIDADORREGISTROTISS_H

#include <string>

#include "RegistroTissBruto.h"
#include "ResultadoValidacao.h"

class ValidadorRegistroTiss
{
public:
    // Validacao sem contexto externo.
    ResultadoValidacao validar(
        const RegistroTissBruto& registro
    ) const;

    // Validacao considerando a competencia
    // identificada no arquivo.
    ResultadoValidacao validar(
        const RegistroTissBruto& registro,
        const std::string& competenciaArquivo
    ) const;
};

#endif
