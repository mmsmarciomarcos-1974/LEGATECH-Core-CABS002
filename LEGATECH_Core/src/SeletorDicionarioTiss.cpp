#include "../include/SeletorDicionarioTiss.h"

PeriodoDicionarioTiss
SeletorDicionarioTiss::selecionar(
    const ContextoTiss& contexto
) const
{
    const std::string& competencia =
        contexto.getCompetencia();

    if (competencia <= "202212")
    {
        return PeriodoDicionarioTiss::ATE_2022;
    }

    return PeriodoDicionarioTiss::A_PARTIR_2023;
}
