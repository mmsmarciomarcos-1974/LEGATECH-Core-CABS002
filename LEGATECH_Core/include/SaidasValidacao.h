#ifndef SAIDASVALIDACAO_H
#define SAIDASVALIDACAO_H
#include "CsvSaidaValidacao.h"
#include "SQLiteAuditoriaValidacao.h"
#include <memory>
// Publica uma pasta completa por rename no mesmo diretorio pai.
class SaidasValidacao final : public DAOSaidaValidacao
{
    std::filesystem::path destino, temporaria;
    std::unique_ptr<CsvSaidaValidacao> csv;
    std::unique_ptr<SQLiteAuditoriaValidacao> sqlite;
public:
    explicit SaidasValidacao(const std::string& destino);
    ~SaidasValidacao() override { cancelar(); }
    void iniciar(const ContextoAuditoria& contexto) override;
    void gravar(std::size_t linha, const std::string& original,
                const RegistroTissBruto* registro, const ResultadoValidacao& resultado) override;
    void finalizar() override;
    void cancelar() noexcept override;
};
#endif
