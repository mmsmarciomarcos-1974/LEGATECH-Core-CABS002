#include "../include/ProcessadorTiss.h"

ProcessadorTiss::ProcessadorTiss(
    DAOSaida& saida,
    const CatalogoTuss* catalogo
)
    : saida(saida), validador(catalogo)
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
