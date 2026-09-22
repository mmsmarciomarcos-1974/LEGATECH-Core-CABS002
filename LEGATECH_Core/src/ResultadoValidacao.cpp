#include "../include/ResultadoValidacao.h"

ResultadoValidacao::ResultadoValidacao(EstadoValidacao estado)
    : estado(estado)
{
}

EstadoValidacao ResultadoValidacao::getEstado() const
{
    return estado;
}
