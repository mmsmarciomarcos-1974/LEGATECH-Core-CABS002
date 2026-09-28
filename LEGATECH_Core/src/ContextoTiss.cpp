#include "../include/ContextoTiss.h"

ContextoTiss::ContextoTiss(
    const std::string& uf,
    const std::string& competencia,
    const std::string& tipoArquivo,
    const std::string& caminhoArquivo
)
    : uf(uf),
      competencia(competencia),
      tipoArquivo(tipoArquivo),
      caminhoArquivo(caminhoArquivo)
{
}

const std::string& ContextoTiss::getUf() const
{
    return uf;
}

const std::string& ContextoTiss::getCompetencia() const
{
    return competencia;
}

const std::string& ContextoTiss::getTipoArquivo() const
{
    return tipoArquivo;
}

const std::string& ContextoTiss::getCaminhoArquivo() const
{
    return caminhoArquivo;
}
