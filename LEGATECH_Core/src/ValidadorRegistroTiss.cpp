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
    bool possuiSomenteDigitos(
        const std::string& valor
    )
    {
        if (valor.empty())
        {
            return false;
        }

        for (char caractere : valor)
        {
            if (!std::isdigit(
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
    bool possuiFormatoDecimalValido(
    const std::string& valor
)
{
    if (valor.empty())
    {
        return false;
    }

    bool separadorEncontrado = false;
    bool digitoEncontrado = false;

    for (char caractere : valor)
    {
        if (std::isdigit(
                static_cast<unsigned char>(caractere)
            ))
        {
            digitoEncontrado = true;
            continue;
        }

        if ((caractere == ',' || caractere == '.') &&
            !separadorEncontrado)
        {
            separadorEncontrado = true;
            continue;
        }

        return false;
    }

    return digitoEncontrado;
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


    const std::string& procedimento =
        registro.getCdProcedimento();

    if (procedimento.empty())
    {
        resultado.adicionarMotivo(
            "CD_PROCEDIMENTO: "
            "campo obrigatorio nao informado"
        );
    }
    else if (!possuiSomenteDigitos(procedimento))
    {
        resultado.adicionarMotivo(
            "CD_PROCEDIMENTO: formato invalido, "
            "esperado codigo numerico"
        );
    }


    // =================================================
    // REGRA 4 - CD_TABELA_REFERENCIA
    // =================================================

    const std::string& tabelaReferencia =
        registro.getCdTabelaReferencia();

    if (tabelaReferencia.empty())
    {
        resultado.adicionarMotivo(
            "CD_TABELA_REFERENCIA: "
            "campo obrigatorio nao informado"
        );
    }
    else if (!possuiSomenteDigitos(tabelaReferencia))
    {
        resultado.adicionarMotivo(
            "CD_TABELA_REFERENCIA: formato invalido, "
            "esperado codigo numerico"
        );
    }
    // =================================================
    // REGRA 5 - QT_ITEM_EVENTO_INFORMADO
    // =================================================

    const std::string& quantidade =
        registro.getQtItemEventoInformado();

    if (!quantidade.empty() &&
        !possuiFormatoDecimalValido(quantidade))
    {
        resultado.adicionarMotivo(
            "QT_ITEM_EVENTO_INFORMADO: "
            "formato numerico invalido"
        );
    }

    // =================================================
    // REGRA 6 - VL_ITEM_EVENTO_INFORMADO
    // =================================================

    const std::string& valorItemInformado =
        registro.getVlItemEventoInformado();

    if (!valorItemInformado.empty() &&
        !possuiFormatoDecimalValido(valorItemInformado))
    {
        resultado.adicionarMotivo(
            "VL_ITEM_EVENTO_INFORMADO: "
            "formato numerico invalido"
        );
    }


    // =================================================
    // REGRA 7 - VL_ITEM_PAGO_FORNECEDOR
    // =================================================

    const std::string& valorPagoFornecedor =
        registro.getVlItemPagoFornecedor();

    if (!valorPagoFornecedor.empty() &&
        !possuiFormatoDecimalValido(valorPagoFornecedor))
    {
        resultado.adicionarMotivo(
            "VL_ITEM_PAGO_FORNECEDOR: "
            "formato numerico invalido"
        );
    }

    // =================================================
    // REGRA 8 - IND_PACOTE
    // =================================================

    const std::string& indPacote =
        registro.getIndPacote();

    if (!indPacote.empty() &&
        indPacote != "0" &&
        indPacote != "1")
    {
        resultado.adicionarMotivo(
            "IND_PACOTE: valor invalido, esperado 0 ou 1"
        );
    }


    // =================================================
    // REGRA 9 - IND_TABELA_PROPRIA
    // =================================================

    const std::string& indTabelaPropria =
        registro.getIndTabelaPropria();

    if (!indTabelaPropria.empty() &&
        indTabelaPropria != "0" &&
        indTabelaPropria != "1")
    {
        resultado.adicionarMotivo(
            "IND_TABELA_PROPRIA: valor invalido, esperado 0 ou 1"
        );
    }

    // =================================================
    // REGRA 10 - ID_EVENTO_ATENCAO_SAUDE
    // =================================================

    const std::string& idEvento =
        registro.getIdEventoAtencaoSaude();

    if (idEvento.empty())
    {
        resultado.adicionarMotivo(
            "ID_EVENTO_ATENCAO_SAUDE: "
            "campo obrigatorio nao informado"
        );
    }
    else if (!possuiSomenteDigitos(idEvento))
    {
        resultado.adicionarMotivo(
            "ID_EVENTO_ATENCAO_SAUDE: "
            "formato invalido, esperado codigo numerico"
        );
    }// =================================================
    // REGRA 11 - UNIDADE_MEDIDA
    // =================================================
    //
    // Nesta etapa nao aplicamos obrigatoriedade,
    // formato ou dominio.
    //
    // O campo aparece vazio em registros reais da massa.
    // A regra sera incorporada posteriormente com base
    // no Dicionario TISS correspondente ao periodo.
    //

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
