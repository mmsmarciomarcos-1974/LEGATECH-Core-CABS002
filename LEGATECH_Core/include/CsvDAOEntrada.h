#ifndef CSVDAOENTRADA_H
#define CSVDAOENTRADA_H

#include <string>
#include <vector>

#include "DAOEntrada.h"
#include "ProcessadorTiss.h"
#include "ContratoTiss.h"

class CsvDAOEntrada : public DAOEntrada
{
private:
    std::string caminhoArquivo;

    const ContratoTiss& contrato;

    // Competencia identificada no nome/contexto do arquivo.
    //
    // Exemplo:
    // 202501
    //
    // Sera utilizada para validacoes contextuais,
    // como DT_REALIZACAO x competencia do arquivo.
    std::string competenciaArquivo;
    const CatalogoTuss* catalogo = nullptr;

public:

    // =================================================
    // CONSTRUTOR COMPATIVEL COM O FLUXO ANTIGO
    // =================================================
    //
    // Mantemos este construtor para nao quebrar
    // chamadas que ainda nao fornecem competencia.
    //
    CsvDAOEntrada(
        const std::string& caminhoArquivo,
        const ContratoTiss& contrato
    );


    // =================================================
    // CONSTRUTOR COM CONTEXTO DE COMPETENCIA
    // =================================================
    //
    // Este sera utilizado pelo fluxo principal TISS.
    //
    CsvDAOEntrada(
        const std::string& caminhoArquivo,
        const ContratoTiss& contrato,
        const std::string& competenciaArquivo,
        const CatalogoTuss* catalogo = nullptr
    );


    void lerPrimeiroRegistro();


    void processarArquivo() override;


    void processarArquivo(
        ProcessadorTiss& processador
    );


    bool validarCabecalho(
        const std::string& cabecalho,
        const ContratoTiss& contrato
    );


    std::vector<std::string> normalizarCampos(
        const std::vector<std::string>& campos,
        const ContratoTiss& contrato
    ) const;
};

#endif
