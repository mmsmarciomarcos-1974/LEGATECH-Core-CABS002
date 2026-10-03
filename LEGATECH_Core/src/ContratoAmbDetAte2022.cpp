#include "../include/ContratoAmbDetAte2022.h"

bool ContratoAmbDetAte2022::suporta(
    const ContextoTiss& contexto
) const
{
    return contexto.getTipoArquivo() == "AMB_DET"
        && contexto.getCompetencia() <= "202212";
}

std::string ContratoAmbDetAte2022::getNome() const
{
    return "ANS_TISS_AMB_DET_ATE_2022";
}

std::vector<std::string>
ContratoAmbDetAte2022::getCamposEsperados() const
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
        "IND_PACOTE",
        "IND_TABELA_PROPRIA"
    };
}
