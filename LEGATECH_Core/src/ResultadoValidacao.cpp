#include "../include/ResultadoValidacao.h"

ResultadoValidacao::ResultadoValidacao(
    EstadoValidacao estado
)
    : estado(estado)
{
}

EstadoValidacao ResultadoValidacao::getEstado() const
{
    return estado;
}

void ResultadoValidacao::adicionarMotivo(
    const std::string& motivo
)
{
    motivos.push_back(motivo);
}

const std::vector<std::string>&
ResultadoValidacao::getMotivos() const
{
    return motivos;
}

bool ResultadoValidacao::temMotivos() const
{
    return !motivos.empty();
}
