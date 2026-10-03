#include "../include/ValidadorRegistroTiss.h"

#include <cctype>
#include <string>


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
    //
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

}


// =====================================================
// VALIDACAO DO REGISTRO TISS
// =====================================================

ResultadoValidacao
ValidadorRegistroTiss::validar(
    const RegistroTissBruto& registro
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
    else
    {
        // Regra tecnica inicial da BETA:
        // UF com duas letras minusculas ainda pode ser
        // reconhecida, mas gera alerta de padronizacao.
        //
        // Ainda nao estamos afirmando que esta seja
        // uma regra oficial de dominio TISS.

        if (!std::isupper(
                static_cast<unsigned char>(uf[0])
            ) ||
            !std::isupper(
                static_cast<unsigned char>(uf[1])
            ))
        {
            resultado.adicionarAlerta(
                "UF_PRESTADOR: valor fora do padrao "
                "de letras maiusculas"
            );
        }
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
    // O estado foi atualizado automaticamente
    // conforme motivos e alertas foram adicionados.
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
