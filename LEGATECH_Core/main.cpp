#include <iostream>
#include <string>

#include "include/IdentificadorContextoTiss.h"
#include "include/ContratoAmbDetV1.h"
#include "include/SQLiteDAOEntrada.h"

int main()
{
    std::cout << "LEGATECH Core - CABS002\n\n";

    // =====================================================
    // IDENTIFICACAO DO CONTEXTO TISS
    // =====================================================

    const std::string caminhoCsv =
        "../data/sample/PR_202501_AMB_DET_sample.csv";

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


    // =====================================================
    // IDENTIFICACAO DO CONTRATO SUPORTADO
    // =====================================================

    ContratoAmbDetV1 contratoAmbDet;

    ContratoTiss& contrato =
        contratoAmbDet;

    if (!contrato.suporta(*contexto))
    {
        std::cout
            << "Contexto identificado, "
            << "mas o tipo de arquivo ainda "
            << "nao possui contrato suportado.\n";

        return 1;
    }

    std::cout
        << "Contrato aplicado: "
        << contrato.getNome()
        << "\n\n";


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
