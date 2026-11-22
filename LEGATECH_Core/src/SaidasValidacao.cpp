#include "../include/SaidasValidacao.h"
#include <chrono>
#include <stdexcept>
SaidasValidacao::SaidasValidacao(const std::string& caminho) : destino(caminho)
{
    if (caminho.empty() || std::filesystem::exists(destino))
        throw std::runtime_error("Pasta de exportacao deve ser nova e nao vazia");
    destino = std::filesystem::absolute(destino).lexically_normal();
    if (!std::filesystem::is_directory(destino.parent_path()))
        throw std::runtime_error("Diretorio pai da exportacao nao existe");
}
void SaidasValidacao::iniciar(const ContextoAuditoria& contexto)
{
    if (!temporaria.empty()) throw std::logic_error("Exportacao ja iniciada");
    const auto token = std::chrono::steady_clock::now().time_since_epoch().count();
    for (int i = 0; i < 100; ++i)
    {
        auto candidato = destino;
        candidato += ".tmp-" + std::to_string(token) + "-" + std::to_string(i);
        if (std::filesystem::create_directory(candidato)) { temporaria = candidato; break; }
    }
    if (temporaria.empty()) throw std::runtime_error("Nao foi possivel criar pasta temporaria");
    try
    {
        csv = std::make_unique<CsvSaidaValidacao>(temporaria);
        sqlite = std::make_unique<SQLiteAuditoriaValidacao>(
            caminhoArquivoExportado(temporaria, "resultados", ".db").string());
        DAOSaidaValidacao* saida = csv.get(); saida->iniciar(contexto);
        saida = sqlite.get(); saida->iniciar(contexto);
    }
    catch (...) { cancelar(); throw; }
}
void SaidasValidacao::gravar(std::size_t linha, const std::string& original,
                           const RegistroTissBruto* registro, const ResultadoValidacao& resultado)
{
    if (!csv || !sqlite) throw std::logic_error("Exportacao nao iniciada");
    try
    {
        csv->gravar(linha, original, registro, resultado);
        sqlite->gravar(linha, original, registro, resultado);
    }
    catch (...) { cancelar(); throw; }
}
void SaidasValidacao::finalizar()
{
    if (!csv || !sqlite) throw std::logic_error("Exportacao nao iniciada");
    try
    {
        csv->finalizar(); sqlite->finalizar();
        csv.reset(); sqlite.reset(); // Fecha tambem o banco antes do rename no Windows.
        if (std::filesystem::exists(destino)) throw std::runtime_error("Destino criado durante exportacao");
        std::filesystem::rename(temporaria, destino);
        temporaria.clear();
    }
    catch (...) { cancelar(); throw; }
}
void SaidasValidacao::cancelar() noexcept
{
    if (csv) csv->cancelar();
    if (sqlite) sqlite->cancelar();
    csv.reset(); sqlite.reset();
    if (!temporaria.empty())
    {
        std::error_code erro;
        std::filesystem::remove_all(temporaria, erro);
        temporaria.clear();
    }
}
