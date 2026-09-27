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

const std::string& RegistroTissBruto::getCdProcedimento() const
{
    return cdProcedimento;
}

const std::string& RegistroTissBruto::getCdTabelaReferencia() const
{
    return cdTabelaReferencia;
}

const std::string& RegistroTissBruto::getQtItemEventoInformado() const
{
    return qtItemEventoInformado;
}

const std::string& RegistroTissBruto::getVlItemEventoInformado() const
{
    return vlItemEventoInformado;
}

const std::string& RegistroTissBruto::getVlItemPagoFornecedor() const
{
    return vlItemPagoFornecedor;
}

const std::string& RegistroTissBruto::getUnidadeMedida() const
{
    return unidadeMedida;
}

const std::string& RegistroTissBruto::getIndPacote() const
{
    return indPacote;
}

const std::string& RegistroTissBruto::getIndTabelaPropria() const
{
    return indTabelaPropria;
}

