#include "../include/RegistroTissBruto.h"
#include "../include/CsvDAOEntrada.h"
#include "../include/ResultadoValidacao.h"

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
bool CsvDAOEntrada::validarCabecalho(const std::string& cabecalho)
{
    std::string cabecalhoLimpo = cabecalho;

    // Remove o BOM UTF-8, caso exista no inicio do arquivo
    if (cabecalhoLimpo.size() >= 3 &&
        static_cast<unsigned char>(cabecalhoLimpo[0]) == 0xEF &&
        static_cast<unsigned char>(cabecalhoLimpo[1]) == 0xBB &&
        static_cast<unsigned char>(cabecalhoLimpo[2]) == 0xBF)
    {
        cabecalhoLimpo.erase(0, 3);
    }

    const std::vector<std::string> colunasEsperadas =
    {
        "ID_EVENTO_ATENCAO_SAUDE",
        "UF_PRESTADOR",
        "DT_REALIZACAO",
        "CD_PROCEDIMENTO",
        "CD_TABELA_REFERENCIA",
        "QT_ITEM_EVENTO_INFORMADO",
        "VL_ITEM_EVENTO_INFORMADO",
        "VL_ITEM_PAGO_FORNECEDOR",
        "UNIDADE_MEDIDA",
        "IND_PACOTE",
        "IND_TABELA_PROPRIA"
    };

    std::vector<std::string> colunasEncontradas =
        separarCampos(cabecalhoLimpo);

    if (colunasEncontradas.size() != colunasEsperadas.size())
    {
        return false;
    }

    for (std::size_t i = 0; i < colunasEsperadas.size(); ++i)
    {
        if (colunasEncontradas[i] != colunasEsperadas[i])
        {
            return false;
        }
    }

    return true;
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
if (!validarCabecalho(cabecalho))
{
    std::cout << "ERRO: cabecalho invalido ou estrutura divergente.\n";
    arquivo.close();
    return;
}

std::cout << "Cabecalho validado com sucesso.\n\n";

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
            RegistroTissBruto registro(
            campos[0],
            campos[1],
            campos[2],
            campos[3],
            campos[4],
            campos[5],
            campos[6],
            campos[7],
            campos[8],
            campos[9],
            campos[10]
        );
            std::cout << "\nObjeto RegistroTissBruto criado com sucesso.\n";

            std::cout << "ID Evento: "
                    << registro.getIdEventoAtencaoSaude()
                    << '\n';

            std::cout << "UF: "
                    << registro.getUfPrestador()
                    << '\n';

            std::cout << "Data: "
                    << registro.getDtRealizacao()
                    << '\n';
        ResultadoValidacao resultado(EstadoValidacao::VALIDO);

        if (resultado.getEstado() == EstadoValidacao::VALIDO)
        {
            std::cout << "Resultado inicial: VALIDO\n";
        }
        }
        else
        {
            std::cout << "ERRO: estrutura divergente.\n";
        }
    }

    arquivo.close();
}
