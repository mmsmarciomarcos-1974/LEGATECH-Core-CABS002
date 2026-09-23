#include "../include/ResumoProcessamento.h"

ResumoProcessamento::ResumoProcessamento()
    : total(0),
      validos(0),
      validosComAlerta(0),
      rejeitados(0),
      estruturaDivergente(0)
{
}

void ResumoProcessamento::incrementarTotal()
{
    ++total;
}

void ResumoProcessamento::incrementarValidos()
{
    ++validos;
}

void ResumoProcessamento::incrementarValidosComAlerta()
{
    ++validosComAlerta;
}

void ResumoProcessamento::incrementarRejeitados()
{
    ++rejeitados;
}

void ResumoProcessamento::incrementarEstruturaDivergente()
{
    ++estruturaDivergente;
}

std::size_t ResumoProcessamento::getTotal() const
{
    return total;
}

std::size_t ResumoProcessamento::getValidos() const
{
    return validos;
}

std::size_t ResumoProcessamento::getValidosComAlerta() const
{
    return validosComAlerta;
}

std::size_t ResumoProcessamento::getRejeitados() const
{
    return rejeitados;
}

std::size_t ResumoProcessamento::getEstruturaDivergente() const
{
    return estruturaDivergente;
}
