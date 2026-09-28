#include "../include/IdentificadorContextoTiss.h"

#include <filesystem>
#include <sstream>
#include <vector>
#include <cctype>

std::optional<ContextoTiss>
IdentificadorContextoTiss::identificar(
    const std::string& caminhoArquivo
) const
{
    std::filesystem::path caminho(caminhoArquivo);

    // Retira diretorio e extensao.
    // Exemplo:
    // ../data/PR_202501_AMB_DET.csv
    // vira:
    // PR_202501_AMB_DET
    std::string nomeArquivo =
        caminho.stem().string();

    std::stringstream fluxo(nomeArquivo);
    std::vector<std::string> partes;
    std::string parte;

    while (std::getline(fluxo, parte, '_'))
    {
        partes.push_back(parte);
    }

    // Esperamos pelo menos:
    // UF_COMPETENCIA_TIPO
    //
    // Exemplo:
    // PR_202501_AMB_DET
    if (partes.size() < 3)
    {
        return std::nullopt;
    }

    std::string uf = partes[0];
    std::string competencia = partes[1];

    // O tipo pode possuir mais de uma parte:
    // AMB_DET
// Para o contrato inicial trabalhamos com arquivos
// no padrao:
//
// UF_AAAAMM_AMB_DET.csv
//
// Um complemento posterior, como "_sample",
// pertence ao ambiente de desenvolvimento e nao
// ao tipo TISS identificado.
    if (partes.size() < 4)
    {
        return std::nullopt;
    }

    std::string tipoArquivo =
        partes[2] + "_" + partes[3];
    // Validacao minima da UF.
    if (uf.size() != 2)
    {
        return std::nullopt;
    }

    // Competencia esperada no formato AAAAMM.
    if (competencia.size() != 6)
    {
        return std::nullopt;
    }

    for (char caractere : competencia)
    {
        if (!std::isdigit(
                static_cast<unsigned char>(caractere)
            ))
        {
            return std::nullopt;
        }
    }

    if (tipoArquivo.empty())
    {
        return std::nullopt;
    }

    return ContextoTiss(
        uf,
        competencia,
        tipoArquivo,
        caminhoArquivo
    );
}
