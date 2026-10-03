#include "../include/ValidadorRegistroTiss.h"

#include <cctype>
#include <string>
#include <unordered_set>


// =====================================================
// FUNCOES AUXILIARES
// =====================================================

namespace
{

bool validarFormatoAnoMes(
    const std::string& valor
)
{
    // Formato esperado:
    // AAAA-MM
    //
    // Exemplo:
    // 2025-01

    if (valor.size() != 7)
    {
        return false;
    }

    if (valor[4] != '-')
    {
        return false;
    }

    for (std::size_t i = 0;
         i < valor.size();
         ++i)
    {
        if (i == 4)
        {
            continue;
        }

        if (!std::isdigit(
                static_cast<unsigned char>(
                    valor[i]
                )
            ))
        {
            return false;
        }
    }

    const int mes =
        std::stoi(
            valor.substr(5, 2)
        );

    if (mes < 1 || mes > 12)
    {
        return false;
    }

    return true;
}


bool possuiSomenteLetras(
    const std::string& valor
)
{
    for (char caractere : valor)
    {
        if (!std::isalpha(
                static_cast<unsigned char>(
                    caractere
                )
            ))
        {
            return false;
        }
    }

    return true;
}


std::string paraMaiusculo(
    const std::string& valor
)
{
    std::string resultado = valor;

    for (char& caractere : resultado)
    {
        caractere = static_cast<char>(
            std::toupper(
                static_cast<unsigned char>(
                    caractere
                )
            )
        );
    }

    return resultado;
}


bool ufPertenceAoDominio(
    const std::string& uf
)
{
    static const std::unordered_set<std::string>
        ufsValidas =
    {
        "AC", "AL", "AP", "AM", "BA", "CE", "DF",
        "ES", "GO", "MA", "MT", "MS", "MG", "PA",
        "PB", "PR", "PE", "PI", "RJ", "RN", "RS",
        "RO", "RR", "SC", "SP", "SE", "TO"
    };

    return ufsValidas.contains(
        paraMaiusculo(uf)
    );
}


std::string converterCompetenciaParaAnoMes(
    const std::string& competencia
)
{
    // Formato esperado:
    // AAAAMM
    //
    // Exemplo:
    // 202501 -> 2025-01

    if (competencia.size() != 6)
    {
        return "";
    }

    for (char caractere : competencia)
    {
        if (!std::isdigit(
                static_cast<unsigned char>(
                    caractere
                )
            ))
        {
            return "";
        }
    }

    return competencia.substr(0, 4)
         + "-"
         + competencia.substr(4, 2);
}

}


// =====================================================
// VALIDACAO SEM CONTEXTO DE COMPETENCIA
// =====================================================
//
// Mantida para preservar compatibilidade com os
// pontos do sistema que ainda chamam:
//
//     validar(registro)
//
// Exemplos:
// SQLiteDAOEntrada
// ProcessadorTiss
//
// Internamente, delega para a versao mais completa.
// =====================================================

ResultadoValidacao
ValidadorRegistroTiss::validar(
    const RegistroTissBruto& registro
) const
{
    return validar(
        registro,
        ""
    );
}


// =====================================================
// VALIDACAO COM CONTEXTO DE COMPETENCIA
// =====================================================

ResultadoValidacao
ValidadorRegistroTiss::validar(
    const RegistroTissBruto& registro,
    const std::string& competenciaArquivo
) const
{
    ResultadoValidacao resultado(
        EstadoValidacao::VALIDO
    );


    // =================================================
    // REGRA 1 - UF_PRESTADOR
    // =================================================

    const std::string& uf =
        registro.getUfPrestador();

    if (uf.empty())
    {
        resultado.adicionarMotivo(
            "UF_PRESTADOR: campo obrigatorio nao informado"
        );
    }
    else if (uf.size() != 2 ||
             !possuiSomenteLetras(uf))
    {
        resultado.adicionarMotivo(
            "UF_PRESTADOR: formato invalido"
        );
    }
    else if (!ufPertenceAoDominio(uf))
    {
        resultado.adicionarMotivo(
            "UF_PRESTADOR: valor fora do dominio "
            "de UFs brasileiras"
        );
    }
    else if (uf != paraMaiusculo(uf))
    {
        resultado.adicionarAlerta(
            "UF_PRESTADOR: valor reconhecido, "
            "mas fora do padrao de letras maiusculas"
        );
    }


    // =================================================
    // REGRA 2 - DT_REALIZACAO
    // =================================================

    const std::string& data =
        registro.getDtRealizacao();

    if (data.empty())
    {
        resultado.adicionarMotivo(
            "DT_REALIZACAO: campo obrigatorio nao informado"
        );
    }
    else if (!validarFormatoAnoMes(data))
    {
        resultado.adicionarMotivo(
            "DT_REALIZACAO: formato invalido, "
            "esperado AAAA-MM"
        );
    }
    else if (!competenciaArquivo.empty())
    {
        const std::string competenciaEsperada =
            converterCompetenciaParaAnoMes(
                competenciaArquivo
            );

        if (!competenciaEsperada.empty() &&
            data != competenciaEsperada)
        {
            resultado.adicionarAlerta(
                "DT_REALIZACAO: valor "
                + data
                + " difere da competencia do arquivo "
                + competenciaEsperada
            );
        }
    }


    // =================================================
    // REGRA 3 - CD_PROCEDIMENTO
    // =================================================

    if (registro.getCdProcedimento().empty())
    {
        resultado.adicionarMotivo(
            "CD_PROCEDIMENTO: "
            "campo obrigatorio nao informado"
        );
    }


    // =================================================
    // REGRA 4 - CD_TABELA_REFERENCIA
    // =================================================

    if (registro.getCdTabelaReferencia().empty())
    {
        resultado.adicionarMotivo(
            "CD_TABELA_REFERENCIA: "
            "campo obrigatorio nao informado"
        );
    }


    // =================================================
    // RESULTADO FINAL
    // =================================================
    //
    // Sem ocorrencias:
    //     VALIDO
    //
    // Somente alerta:
    //     VALIDO_COM_ALERTA
    //
    // Um ou mais motivos:
    //     REJEITADO
    //

    return resultado;
}
