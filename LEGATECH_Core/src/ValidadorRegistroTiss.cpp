#include "../include/ValidadorRegistroTiss.h"

ResultadoValidacao ValidadorRegistroTiss::validar(
    const RegistroTissBruto& registro
) const
{
    // Primeira regra real de validacao:
    // UF_PRESTADOR e obrigatoria.

    if (registro.getUfPrestador().empty())
    {
        return ResultadoValidacao(
            EstadoValidacao::REJEITADO
        );
    }

    return ResultadoValidacao(
        EstadoValidacao::VALIDO
    );
}
