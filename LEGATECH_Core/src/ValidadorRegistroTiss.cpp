#include "../include/ValidadorRegistroTiss.h"

ResultadoValidacao ValidadorRegistroTiss::validar(
    const RegistroTissBruto& registro
) const
{
    // =====================================================
    // REGRA 1 - UF_PRESTADOR OBRIGATORIA
    // =====================================================

    if (registro.getUfPrestador().empty())
    {
        ResultadoValidacao resultado(
            EstadoValidacao::REJEITADO
        );

        resultado.adicionarMotivo(
            "UF_PRESTADOR: campo obrigatorio nao informado"
        );

        return resultado;
    }


    // =====================================================
    // REGISTRO VALIDO ATE ESTE PONTO
    // =====================================================

    return ResultadoValidacao(
        EstadoValidacao::VALIDO
    );
}
