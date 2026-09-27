#include "../include/ProcessadorTiss.h"

ProcessadorTiss::ProcessadorTiss(
    DAOSaida& saida
)
    : saida(saida)
{
}

ResultadoValidacao ProcessadorTiss::processar(
    const RegistroTissBruto& registro
)
{
    ResultadoValidacao resultado =
        validador.validar(registro);

    if (resultado.getEstado() ==
        EstadoValidacao::VALIDO)
    {
        saida.gravar(registro);
    }

    return resultado;
}
