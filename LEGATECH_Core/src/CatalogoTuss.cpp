#include "../include/CatalogoTuss.h"

#include <fstream>
#include <stdexcept>
#include <vector>
#include <algorithm>

namespace
{
// CSV com aspas duplicadas, ponto e virgula e quebras de linha citadas.
// Leitura estrita: arquivos truncados nao podem virar um catalogo parcial.
bool lerLinhaCsv(std::istream& entrada, std::vector<std::string>& campos)
{
    campos.clear();
    std::string campo;
    bool citado = false, fechouAspas = false, iniciou = false;
    char c;
    while (entrada.get(c))
    {
        iniciou = true;
        if (citado)
        {
            if (c == '"')
            {
                if (entrada.peek() == '"') { entrada.get(c); campo += '"'; }
                else { citado = false; fechouAspas = true; }
            }
            else campo += c;
            continue;
        }
        if (c == ';')
        {
            campos.push_back(campo); campo.clear(); fechouAspas = false;
        }
        else if (c == '\n' || c == '\r')
        {
            if (c == '\r' && entrada.peek() == '\n') entrada.get(c);
            campos.push_back(campo); return true;
        }
        else if (c == '"' && campo.empty() && !fechouAspas) citado = true;
        else
        {
            if (fechouAspas || c == '"')
                throw std::runtime_error("CSV TUSS: aspas invalidas");
            campo += c;
        }
    }
    if (!entrada.eof()) throw std::runtime_error("Erro de leitura do catalogo TUSS");
    if (citado) throw std::runtime_error("CSV TUSS: campo citado truncado");
    if (iniciou) campos.push_back(campo);
    return iniciou;
}

bool digitos(const std::string& valor)
{
    if (valor.empty()) return false;
    for (char c : valor) if (c < '0' || c > '9') return false;
    return true;
}

std::chrono::sys_days dataIso(const std::string& valor)
{
    if (valor.size() != 10 || valor[4] != '-' || valor[7] != '-' ||
        !digitos(valor.substr(0, 4)) || !digitos(valor.substr(5, 2)) ||
        !digitos(valor.substr(8, 2)))
        throw std::runtime_error("Catalogo TUSS: data ISO invalida");
    using namespace std::chrono;
    year_month_day data{year{std::stoi(valor.substr(0, 4))},
                        month{static_cast<unsigned>(std::stoi(valor.substr(5, 2)))},
                        day{static_cast<unsigned>(std::stoi(valor.substr(8, 2)))}};
    if (!data.ok() || int(data.year()) < 1)
        throw std::runtime_error("Catalogo TUSS: data inexistente");
    return sys_days{data};
}
}

CatalogoTuss::CatalogoTuss(const std::string& caminho,
                         const std::string& versaoEsperada,
                         const std::string& tabelaEsperada)
    : CatalogoTuss(std::vector<ReferenciaCatalogoTuss>{{caminho, tabelaEsperada}}, versaoEsperada)
{}

CatalogoTuss::CatalogoTuss(const std::vector<ReferenciaCatalogoTuss>& referencias,
                         const std::string& versaoEsperada)
    : versao(versaoEsperada)
{
    if (versao.empty() || referencias.empty())
        throw std::runtime_error("Catalogo TUSS: versao/tabela nao informadas");
    for (const auto& referencia : referencias)
    {
        const bool tabelaJaCarregada = contemTabela(referencia.tabela);
        if (!digitos(referencia.tabela) ||
            (tabelaJaCarregada && referencia.tabela != "19"))
            throw std::runtime_error("Catalogo TUSS: tabela invalida ou repetida");
        carregar(referencia);
        if (!tabelaJaCarregada)
        {
            tabelas.push_back(referencia.tabela);
            if (!tabela.empty()) tabela += ',';
            tabela += referencia.tabela;
        }
    }
}

void CatalogoTuss::carregar(const ReferenciaCatalogoTuss& referencia)
{
    const auto& caminho = referencia.caminho;
    const auto& tabelaEsperada = referencia.tabela;
    const auto tamanhoAnterior = termos.size();
    std::ifstream entrada(caminho, std::ios::binary);
    if (!entrada) throw std::runtime_error("Nao foi possivel abrir catalogo TUSS: " + caminho);
    std::vector<std::string> campos;
    const std::vector<std::string> cabecalhoCompleto{
        "versao_catalogo", "cd_tabela_referencia", "cd_procedimento", "termo",
        "descricao_detalhada", "inicio_vigencia", "fim_vigencia", "fim_implantacao"};
    const std::vector<std::string> cabecalhoTuss19{
        "cd_procedimento", "inicio_vigencia", "fim_vigencia"};
    if (!lerLinhaCsv(entrada, campos)) throw std::runtime_error("Catalogo TUSS vazio");
    if (campos[0].starts_with("\xEF\xBB\xBF")) campos[0].erase(0, 3);
    const bool formatoCompactoTuss19 = tabelaEsperada == "19" && campos == cabecalhoTuss19;
    if (!formatoCompactoTuss19 && campos != cabecalhoCompleto)
        throw std::runtime_error("Cabecalho do catalogo TUSS divergente");
    std::size_t registrosCarregados = 0;
    while (lerLinhaCsv(entrada, campos))
    {
        const std::size_t indiceCodigo = formatoCompactoTuss19 ? 0 : 2;
        const std::size_t indiceInicio = formatoCompactoTuss19 ? 1 : 5;
        const std::size_t indiceFim = formatoCompactoTuss19 ? 2 : 6;
        const bool estruturaValida = formatoCompactoTuss19
            ? campos.size() == 3
            : campos.size() == 8 && campos[0] == versao && campos[1] == tabelaEsperada;
        if (!estruturaValida)
            throw std::runtime_error("Registro do catalogo TUSS divergente");
        const std::string& codigo = campos[indiceCodigo];
        const bool formatoCodigoValido = digitos(codigo) &&
            (tabelaEsperada != "22" || codigo.size() == 8) &&
            (tabelaEsperada != "63" || codigo.size() == 3) &&
            (tabelaEsperada != "60" || codigo.size() == 3) &&
            (tabelaEsperada != "19" || (codigo.size() == 8 || codigo.size() == 9));
        if (!formatoCodigoValido)
            throw std::runtime_error("Registro do catalogo TUSS divergente");
        TermoTuss termo{dataIso(campos[indiceInicio]), std::nullopt};
        if (!campos[indiceFim].empty()) termo.fimVigencia = dataIso(campos[indiceFim]);
        if (!formatoCompactoTuss19 && !campos[7].empty()) (void)dataIso(campos[7]);
        if (termo.fimVigencia && *termo.fimVigencia < termo.inicioVigencia)
            throw std::runtime_error("Catalogo TUSS: intervalo invertido");
        if (!termos.emplace(tabelaEsperada + ':' + codigo, termo).second)
            throw std::runtime_error("Catalogo TUSS: codigo duplicado " + codigo);
        ++registrosCarregados;
    }
    if (registrosCarregados == 0) throw std::runtime_error("Catalogo TUSS sem termos");
    if (referencia.registrosEsperados != 0 && registrosCarregados != referencia.registrosEsperados)
        throw std::runtime_error("Catalogo TUSS: quantidade de registros divergente em " + caminho);
    if (termos.size() == tamanhoAnterior) throw std::runtime_error("Catalogo TUSS sem termos");
}

bool CatalogoTuss::contemTabela(const std::string& tabelaReferencia) const
{
    return std::find(tabelas.begin(), tabelas.end(), tabelaReferencia) != tabelas.end();
}

const TermoTuss* CatalogoTuss::consultar(const std::string& tabelaReferencia,
                                       const std::string& procedimento) const
{
    const auto it = termos.find(tabelaReferencia + ':' + procedimento);
    return it == termos.end() ? nullptr : &it->second;
}
