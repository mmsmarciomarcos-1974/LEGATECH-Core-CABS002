#ifndef IDENTIFICADORCONTEXTOTISS_H
#define IDENTIFICADORCONTEXTOTISS_H

#include <optional>
#include <string>

#include "ContextoTiss.h"

class IdentificadorContextoTiss
{
public:
    std::optional<ContextoTiss> identificar(
        const std::string& caminhoArquivo
    ) const;
};

#endif
