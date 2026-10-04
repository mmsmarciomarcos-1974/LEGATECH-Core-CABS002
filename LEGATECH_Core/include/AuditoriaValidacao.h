#ifndef AUDITORIAVALIDACAO_H
#define AUDITORIAVALIDACAO_H

#include "ResultadoValidacao.h"
#include "RegistroTissBruto.h"
#include <cstddef>
#include <string>

struct ContextoAuditoria
{
    std::string arquivo, competencia, contrato, versaoCatalogo, tabelaCatalogo, cabecalho;
};

// Uma execucao completa e confirmada; qualquer falha cancela seus registros.
// A linha fisica permite localizar tambem registros que nao formam um objeto TISS.
class AuditoriaValidacao
{
public:
    virtual ~AuditoriaValidacao() = default;
    virtual void iniciar(const ContextoAuditoria& contexto) = 0;
    virtual void gravar(std::size_t linhaFisica, const std::string& linhaOriginal,
                        const RegistroTissBruto* registro,
                        const ResultadoValidacao& resultado) = 0;
    virtual void finalizar() = 0;
    virtual void cancelar() noexcept = 0;
};

#endif
