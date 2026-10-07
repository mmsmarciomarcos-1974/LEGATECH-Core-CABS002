#ifndef CSVSAIDAVALIDACAO_H
#define CSVSAIDAVALIDACAO_H
#include "AuditoriaValidacao.h"
#include <fstream>
#include <filesystem>
#include <vector>
std::vector<std::string> camposRegistro(const RegistroTissBruto& registro);
std::string linhaCsv(const std::vector<std::string>& campos);
class CsvSaidaValidacao final : public DAOSaidaValidacao
{
    std::filesystem::path pasta;
    std::ofstream aceitos, rejeitados;
    bool ativa = false;
public:
    explicit CsvSaidaValidacao(const std::filesystem::path& pasta) : pasta(pasta) {}
    ~CsvSaidaValidacao() override { cancelar(); }
    void iniciar(const ContextoAuditoria& contexto) override;
    void gravar(std::size_t linha, const std::string& original,
                const RegistroTissBruto* registro, const ResultadoValidacao& resultado) override;
    void finalizar() override;
    void cancelar() noexcept override;
};
#endif
