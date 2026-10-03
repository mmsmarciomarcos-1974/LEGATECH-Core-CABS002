#include <iostream>
#include <string>

#include "include/IdentificadorContextoTiss.h"
#include "include/SQLiteDAOEntrada.h"
#include "include/SeletorDicionarioTiss.h"
#include "include/ContratoAmbDetAte2022.h"
#include "include/ContratoAmbDet2023Plus.h"
#include "include/CsvDAOEntrada.h"

int main()
{
    std::cout << "LEGATECH Core - CABS002\n\n";

    // =====================================================
    // IDENTIFICACAO DO CONTEXTO TISS
    // =====================================================

    const std::string caminhoCsv =
       // "../data/sample/PR_202501_AMB_DET_sample.csv";
          "../data/sample/PR_201501_AMB_DET.csv";
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

    CsvDAOEntrada csv(
        caminhoCsv,
        *contrato
    );

    csv.lerPrimeiroRegistro();

    std::cout << '\n';

    std::cout
    << "\n===== PROCESSAMENTO COMPLETO DO CSV =====\n";

    csv.processarArquivo();

    std::cout << '\n';

    // =====================================================
    // TESTE JA EXISTENTE - LEITURA SQLITE
    // =====================================================

    SQLiteDAOEntrada sqlite(
        "../data/legatech.db"
    );

    DAOEntrada& dao = sqlite;

    dao.processarArquivo();

    return 0;
}
