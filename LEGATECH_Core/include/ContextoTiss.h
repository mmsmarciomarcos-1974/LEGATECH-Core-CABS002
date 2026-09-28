#ifndef CONTEXTOTISS_H
#define CONTEXTOTISS_H

#include <string>

class ContextoTiss
{
private:
    std::string uf;
    std::string competencia;
    std::string tipoArquivo;
    std::string caminhoArquivo;

public:
    ContextoTiss(
        const std::string& uf,
        const std::string& competencia,
        const std::string& tipoArquivo,
        const std::string& caminhoArquivo
    );

    const std::string& getUf() const;
    const std::string& getCompetencia() const;
    const std::string& getTipoArquivo() const;
    const std::string& getCaminhoArquivo() const;
};

#endif
