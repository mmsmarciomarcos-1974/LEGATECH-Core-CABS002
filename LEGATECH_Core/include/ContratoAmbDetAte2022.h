#ifndef CONTRATOAMBDETATE2022_H
#define CONTRATOAMBDETATE2022_H

#include "ContratoTiss.h"

class ContratoAmbDetAte2022 : public ContratoTiss
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
