#include "../include/RegistroTissBruto.h"

RegistroTissBruto::RegistroTissBruto()
{
}
RegistroTissBruto::RegistroTissBruto(
    const std::string& idEventoAtencaoSaude,
    const std::string& ufPrestador,
    const std::string& dtRealizacao,
    const std::string& cdProcedimento,
    const std::string& cdTabelaReferencia,
    const std::string& qtItemEventoInformado,
    const std::string& vlItemEventoInformado,
    const std::string& vlItemPagoFornecedor,
    const std::string& unidadeMedida,
    const std::string& indPacote,
    const std::string& indTabelaPropria
)
    : idEventoAtencaoSaude(idEventoAtencaoSaude),
      ufPrestador(ufPrestador),
      dtRealizacao(dtRealizacao),
      cdProcedimento(cdProcedimento),
      cdTabelaReferencia(cdTabelaReferencia),
      qtItemEventoInformado(qtItemEventoInformado),
      vlItemEventoInformado(vlItemEventoInformado),
      vlItemPagoFornecedor(vlItemPagoFornecedor),
      unidadeMedida(unidadeMedida),
      indPacote(indPacote),
      indTabelaPropria(indTabelaPropria)
{
}
const std::string& RegistroTissBruto::getIdEventoAtencaoSaude() const
{
    return idEventoAtencaoSaude;
}

const std::string& RegistroTissBruto::getUfPrestador() const
{
    return ufPrestador;
}

const std::string& RegistroTissBruto::getDtRealizacao() const
{
    return dtRealizacao;
}
