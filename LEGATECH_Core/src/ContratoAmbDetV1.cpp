#include "../include/ContratoAmbDetV1.h"

bool ContratoAmbDetV1::suporta(
    const ContextoTiss& contexto
) const
{
    return contexto.getTipoArquivo() == "AMB_DET";
}

std::string ContratoAmbDetV1::getNome() const
{
    return "ANS_TISS_AMB_DET_V1";
}
