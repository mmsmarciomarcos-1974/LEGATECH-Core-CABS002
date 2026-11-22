#include <iostream>
#include <string>
#include <exception>
#include <memory>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <map>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "include/CatalogoTuss.h"
#include "include/SQLiteAuditoriaValidacao.h"
#include "include/SaidasValidacao.h"
#include "include/IdentificadorContextoTiss.h"
#include "include/SQLiteDAOEntrada.h"
#include "include/SeletorDicionarioTiss.h"
#include "include/ContratoAmbDetAte2022.h"
#include "include/ContratoAmbDet2023Plus.h"
#include "include/CsvDAOEntrada.h"

namespace
{
// Periodo padrao mantido para preservar as execucoes atuais.
constexpr char VERSAO_CATALOGO_TUSS_PADRAO[] = "202501";

bool competenciaValida(const std::string& valor)
{
    return valor.size() == 6 &&
        std::all_of(valor.begin(), valor.end(), [](char c) { return c >= '0' && c <= '9'; }) &&
        valor.substr(0, 4) != "0000" &&
        valor.substr(4) >= "01" && valor.substr(4) <= "12";
}

std::string extrairVersaoCatalogoTuss(int& argc, char* argv[])
{
    std::string versao = VERSAO_CATALOGO_TUSS_PADRAO;
    bool encontrouOpcao = false;
    for (int i = 1; i < argc;)
    {
        if (std::string(argv[i]) != "--catalogo-tuss")
        {
            ++i;
            continue;
        }
        if (encontrouOpcao)
            throw std::runtime_error("--catalogo-tuss informado mais de uma vez");
        if (i + 1 >= argc)
            throw std::runtime_error("Informe AAAAMM apos --catalogo-tuss");

        encontrouOpcao = true;
        versao = argv[i + 1];
        for (int j = i; j + 2 < argc; ++j)
            argv[j] = argv[j + 2];
        argc -= 2;
        argv[argc] = nullptr;
    }

    if (!competenciaValida(versao))
        throw std::runtime_error("Versao do catalogo TUSS deve usar AAAAMM valido");
    return versao;
}

std::string resolverCaminhoTuss20(const std::string& versaoCatalogo)
{
    const std::filesystem::path relativo =
        std::filesystem::path("data") / "reference" /
        (std::string("tuss20_") + versaoCatalogo + ".csv");
    std::filesystem::path base = std::filesystem::current_path();

    while (true)
    {
        const auto candidato = base / relativo;
        if (std::filesystem::is_regular_file(candidato))
            return std::filesystem::absolute(candidato).string();

        const auto pai = base.parent_path();
        if (pai == base)
            break;
        base = pai;
    }

    throw std::runtime_error(
        "Nao foi possivel localizar " + relativo.generic_string() + ".");
}

std::string resolverCaminhoTuss60(const std::string& versaoCatalogo)
{
    const std::filesystem::path relativo =
        std::filesystem::path("data") / "reference" /
        (std::string("tuss60_") + versaoCatalogo) /
        (std::string("tuss60_") + versaoCatalogo + ".csv");
    std::filesystem::path base = std::filesystem::current_path();

    while (true)
    {
        const auto candidato = base / relativo;
        if (std::filesystem::is_regular_file(candidato))
            return std::filesystem::absolute(candidato).string();

        const auto pai = base.parent_path();
        if (pai == base)
            break;
        base = pai;
    }

    throw std::runtime_error(
        "Nao foi possivel localizar " + relativo.generic_string() + ".");
}

std::vector<ReferenciaCatalogoTuss> resolverReferenciasTuss19(
    const std::string& versaoCatalogo)
{
    const std::string prefixo = "tuss19_" + versaoCatalogo + "_part";
    const std::filesystem::path relativo =
        std::filesystem::path("data") / "reference" /
        ("tuss19_" + versaoCatalogo);
    // Contagens conhecidas: validar apenas os arquivos TUSS 19 conferidos.
    static const std::map<std::string, std::vector<std::size_t>> contagensConhecidas{
        {"202501", {184598, 184598, 184598, 184598, 184598, 184598, 184594}}
    };
    std::filesystem::path base = std::filesystem::current_path();

    while (true)
    {
        const auto diretorio = base / relativo;
        std::map<std::size_t, std::filesystem::path> partes;
        if (std::filesystem::is_directory(diretorio))
        {
            for (const auto& item : std::filesystem::directory_iterator(diretorio))
            {
                if (!item.is_regular_file() || item.path().extension() != ".csv")
                    continue;
                const std::string nome = item.path().filename().string();
                if (nome.rfind(prefixo, 0) != 0)
                    continue;

                const std::string numeroTexto =
                    nome.substr(prefixo.size(), nome.size() - prefixo.size() - 4);
                if (numeroTexto.empty() ||
                    !std::all_of(numeroTexto.begin(), numeroTexto.end(),
                                 [](char c) { return c >= '0' && c <= '9'; }))
                    throw std::runtime_error("Nome de parte do catalogo TUSS 19 invalido: " + nome);

                const std::size_t numero = static_cast<std::size_t>(std::stoull(numeroTexto));
                if (numero == 0 || !partes.emplace(numero, item.path()).second)
                    throw std::runtime_error("Numero de parte do catalogo TUSS 19 duplicado ou invalido");
            }
        }

        if (!partes.empty())
        {
            const auto contagens = contagensConhecidas.find(versaoCatalogo);
            if (contagens != contagensConhecidas.end() &&
                partes.size() != contagens->second.size())
                throw std::runtime_error(
                    "Catalogo TUSS 19 incompleto: quantidade de partes divergente em " +
                    versaoCatalogo + ".");

            const std::size_t totalPartes = partes.rbegin()->first;
            if (partes.size() != totalPartes)
                throw std::runtime_error(
                    "Catalogo TUSS 19 incompleto: ha lacuna entre as partes em " +
                    versaoCatalogo + ".");

            std::vector<ReferenciaCatalogoTuss> referencias;
            for (std::size_t i = 1; i <= totalPartes; ++i)
            {
                const auto parte = partes.find(i);
                if (parte == partes.end())
                    throw std::runtime_error("Parte do catalogo TUSS 19 ausente");
                const std::size_t registrosEsperados =
                    contagens == contagensConhecidas.end() ? 0 : contagens->second[i - 1];
                referencias.push_back(
                    {parte->second.string(), "19", registrosEsperados});
            }
            return referencias;
        }

        const auto pai = base.parent_path();
        if (pai == base)
            break;
        base = pai;
    }

    return {};
}

std::string criarCaminhoExportacaoComTimestamp(const std::string& caminhoBase)
{
    const auto agora = std::chrono::system_clock::now();
    const std::time_t instante = std::chrono::system_clock::to_time_t(agora);
    const std::tm* horarioLocal = std::localtime(&instante);
    if (horarioLocal == nullptr)
        throw std::runtime_error("Nao foi possivel obter data e hora locais.");

    std::ostringstream timestamp;
    timestamp << std::put_time(horarioLocal, "%Y%m%d_%H%M%S");

    const std::filesystem::path base(caminhoBase);
    std::filesystem::path candidato = base;
    candidato += "_" + timestamp.str();

    // Se houver outra execucao no mesmo segundo, acrescenta um contador.
    unsigned int tentativa = 1;
    while (std::filesystem::exists(candidato))
    {
        std::ostringstream sufixo;
        sufixo << '_' << std::setw(2) << std::setfill('0') << tentativa++;
        candidato = base;
        candidato += "_" + timestamp.str() + sufixo.str();
    }

    return candidato.string();
}
}
int main(int argc, char* argv[])
try
{
    const std::string versaoCatalogoTuss = extrairVersaoCatalogoTuss(argc, argv);
    std::string pastaExportacao;
    if (argc >= 3 && std::string(argv[argc - 2]) == "--export")
    {
        pastaExportacao = argv[argc - 1];
        if (pastaExportacao.empty()) throw std::runtime_error("Pasta de exportacao vazia");
        pastaExportacao = criarCaminhoExportacaoComTimestamp(pastaExportacao);
        argc -= 2;
    }
    // Entrada relacional normalizada: competencia explicita, sem depender do nome do banco.
    if (argc > 1 && std::string(argv[1]) == "--sqlite")
    {
        if (argc != 5 && argc != 6)
            throw std::runtime_error("Uso: LEGATECH_Core --sqlite banco.db AAAAMM catalogo22.csv [catalogo63.csv] [--catalogo-tuss AAAAMM] [--export pasta_nova]");
        const std::string competencia = argv[3];
        if (!competenciaValida(competencia))
            throw std::runtime_error("Competencia SQLite deve usar AAAAMM com ano e mes validos");
        std::vector<ReferenciaCatalogoTuss> refs{
            {argv[4], "22"},
            {resolverCaminhoTuss20(versaoCatalogoTuss), "20"},
            {resolverCaminhoTuss60(versaoCatalogoTuss), "60"}
        };
        if (argc == 6) refs.push_back({argv[5], "63"});
        const auto referenciasTuss19 = resolverReferenciasTuss19(versaoCatalogoTuss);
        refs.insert(refs.end(), referenciasTuss19.begin(), referenciasTuss19.end());
        const CatalogoTuss catalogo(refs, versaoCatalogoTuss);
        std::cout << "LEGATECH Core - CABS002\nEntrada: SQLite / competencia " << competencia
                  << "\nCatalogo TUSS " << catalogo.getVersao() << " / tabela " << catalogo.getTabela()
                  << ": " << catalogo.tamanho() << " termos\n";
        std::unique_ptr<SaidasValidacao> saidas;
        if (!pastaExportacao.empty()) saidas = std::make_unique<SaidasValidacao>(pastaExportacao);
        SQLiteDAOEntrada sqlite(argv[2], &catalogo, competencia, saidas.get());
        // Ponteiro da base, objeto concreto SQLite: despacho virtual.
        DAOEntrada* entrada = &sqlite;
        entrada->processarArquivo();
        if (saidas) std::cout << "Exportacao confirmada: " << pastaExportacao << '\n';
        return 0;
    }
    if (argc > 5)
    {
        std::cerr << "Uso: LEGATECH_Core [arquivo.csv [catalogo22.csv [auditoria.db [catalogo63.csv]]]] ou arquivo.csv catalogo22.csv [catalogo63.csv] [--catalogo-tuss AAAAMM] --export pasta_nova\n";
        return 1;
    }
    // Carregado uma vez; falha de referencia interrompe antes de ler a massa.
    const std::string caminhoTuss22Padrao =
        (std::filesystem::path("..") / "data" / "reference" /
         (std::string("tuss22_") + versaoCatalogoTuss + ".csv")).string();
    std::vector<ReferenciaCatalogoTuss> referencias{
        {argc > 2 ? std::string(argv[2]) : caminhoTuss22Padrao, "22"},
        {resolverCaminhoTuss20(versaoCatalogoTuss), "20"},
        {resolverCaminhoTuss60(versaoCatalogoTuss), "60"}
    };
    if (!pastaExportacao.empty() && argc == 4) referencias.push_back({argv[3], "63"});
    else if (argc > 4) referencias.push_back({argv[4], "63"});
    const auto referenciasTuss19 = resolverReferenciasTuss19(versaoCatalogoTuss);
    referencias.insert(referencias.end(), referenciasTuss19.begin(), referenciasTuss19.end());
    const CatalogoTuss catalogo(referencias, versaoCatalogoTuss);
    std::cout << "Catalogo TUSS " << catalogo.getVersao()
              << " / tabela " << catalogo.getTabela()
              << ": " << catalogo.tamanho() << " termos\n";
    std::cout << "LEGATECH Core - CABS002\n\n";

    // =====================================================
    // IDENTIFICACAO DO CONTEXTO TISS
    // =====================================================

    const std::string caminhoCsv =
        //"../data/sample/PR_202501_AMB_DET_sample.csv";
        argc > 1 ? argv[1] : "../data/sample/PR_202501_AMB_DET_sample.csv";
       //   "../data/sample/PR_201501_AMB_DET.csv";
       //"../data/sample/PR_202501_AMB_DET.csv";
    IdentificadorContextoTiss identificador;

    auto contexto =
        identificador.identificar(caminhoCsv);

    if (!contexto)
    {
        std::cout
            << "Nao foi possivel identificar "
            << "o contexto TISS do arquivo.\n";

        return 1;
    }

    std::cout << "===== CONTEXTO TISS IDENTIFICADO =====\n";
    std::cout << "UF:          "
              << contexto->getUf() << '\n';

    std::cout << "Competencia: "
              << contexto->getCompetencia() << '\n';

    std::cout << "Tipo:        "
              << contexto->getTipoArquivo() << '\n';

    std::cout << "Arquivo:     "
              << contexto->getCaminhoArquivo() << "\n\n";

    SeletorDicionarioTiss seletorDicionario;

    PeriodoDicionarioTiss periodoDicionario =
        seletorDicionario.selecionar(*contexto);

    std::cout << "Dicionario selecionado: ";

    if (periodoDicionario ==
        PeriodoDicionarioTiss::ATE_2022)
    {
        std::cout << "TISS ate 2022\n";
    }
    else
    {
        std::cout << "TISS a partir de 2023\n";
    }

    std::cout << '\n';
    // =====================================================
    // IDENTIFICACAO DO CONTRATO SUPORTADO
    // =====================================================

    ContratoAmbDetAte2022 contratoAte2022;
    ContratoAmbDet2023Plus contrato2023Plus;

    ContratoTiss* contrato = nullptr;

    if (periodoDicionario ==
        PeriodoDicionarioTiss::ATE_2022)
    {
        contrato = &contratoAte2022;
    }
    else
    {
        contrato = &contrato2023Plus;
    }
    if (!contrato->suporta(*contexto))
    {
        std::cout
            << "Contexto identificado, "
            << "mas o tipo de arquivo ainda "
            << "nao possui contrato suportado.\n";

        return 1;
    }

    std::cout
        << "Contrato aplicado: "
        << contrato->getNome()
        << "\n\n";

    const auto camposEsperados =
        contrato->getCamposEsperados();

    std::cout
        << "===== ESTRUTURA ESPERADA PELO CONTRATO =====\n";

    std::cout
        << "Quantidade de campos: "
        << camposEsperados.size()
        << "\n";

    for (std::size_t i = 0; i < camposEsperados.size(); ++i)
    {
        std::cout
            << (i + 1)
            << " - "
            << camposEsperados[i]
            << '\n';
    }

    std::cout << '\n';

      std::cout
    << "===== TESTE DO CSV PELO CONTRATO =====\n";

    // Terceiro argumento habilita a auditoria; sem ele, mantem o fluxo de console.
    std::unique_ptr<SQLiteAuditoriaValidacao> auditoria;
    std::unique_ptr<SaidasValidacao> saidas;
    if (!pastaExportacao.empty())
    {
        if (argc > 4) throw std::runtime_error("Com --export use arquivo.csv catalogo22.csv [catalogo63.csv] [--export pasta_nova]");
        saidas = std::make_unique<SaidasValidacao>(pastaExportacao);
    }
    if (argc > 3 && pastaExportacao.empty())
    {
        const auto destino = std::filesystem::weakly_canonical(argv[3]);
        bool coincideReferencia = false;
        for (const auto& referencia : referencias)
            coincideReferencia |= destino == std::filesystem::weakly_canonical(referencia.caminho);
        if (destino == std::filesystem::weakly_canonical(caminhoCsv) || coincideReferencia)
            throw std::runtime_error("Banco de auditoria deve ser diferente do CSV e do catalogo");
        auditoria = std::make_unique<SQLiteAuditoriaValidacao>(argv[3]);
    }
    CsvDAOEntrada csv(
        caminhoCsv,
        *contrato,
        contexto->getCompetencia(),
        &catalogo,
        saidas ? static_cast<AuditoriaValidacao*>(saidas.get()) : auditoria.get()
    );

    //csv.lerPrimeiroRegistro();

    std::cout << '\n';

    std::cout
    << "\n===== PROCESSAMENTO COMPLETO DO CSV =====\n";

    // O mesmo contrato usado na entrada SQLite; ponteiro nao proprietario.
    DAOEntrada* entrada = &csv;
    entrada->processarArquivo();
    if (saidas) std::cout << "Exportacao confirmada: " << pastaExportacao << '\n';
    if (auditoria)
        std::cout << "Auditoria confirmada: execucao " << auditoria->getIdExecucao()
                  << " / banco " << argv[3] << '\n';

    std::cout << '\n';

    return 0;
}

catch (const std::exception& erro)
{
    std::cerr << "Falha de configuracao/execucao: " << erro.what() << '\n';
    return 1;
}
