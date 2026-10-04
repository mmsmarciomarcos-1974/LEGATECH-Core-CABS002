#include <iostream>
#include <string>
#include <exception>
#include <memory>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include "include/CatalogoTuss.h"
#include "include/SQLiteAuditoriaValidacao.h"

#include "include/IdentificadorContextoTiss.h"
#include "include/SQLiteDAOEntrada.h"
#include "include/SeletorDicionarioTiss.h"
#include "include/ContratoAmbDetAte2022.h"
#include "include/ContratoAmbDet2023Plus.h"
#include "include/CsvDAOEntrada.h"

int main(int argc, char* argv[])
try
{
    if (argc > 5)
    {
        std::cerr << "Uso: LEGATECH_Core [arquivo.csv [catalogo22.csv [auditoria.db [catalogo63.csv]]]]\n";
        return 1;
    }
    // Carregado uma vez; falha de referencia interrompe antes de ler a massa.
    std::vector<ReferenciaCatalogoTuss> referencias{
        {argc > 2 ? argv[2] : "../data/reference/tuss22_202501.csv", "22"}};
    if (argc > 4) referencias.push_back({argv[4], "63"});
    const CatalogoTuss catalogo(referencias, "202501");
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
    if (argc > 3)
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
        auditoria.get()
    );

    //csv.lerPrimeiroRegistro();

    std::cout << '\n';

    std::cout
    << "\n===== PROCESSAMENTO COMPLETO DO CSV =====\n";

    csv.processarArquivo();
    if (auditoria)
        std::cout << "Auditoria confirmada: execucao " << auditoria->getIdExecucao()
                  << " / banco " << argv[3] << '\n';

    std::cout << '\n';

    // =====================================================
    // TESTE JA EXISTENTE - LEITURA SQLITE
    // =====================================================

    //SQLiteDAOEntrada sqlite(
    //    "../data/legatech.db"
    //);

    //DAOEntrada& dao = sqlite;

    //dao.processarArquivo();

    //return 0;
}

catch (const std::exception& erro)
{
    std::cerr << "Falha de configuracao/execucao: " << erro.what() << '\n';
    return 1;
}
