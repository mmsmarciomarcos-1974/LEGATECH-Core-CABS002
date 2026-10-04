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
    bool tabelaTussSuportada(
        const std::string& tabela
    )
    {
        static const std::unordered_set<std::string>
            tabelas =
        {
            "19",
            "20",
            "22",
            "63"
        };

        return tabelas.contains(tabela);
    }


    bool tabelaPacote(
        const std::string& tabela
    )
    {
        return tabela == "90" ||
            tabela == "98";
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
    // REGRA 3 E 4
    // CD_TABELA_REFERENCIA + CD_PROCEDIMENTO
    // =================================================

    const std::string& procedimento =
        registro.getCdProcedimento();

    const std::string& tabelaReferencia =
        registro.getCdTabelaReferencia();

    const std::string& indPacote =
        registro.getIndPacote();

    const std::string& indTabelaPropria =
        registro.getIndTabelaPropria();


    // -------------------------------------------------
    // CD_TABELA_REFERENCIA
    // -------------------------------------------------

    if (tabelaReferencia.empty())
    {
        resultado.adicionarMotivo(
            "CD_TABELA_REFERENCIA: "
            "campo obrigatorio nao informado"
        );
    }
    else if (!possuiSomenteDigitos(
                tabelaReferencia
            ))
    {
        resultado.adicionarMotivo(
            "CD_TABELA_REFERENCIA: "
            "formato invalido"
        );
    }
    else
    {
        // =============================================
        // TABELA PROPRIA
        // =============================================
        //
        // Na massa real 2025:
        //
        // CD_TABELA_REFERENCIA = 00
        // IND_TABELA_PROPRIA   = 1
        //
        if (tabelaReferencia == "00")
        {
            if (indTabelaPropria != "1")
            {
                resultado.adicionarMotivo(
                    "CD_TABELA_REFERENCIA: "
                    "referencia 00 incompatível com "
                    "IND_TABELA_PROPRIA"
                );
            }
        }

        // =============================================
        // PACOTES DA OPERADORA
        // =============================================

        else if (tabelaPacote(
                    tabelaReferencia
                ))
        {
            if (indPacote != "1")
            {
                resultado.adicionarMotivo(
                    "CD_TABELA_REFERENCIA: "
                    "referencia de pacote incompatível "
                    "com IND_PACOTE"
                );
            }
        }

        // =============================================
        // TUSS
        // =============================================

        else if (!tabelaTussSuportada(
                    tabelaReferencia
                ))
        {
            resultado.adicionarMotivo(
                "CD_TABELA_REFERENCIA: "
                "codigo fora do dominio suportado "
                "para AMB_DET"
            );
        }
    }


    // -------------------------------------------------
    // CD_PROCEDIMENTO
    // -------------------------------------------------

    if (procedimento.empty())
    {
        resultado.adicionarMotivo(
            "CD_PROCEDIMENTO: "
            "campo obrigatorio nao informado"
        );
    }
    else
    {
        // Para tabelas TUSS conhecidas,
        // o codigo observado deve possuir
        // formato numerico.
        //
        // A existencia real do codigo na TUSS
        // sera validada na proxima etapa.

        if (tabelaTussSuportada(
                tabelaReferencia
            ) &&
            !possuiSomenteDigitos(
                procedimento
            ))
        {
            resultado.adicionarMotivo(
                "CD_PROCEDIMENTO: "
                "formato invalido para tabela TUSS"
            );
        }

        // Para tabela propria (00) e pacotes
        // 90/98 nao forçamos codigo numerico,
        // pois sao codigos definidos pela operadora.
    }
    // Consulta externa do PAR, sem lista de procedimentos no codigo.
    // Referencias proprias/pacotes seguem as regras acima.
    if (tabelaTussSuportada(tabelaReferencia) &&
        possuiSomenteDigitos(procedimento))
    {
        if (indTabelaPropria == "1")
        {
            resultado.adicionarAlerta(
                "PAR_TUSS: tabela padronizada com IND_TABELA_PROPRIA=1; "
                "existencia TUSS nao verificada");
        }
        else if (!catalogo || !catalogo->contemTabela(tabelaReferencia))
        {
            resultado.adicionarAlerta(
                "CATALOGO_NAO_CARREGADO: tabela " + tabelaReferencia);
        }
        else
        {
            const std::string referencia =
                " [TUSS " + catalogo->getVersao() + ", tabela " +
                tabelaReferencia + ", procedimento " + procedimento + "]";
            const TermoTuss* termo =
                catalogo->consultar(tabelaReferencia, procedimento);
            if (!termo)
            {
                // Snapshot nao e prova de cobertura de todo o historico.
                resultado.adicionarAlerta(
                    "CODIGO_NAO_LOCALIZADO_NO_CATALOGO" + referencia);
            }
            else if (validarFormatoAnoMes(data) && data.substr(0, 4) != "0000")
            {
                using namespace std::chrono;
                const year_month mes{
                    year{std::stoi(data.substr(0, 4))},
                    month{static_cast<unsigned>(std::stoi(data.substr(5, 2)))}};
                const sys_days primeiro{mes / day{1}};
                const sys_days ultimo{mes / last};
                if (termo->inicioVigencia > ultimo ||
                    (termo->fimVigencia && *termo->fimVigencia < primeiro))
                {
                    resultado.adicionarMotivo(
                        "PROCEDIMENTO_FORA_DA_VIGENCIA: mes " + data + referencia);
                }
                else if (termo->inicioVigencia > primeiro ||
                         (termo->fimVigencia && *termo->fimVigencia <= ultimo))
                {
                    // Mes sem dia nao permite decidir em uma fronteira.
                    // Inclui fim no ultimo dia: convencao de inclusao nao presumida.
                    resultado.adicionarAlerta(
                        "VIGENCIA_PARCIAL_NO_MES: dia do atendimento necessario" +
                        referencia);
                }
            }
            else
            {
                resultado.adicionarAlerta(
                    "VIGENCIA_NAO_VERIFICADA: periodo indisponivel" + referencia);
            }
        }
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
