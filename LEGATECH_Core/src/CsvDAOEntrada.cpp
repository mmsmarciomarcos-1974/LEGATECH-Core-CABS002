#include "../include/CsvDAOEntrada.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

std::vector<std::string> separarCampos(const std::string& linha)
{
    std::vector<std::string> campos;
    std::stringstream ss(linha);
    std::string campo;

    while (std::getline(ss, campo, ';'))
    {
        // Remove aspas externas quando existirem
        if (campo.size() >= 2 &&
            campo.front() == '"' &&
            campo.back() == '"')
        {
            campo = campo.substr(1, campo.size() - 2);
        }

        campos.push_back(campo);
    }

    return campos;
}

CsvDAOEntrada::CsvDAOEntrada(const std::string& caminhoArquivo)
    : caminhoArquivo(caminhoArquivo)
{
}

void CsvDAOEntrada::lerPrimeiroRegistro()
{
    std::ifstream arquivo(caminhoArquivo);

    if (!arquivo.is_open())
    {
        std::cout << "ERRO: Nao foi possivel abrir o arquivo.\n";
        return;
    }

    std::string cabecalho;
    std::string linha;

    // Primeira leitura: cabecalho
    std::getline(arquivo, cabecalho);

    // Segunda leitura: primeiro registro
    if (std::getline(arquivo, linha))
    {
        std::cout << "Cabecalho:\n";
        std::cout << cabecalho << "\n\n";

        std::cout << "Primeiro registro:\n";
        std::cout << linha << "\n\n";

        std::vector<std::string> campos = separarCampos(linha);

        std::cout << "Quantidade de campos: "
                  << campos.size()
                  << '\n';

        if (campos.size() == 11)
        {
            std::cout << "ID Evento: "
                      << campos[0] << '\n';

            std::cout << "UF: "
                      << campos[1] << '\n';

            std::cout << "Data: "
                      << campos[2] << '\n';

            std::cout << "Procedimento: "
                      << campos[3] << '\n';
        }
        else
        {
            std::cout << "ERRO: estrutura divergente.\n";
        }
    }

    arquivo.close();
}
