#ifndef CONTRATOAMBDET2023PLUS_H
#define CONTRATOAMBDET2023PLUS_H

#include "ContratoTiss.h"

class ContratoAmbDet2023Plus : public ContratoTiss
{
public:
    bool suporta(
        const ContextoTiss& contexto
    ) const override;

    std::string getNome() const override;

    std::vector<std::string>
    getCamposEsperados() const override;
};

#endif
