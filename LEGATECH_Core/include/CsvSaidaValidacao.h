#ifndef CSVSAIDAVALIDACAO_H
#define CSVSAIDAVALIDACAO_H
#include "AuditoriaValidacao.h"
#include <fstream>
#include <filesystem>
#include <regex>
#include <string>
#include <vector>
std::vector<std::string> camposRegistro(const RegistroTissBruto& registro);
std::string linhaCsv(const std::vector<std::string>& campos);

// Recupera o mesmo timestamp da pasta final, inclusive ao receber a pasta temporaria.
inline std::string timestampDaPastaExportacao(const std::filesystem::path& pasta)
{
    const std::string nome = pasta.filename().string();
    static const std::regex padrao(R"(.*_([0-9]{8}_[0-9]{6}(_[0-9]{2})?)(\.tmp-.*)?)");
    std::smatch resultado;
    if (std::regex_match(nome, resultado, padrao))
        return resultado[1].str();
    return {};
}

inline std::filesystem::path caminhoArquivoExportado(
    const std::filesystem::path& pasta, const std::string& nomeBase, const std::string& extensao)
{
    const std::string timestamp = timestampDaPastaExportacao(pasta);
    return pasta / (nomeBase + (timestamp.empty() ? std::string() : "_" + timestamp) + extensao);
}

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
