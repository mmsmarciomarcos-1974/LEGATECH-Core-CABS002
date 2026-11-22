#include "../include/ContratoAmbDet2023Plus.h"

bool ContratoAmbDet2023Plus::suporta(
    const ContextoTiss& contexto
) const
{
    return contexto.getTipoArquivo() == "AMB_DET"
        && contexto.getCompetencia() >= "202301";
}

std::string ContratoAmbDet2023Plus::getNome() const
{
    return "ANS_TISS_AMB_DET_2023_PLUS";
}

std::vector<std::string>
ContratoAmbDet2023Plus::getCamposEsperados() const
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
        "QT_ITEM_EVENTO_PAGO",
        "VL_ITEM_EVENTO_PAGO",
        "UNIDADE_MEDIDA",
        "IND_PACOTE",
        "IND_TABELA_PROPRIA"
    };
}
