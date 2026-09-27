#ifndef DAOSAIDA_H
#define DAOSAIDA_H

#include "RegistroTissBruto.h"

class DAOSaida
{
public:
    virtual ~DAOSaida() = default;

    virtual void gravar(
        const RegistroTissBruto& registro
    ) = 0;
};

#endif
