#ifndef CONTRATOTISS_H
#define CONTRATOTISS_H

#include <string>
#include <vector>

#include "ContextoTiss.h"

class ContratoTiss
{
public:
    virtual ~ContratoTiss() = default;

    virtual bool suporta(
        const ContextoTiss& contexto
    ) const = 0;

    virtual std::string getNome() const = 0;

    virtual std::vector<std::string>
    getCamposEsperados() const = 0;
};

#endif
