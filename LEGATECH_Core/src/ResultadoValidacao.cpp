#include "../include/ResultadoValidacao.h"

ResultadoValidacao::ResultadoValidacao(
    EstadoValidacao estado
)
    : estado(estado)
{
}

EstadoValidacao
ResultadoValidacao::getEstado() const
{
    return estado;
}

void ResultadoValidacao::adicionarMotivo(
    const std::string& motivo
)
{
    motivos.push_back(motivo);

    // Qualquer motivo de rejeicao torna
    // o resultado final REJEITADO.
    estado = EstadoValidacao::REJEITADO;
}

void ResultadoValidacao::adicionarAlerta(
    const std::string& alerta
)
{
    alertas.push_back(alerta);

    // Um alerta muda VALIDO para VALIDO_COM_ALERTA,
    // mas nunca transforma um REJEITADO novamente
    // em registro valido.
    if (estado != EstadoValidacao::REJEITADO)
    {
        estado =
            EstadoValidacao::VALIDO_COM_ALERTA;
    }
}

const std::vector<std::string>&
ResultadoValidacao::getMotivos() const
{
    return motivos;
}

const std::vector<std::string>&
ResultadoValidacao::getAlertas() const
{
    return alertas;
}

bool ResultadoValidacao::temMotivos() const
{
    return !motivos.empty();
}

bool ResultadoValidacao::temAlertas() const
{
    return !alertas.empty();
}
