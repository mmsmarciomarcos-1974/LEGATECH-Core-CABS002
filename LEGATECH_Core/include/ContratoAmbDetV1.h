#ifndef CONTRATOAMBDETV1_H
#define CONTRATOAMBDETV1_H

#include "ContratoTiss.h"

class ContratoAmbDetV1 : public ContratoTiss
{
public:
    bool suporta(
        const ContextoTiss& contexto
    ) const override;

    std::string getNome() const override;
};

#endif
