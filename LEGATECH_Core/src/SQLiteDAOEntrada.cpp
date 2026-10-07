#include "../include/SQLiteDAOEntrada.h"
#include "../include/sqlite3.h"
#include "../include/RegistroTissBruto.h"
#include "../include/ValidadorRegistroTiss.h"
#include "../include/ResumoProcessamento.h"

#include "../include/CsvSaidaValidacao.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

SQLiteDAOEntrada::SQLiteDAOEntrada(const std::string& caminhoBanco,
                                   const CatalogoTuss* catalogo,
                                   const std::string& competenciaArquivo,
                                   AuditoriaValidacao* auditoria)
    : caminhoBanco(caminhoBanco), catalogo(catalogo), competenciaArquivo(competenciaArquivo), auditoria(auditoria)
{
}

void SQLiteDAOEntrada::processarArquivo()
{
    const auto inicio = std::chrono::steady_clock::now();
    // Somente leitura: banco ausente deve falhar, nunca criar uma entrada vazia.
    sqlite3* bancoBruto = nullptr;
    const int abertura = sqlite3_open_v2(caminhoBanco.c_str(), &bancoBruto, SQLITE_OPEN_READONLY, nullptr);
    const auto fecharBanco = [](sqlite3* p) { if (p) sqlite3_close(p); };
    std::unique_ptr<sqlite3, decltype(fecharBanco)> banco(bancoBruto, fecharBanco);
    if (abertura != SQLITE_OK)
        throw std::runtime_error("Erro ao abrir entrada SQLite: " +
                                 std::string(banco ? sqlite3_errmsg(banco.get()) : "sem conexao"));

    // Representacao relacional comum de onze campos, ja normalizada.
    const char* sql =
        "SELECT id_evento_atencao_saude,uf_prestador,dt_realizacao,cd_procedimento,"
        "cd_tabela_referencia,qt_item_evento_informado,vl_item_evento_informado,"
        "vl_item_pago_fornecedor,unidade_medida,ind_pacote,ind_tabela_propria "
        "FROM registro_tiss;";
    sqlite3_stmt* consultaBruta = nullptr;
    const int preparo = sqlite3_prepare_v2(banco.get(), sql, -1, &consultaBruta, nullptr);
    const auto finalizarConsulta = [](sqlite3_stmt* p) { if (p) sqlite3_finalize(p); };
    std::unique_ptr<sqlite3_stmt, decltype(finalizarConsulta)> consulta(consultaBruta, finalizarConsulta);
    if (preparo != SQLITE_OK)
        throw std::runtime_error("Estrutura SQLite invalida para registro_tiss: " +
                                 std::string(sqlite3_errmsg(banco.get())));

    ResumoProcessamento resumo;
    ValidadorRegistroTiss validador(catalogo);
    const auto obterTexto = [&](int coluna) -> std::string
    {
        const unsigned char* valor = sqlite3_column_text(consulta.get(), coluna);
        if (!valor) return "";
        return std::string(reinterpret_cast<const char*>(valor),
                           static_cast<std::size_t>(sqlite3_column_bytes(consulta.get(), coluna)));
    };

    int retorno;
    std::size_t linhaOrigem = 1;
    try
    {
        if (auditoria) auditoria->iniciar({caminhoBanco, competenciaArquivo, "SQLITE_NORMALIZADO_11",
            catalogo ? catalogo->getVersao() : "", catalogo ? catalogo->getTabela() : "",
            "registro_tiss: onze colunas normalizadas; linha de origem = ordinal da consulta + 1"});
        while ((retorno = sqlite3_step(consulta.get())) == SQLITE_ROW)
        {
            RegistroTissBruto registro(obterTexto(0), obterTexto(1), obterTexto(2), obterTexto(3),
                                      obterTexto(4), obterTexto(5), obterTexto(6), obterTexto(7),
                                      obterTexto(8), obterTexto(9), obterTexto(10));
            const auto resultado = competenciaArquivo.empty()
                ? validador.validar(registro)
                : validador.validar(registro, competenciaArquivo);
            ++linhaOrigem;
            if (auditoria) auditoria->gravar(linhaOrigem, linhaCsv(camposRegistro(registro)), &registro, resultado);
            resumo.incrementarTotal();
            switch (resultado.getEstado())
            {
            case EstadoValidacao::VALIDO: resumo.incrementarValidos(); break;
            case EstadoValidacao::VALIDO_COM_ALERTA: resumo.incrementarValidosComAlerta(); break;
            case EstadoValidacao::REJEITADO: resumo.incrementarRejeitados(); break;
            }
        }
        // SQLITE_DONE significa fim da consulta; erro nao pode produzir resumo de sucesso.
        if (retorno != SQLITE_DONE)
            throw std::runtime_error("Erro durante leitura SQLite: " + std::string(sqlite3_errmsg(banco.get())));

        if (auditoria) auditoria->finalizar();
    }
    catch (...) { if (auditoria) auditoria->cancelar(); throw; }

    const std::chrono::duration<double> duracao = std::chrono::steady_clock::now() - inicio;
    std::cout << "\n===== RESUMO SQLITE -> VALIDACAO =====\n"
              << "Total processado:       " << resumo.getTotal() << '\n'
              << "Validos:                " << resumo.getValidos() << '\n'
              << "Validos com alerta:     " << resumo.getValidosComAlerta() << '\n'
              << "Rejeitados:             " << resumo.getRejeitados() << '\n'
              << "Estrutura divergente:   " << resumo.getEstruturaDivergente() << '\n'
              << "Tempo de processamento: " << duracao.count() << " segundos\n";
}
