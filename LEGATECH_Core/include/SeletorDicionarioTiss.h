#ifndef SELETORDICIONARIOTISS_H
#define SELETORDICIONARIOTISS_H

#include "ContextoTiss.h"

enum class PeriodoDicionarioTiss
{
    ATE_2022,
    A_PARTIR_2023
};

class SeletorDicionarioTiss
{
public:
    PeriodoDicionarioTiss selecionar(
        const ContextoTiss& contexto
    ) const;
};

#endif
