#include "../include/RegistroTissBruto.h"
#include "../include/CsvDAOEntrada.h"
#include "../include/ResultadoValidacao.h"
#include "../include/ValidadorRegistroTiss.h"
#include "../include/ProcessadorTiss.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <chrono>
#include <stdexcept>


// =====================================================
// FUNCAO AUXILIAR - SEPARACAO DOS CAMPOS CSV
// =====================================================

std::vector<std::string> separarCampos(
    const std::string& linha
)
{
    std::vector<std::string> campos;
    std::stringstream ss(linha);
    std::string campo;

    while (std::getline(ss, campo, ';'))
    {
        // Remove aspas externas quando existirem.
        if (campo.size() >= 2 &&
            campo.front() == '"' &&
            campo.back() == '"')
        {
            campo = campo.substr(
                1,
                campo.size() - 2
            );
        }

        campos.push_back(campo);
    }

    // getline nao devolve o campo vazio depois do ultimo separador.
    if (!linha.empty() && linha.back() == ';') campos.emplace_back();
    return campos;
}


// =====================================================
// VALIDACAO DO CABECALHO CONTRA O CONTRATO TISS
// =====================================================

bool CsvDAOEntrada::validarCabecalho(
    const std::string& cabecalho,
    const ContratoTiss& contrato
)
{
    std::string cabecalhoLimpo = cabecalho;
    if (!cabecalhoLimpo.empty() && cabecalhoLimpo.back() == '\r') cabecalhoLimpo.pop_back();

    // Remove BOM UTF-8, caso exista no inicio do arquivo.
    if (cabecalhoLimpo.size() >= 3 &&
        static_cast<unsigned char>(
            cabecalhoLimpo[0]
        ) == 0xEF &&
        static_cast<unsigned char>(
            cabecalhoLimpo[1]
        ) == 0xBB &&
        static_cast<unsigned char>(
            cabecalhoLimpo[2]
        ) == 0xBF)
    {
        cabecalhoLimpo.erase(0, 3);
    }

    const std::vector<std::string> colunasEsperadas =
        contrato.getCamposEsperados();

    const std::vector<std::string> colunasEncontradas =
        separarCampos(cabecalhoLimpo);

    if (colunasEncontradas.size() !=
        colunasEsperadas.size())
    {
        return false;
    }

    for (std::size_t i = 0;
         i < colunasEsperadas.size();
         ++i)
    {
        if (colunasEncontradas[i] !=
            colunasEsperadas[i])
        {
            return false;
        }
    }

    return true;
}


// =====================================================
// NORMALIZACAO DO LAYOUT FISICO PARA O MODELO INTERNO
// =====================================================

std::vector<std::string>
CsvDAOEntrada::normalizarCampos(
    const std::vector<std::string>& campos,
    const ContratoTiss& contrato
) const
{
    const std::vector<std::string> camposEsperados =
        contrato.getCamposEsperados();

    // =================================================
    // A PARTIR DE 2023
    // =================================================
    //
    // Layout fisico com 11 campos.
    // Ja possui UNIDADE_MEDIDA.
    //
    if (camposEsperados.size() == 11 &&
        campos.size() == 11)
    {
        return campos;
    }

    // =================================================
    // ATE 2022
    // =================================================
    //
    // Layout fisico com 10 campos.
    // Nao possui UNIDADE_MEDIDA.
    //
    // O LEGATECH converte para a representacao
    // interna comum de 11 campos.
    //
    if (camposEsperados.size() == 10 &&
        campos.size() == 10)
    {
        std::vector<std::string> normalizados;

        normalizados.reserve(11);

        // Campos 0 a 7 permanecem iguais.
        for (std::size_t i = 0; i < 8; ++i)
        {
            normalizados.push_back(
                campos[i]
            );
        }

        // Campo inexistente no layout historico.
        // Posicao interna: UNIDADE_MEDIDA.
        normalizados.push_back("");

        // IND_PACOTE
        normalizados.push_back(
            campos[8]
        );

        // IND_TABELA_PROPRIA
        normalizados.push_back(
            campos[9]
        );

        return normalizados;
    }

    // Estrutura fisica incompatível com o contrato.
    return {};
}


// =====================================================
// CONSTRUTOR
// =====================================================

CsvDAOEntrada::CsvDAOEntrada(
    const std::string& caminhoArquivo,
    const ContratoTiss& contrato
)
    : caminhoArquivo(caminhoArquivo),
      contrato(contrato),
      competenciaArquivo("")
{
}
CsvDAOEntrada::CsvDAOEntrada(
    const std::string& caminhoArquivo,
    const ContratoTiss& contrato,
    const std::string& competenciaArquivo,
    const CatalogoTuss* catalogo,
    AuditoriaValidacao* auditoria
)
    : caminhoArquivo(caminhoArquivo),
      contrato(contrato),
      competenciaArquivo(competenciaArquivo),
      catalogo(catalogo),
      auditoria(auditoria)
{
}


// =====================================================
// LEITURA DE UM REGISTRO PARA TESTE
// =====================================================

void CsvDAOEntrada::lerPrimeiroRegistro()
{
    std::ifstream arquivo(
        caminhoArquivo
    );

    if (!arquivo.is_open())
    {
        std::cout
            << "ERRO: Nao foi possivel abrir o arquivo.\n";

        return;
    }

    std::string cabecalho;
    std::string linha;


    // =================================================
    // 1. CABECALHO
    // =================================================

    if (!std::getline(
            arquivo,
            cabecalho
        ))
    {
        std::cout
            << "ERRO: arquivo sem cabecalho.\n";

        arquivo.close();
        return;
    }

    if (!validarCabecalho(
            cabecalho,
            contrato
        ))
    {
        std::cout
            << "ERRO: cabecalho invalido "
            << "ou estrutura divergente.\n";

        arquivo.close();
        return;
    }

    std::cout
        << "Cabecalho validado com sucesso.\n\n";


    // =================================================
    // 2. PRIMEIRO REGISTRO
    // =================================================

    if (!std::getline(
            arquivo,
            linha
        ))
    {
        std::cout
            << "Arquivo sem registros de dados.\n";

        arquivo.close();
        return;
    }

    std::cout
        << "Cabecalho:\n"
        << cabecalho
        << "\n\n";

    std::cout
        << "Primeiro registro:\n"
        << linha
        << "\n\n";


    // =================================================
    // 3. SEPARACAO E NORMALIZACAO
    // =================================================

    const std::vector<std::string> camposRecebidos =
        separarCampos(linha);

    const std::vector<std::string> camposNormalizados =
        normalizarCampos(
            camposRecebidos,
            contrato
        );

    std::cout
        << "Campos fisicos encontrados: "
        << camposRecebidos.size()
        << '\n';

    if (camposNormalizados.empty())
    {
        std::cout
            << "ERRO: estrutura do registro "
            << "divergente do contrato.\n";

        arquivo.close();
        return;
    }

    std::cout
        << "Campos na representacao interna: "
        << camposNormalizados.size()
        << "\n\n";


    // =================================================
    // 4. CRIACAO DO OBJETO INTERNO
    // =================================================

    RegistroTissBruto registro(
        camposNormalizados[0],
        camposNormalizados[1],
        camposNormalizados[2],
        camposNormalizados[3],
        camposNormalizados[4],
        camposNormalizados[5],
        camposNormalizados[6],
        camposNormalizados[7],
        camposNormalizados[8],
        camposNormalizados[9],
        camposNormalizados[10]
    );

    std::cout
        << "Objeto RegistroTissBruto "
        << "criado com sucesso.\n";

    std::cout
        << "ID Evento: "
        << registro.getIdEventoAtencaoSaude()
        << '\n';

    std::cout
        << "UF: "
        << registro.getUfPrestador()
        << '\n';

    std::cout
        << "Data: "
        << registro.getDtRealizacao()
        << '\n';


    // =================================================
    // 5. VALIDACAO
    // =================================================

    ValidadorRegistroTiss validador(catalogo);

    ResultadoValidacao resultado =
        validador.validar(
            registro,
            competenciaArquivo
        );


    // =================================================
    // 6. RESULTADO DO PRIMEIRO REGISTRO
    // =================================================

    if (resultado.getEstado() ==
        EstadoValidacao::VALIDO)
    {
        std::cout
            << "Resultado da validacao: VALIDO\n";
    }
    else if (resultado.getEstado() ==
             EstadoValidacao::VALIDO_COM_ALERTA)
    {
        std::cout
            << "Resultado da validacao: "
            << "VALIDO COM ALERTA\n";

        for (const std::string& alerta :
             resultado.getAlertas())
        {
            std::cout
                << "  ALERTA: "
                << alerta
                << '\n';
        }
    }
    else
    {
        std::cout
            << "Resultado da validacao: REJEITADO\n";

        for (const std::string& motivo :
             resultado.getMotivos())
        {
            std::cout
                << "  MOTIVO: "
                << motivo
                << '\n';
        }

        // Um registro rejeitado também pode possuir
        // alertas acumulados.
        for (const std::string& alerta :
             resultado.getAlertas())
        {
            std::cout
                << "  ALERTA: "
                << alerta
                << '\n';
        }
    }

    arquivo.close();
}


// =====================================================
// FLUXO UNICO: VALIDACAO, AUDITORIA OPCIONAL E RESUMO
// =====================================================

void CsvDAOEntrada::processarArquivo()
{
    processarArquivoImpl(nullptr);
}

void CsvDAOEntrada::processarArquivo(ProcessadorTiss& processador)
{
    processarArquivoImpl(&processador);
}

void CsvDAOEntrada::processarArquivoImpl(ProcessadorTiss* processador)
{
    constexpr std::size_t INTERVALO_PROGRESSO = 10000;
    const auto inicio = std::chrono::steady_clock::now();
    std::ifstream arquivo(caminhoArquivo, std::ios::binary);
    if (!arquivo.is_open())
        throw std::runtime_error("Nao foi possivel abrir o CSV: " + caminhoArquivo);

    std::string cabecalho, linha;
    if (!std::getline(arquivo, cabecalho))
        throw std::runtime_error("CSV sem cabecalho: " + caminhoArquivo);
    if (!validarCabecalho(cabecalho, contrato))
        throw std::runtime_error("Cabecalho CSV invalido ou estrutura divergente do contrato");
    std::cout << "Cabecalho validado com sucesso.\n\n";

    std::size_t total = 0, validos = 0, validosComAlerta = 0;
    std::size_t rejeitados = 0, estruturaDivergente = 0, linhaFisica = 1;
    std::map<std::string, std::size_t> contagemAlertas;
    std::map<std::string, std::size_t> contagemMotivos;
    std::map<std::string, std::string> amostrasAlertas;
    std::map<std::string, std::string> amostrasMotivos;
    ValidadorRegistroTiss validador(catalogo);
    const auto exibirProgresso = [&]()
    {
        const auto segundos = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - inicio).count();
        std::cout << "Progresso: " << total << " registros | validos: " << validos
                  << " | com alerta: " << validosComAlerta
                  << " | rejeitados: " << rejeitados
                  << " | tempo: " << segundos << " s\n";
    };
    std::cout << "Processamento iniciado. Progresso a cada "
              << INTERVALO_PROGRESSO << " registros.\n";
    try
    {
        if (auditoria)
            auditoria->iniciar({caminhoArquivo, competenciaArquivo, contrato.getNome(),
                catalogo ? catalogo->getVersao() : "",
                catalogo ? catalogo->getTabela() : "", cabecalho});
        while (std::getline(arquivo, linha))
        {
            ++linhaFisica;
            // Mantem a linha original na auditoria; CR de CRLF so sai da copia de leitura.
            std::string linhaLeitura = linha;
            if (!linhaLeitura.empty() && linhaLeitura.back() == '\r') linhaLeitura.pop_back();
            if (linhaLeitura.empty()) continue;
            ++total;
            const auto campos = normalizarCampos(separarCampos(linhaLeitura), contrato);
            if (campos.empty())
            {
                ++rejeitados;
                ++estruturaDivergente;
                ResultadoValidacao resultado(EstadoValidacao::REJEITADO);
                resultado.adicionarMotivo("ESTRUTURA_DIVERGENTE_DO_CONTRATO");
                if (auditoria) auditoria->gravar(linhaFisica, linha, nullptr, resultado);
                const std::string motivo = "ESTRUTURA_DIVERGENTE_DO_CONTRATO";
                ++contagemMotivos[motivo];
                if (amostrasMotivos.find(motivo) == amostrasMotivos.end())
                    amostrasMotivos[motivo] = "linha fisica " + std::to_string(linhaFisica);
                if (total % INTERVALO_PROGRESSO == 0) exibirProgresso();
                continue;
            }
            RegistroTissBruto registro(campos[0], campos[1], campos[2], campos[3], campos[4],
                                      campos[5], campos[6], campos[7], campos[8], campos[9], campos[10]);
            const auto resultado = processador
                ? processador->processar(registro, competenciaArquivo)
                : validador.validar(registro, competenciaArquivo);
            if (auditoria) auditoria->gravar(linhaFisica, linha, &registro, resultado);
            if (resultado.getEstado() == EstadoValidacao::VALIDO) ++validos;
            else
            {
                const bool alerta = resultado.getEstado() == EstadoValidacao::VALIDO_COM_ALERTA;
                if (alerta) ++validosComAlerta;
                else ++rejeitados;
                for (const auto& motivo : resultado.getMotivos())
                {
                    ++contagemMotivos[motivo];
                    if (amostrasMotivos.find(motivo) == amostrasMotivos.end())
                        amostrasMotivos[motivo] = "linha fisica " + std::to_string(linhaFisica)
                            + ", registro ID " + registro.getIdEventoAtencaoSaude();
                }
                for (const auto& aviso : resultado.getAlertas())
                {
                    ++contagemAlertas[aviso];
                    if (amostrasAlertas.find(aviso) == amostrasAlertas.end())
                        amostrasAlertas[aviso] = "linha fisica " + std::to_string(linhaFisica)
                            + ", registro ID " + registro.getIdEventoAtencaoSaude();
                }
            }
            if (total % INTERVALO_PROGRESSO == 0) exibirProgresso();
        }
        if (arquivo.bad() || (!arquivo.eof() && arquivo.fail()))
            throw std::runtime_error("Falha durante leitura do CSV: " + caminhoArquivo);
        if (auditoria) auditoria->finalizar();
    }
    catch (...)
    {
        if (auditoria) auditoria->cancelar();
        throw;
    }
    const std::chrono::duration<double> duracao = std::chrono::steady_clock::now() - inicio;
    std::cout << "\n===== RESUMO DO PROCESSAMENTO =====\n"
              << "Total processado:       " << total << '\n'
              << "Validos:                " << validos << '\n'
              << "Validos com alerta:     " << validosComAlerta << '\n'
              << "Rejeitados:             " << rejeitados << '\n'
              << "Estrutura divergente:   " << estruturaDivergente << '\n'
              << "Tempo de processamento: " << duracao.count() << " segundos\n";
    if (!contagemAlertas.empty())
    {
        std::cout << "\nAlertas por tipo (ocorrencias):\n";
        for (const auto& item : contagemAlertas)
            std::cout << "  " << item.first << ": " << item.second
                      << " | exemplo: " << amostrasAlertas[item.first] << '\n';
    }
    if (!contagemMotivos.empty())
    {
        std::cout << "\nMotivos de rejeicao por tipo (ocorrencias):\n";
        for (const auto& item : contagemMotivos)
            std::cout << "  " << item.first << ": " << item.second
                      << " | exemplo: " << amostrasMotivos[item.first] << '\n';
    }
}
