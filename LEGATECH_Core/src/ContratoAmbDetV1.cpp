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

std::vector<std::string>
ContratoAmbDetV1::getCamposEsperados() const
{
    return
    {
        "ID_EVENTO_ATENCAO_SAUDE",
        "UF_PRESTADOR",
        "DT_REALIZACAO",
        "CD_PROCEDIMENTO",
        "CD_TABELA_REFERENCIA",
        "QT_ITEM_EVENTO_INFORMADO",
        "VL_ITEM_EVENTO_INFORMADO",
        "VL_ITEM_PAGO_FORNECEDOR",
        "UNIDADE_MEDIDA",
        "IND_PACOTE",
        "IND_TABELA_PROPRIA"
    };
}
