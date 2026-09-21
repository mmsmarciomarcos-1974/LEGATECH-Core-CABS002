#ifndef REGISTROTISSBRUTO_H
#define REGISTROTISSBRUTO_H

#include <string>

class RegistroTissBruto
{
private:
    std::string idEventoAtencaoSaude;
    std::string ufPrestador;
    std::string dtRealizacao;
    std::string cdProcedimento;
    std::string cdTabelaReferencia;
    std::string qtItemEventoInformado;
    std::string vlItemEventoInformado;
    std::string vlItemPagoFornecedor;
    std::string unidadeMedida;
    std::string indPacote;
    std::string indTabelaPropria;

public:
    RegistroTissBruto();

    RegistroTissBruto(
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
    );

    const std::string& getIdEventoAtencaoSaude() const;
    const std::string& getUfPrestador() const;
    const std::string& getDtRealizacao() const;
};

#endif
