#ifndef CATALOGOTUSS_H
#define CATALOGOTUSS_H

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>

struct TermoTuss
{
    std::chrono::sys_days inicioVigencia;
    std::optional<std::chrono::sys_days> fimVigencia;
};

// Catalogo imutavel apos a carga: uma leitura, muitas consultas.
// A versao e a tabela sao explicitas; nao sao inferidas da competencia.
class CatalogoTuss
{
    std::string versao;
    std::string tabela;
    std::unordered_map<std::string, TermoTuss> termos;

public:
    CatalogoTuss(const std::string& caminho,
                 const std::string& versaoEsperada,
                 const std::string& tabelaEsperada);
    const TermoTuss* consultar(const std::string& tabelaReferencia,
                              const std::string& procedimento) const;
    const std::string& getVersao() const { return versao; }
    const std::string& getTabela() const { return tabela; }
    std::size_t tamanho() const { return termos.size(); }
};

#endif
