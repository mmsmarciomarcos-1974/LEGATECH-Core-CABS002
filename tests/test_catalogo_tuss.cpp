#include "CatalogoTuss.h"
#include "ValidadorRegistroTiss.h"
#include "CsvDAOEntrada.h"
#include "ContratoAmbDetAte2022.h"
#include "SQLiteDAOEntrada.h"
#include "SQLiteDAOSaida.h"
#include "ProcessadorTiss.h"
#include "SQLiteAuditoriaValidacao.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

void exigir(bool ok, const std::string& caso)
{
    if (!ok) throw std::runtime_error("Teste falhou: " + caso);
}

RegistroTissBruto registro(const std::string& codigo, const std::string& tabela = "22",
                          const std::string& mes = "2025-01",
                          const std::string& pacote = "0", const std::string& propria = "0")
{
    return {"1", "PR", mes, codigo, tabela, "1", "100", "0", "", pacote, propria};
}

int main(int argc, char* argv[])
try
{
    exigir(argc == 3, "caminhos dos catalogos 22 e 63");
    CatalogoTuss catalogo(argv[1], "202501", "22");
    exigir(catalogo.tamanho() == 5964, "carga integral");
    exigir(catalogo.consultar("19", "10101012") == nullptr, "par isolado por tabela");
    CatalogoTuss ambos(std::vector<ReferenciaCatalogoTuss>{{argv[1], "22"}, {argv[2], "63"}}, "202501");
    exigir(ambos.tamanho() == 5995, "carga integral das duas tabelas");
    exigir(ambos.contemTabela("22") && ambos.contemTabela("63") && !ambos.contemTabela("19"), "tabelas carregadas");
    exigir(ambos.getTabela() == "22,63", "referencias para auditoria");
    ValidadorRegistroTiss comGrupos(&ambos);
    exigir(comGrupos.validar(registro("018", "63")).getEstado() == EstadoValidacao::VALIDO, "grupo vigente com zeros");
    exigir(comGrupos.validar(registro("004", "63")).getEstado() == EstadoValidacao::VALIDO, "segundo grupo vigente");
    exigir(comGrupos.validar(registro("18", "63")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "nao completar zeros");
    exigir(comGrupos.validar(registro("999", "63")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "grupo ausente no snapshot");
    exigir(comGrupos.validar(registro("10101012", "63")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "codigo 22 nao localizado em 63");
    exigir(comGrupos.validar(registro("018", "22")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "grupo 63 nao localizado em 22");
    exigir(comGrupos.validar(registro("10101012")).getEstado() == EstadoValidacao::VALIDO, "tabela 22 preservada");
    exigir(comGrupos.validar(registro("40311511")).getEstado() == EstadoValidacao::REJEITADO, "vigencia 22 preservada");
    ValidadorRegistroTiss v(&catalogo);
    exigir(v.validar(registro("10101012")).getEstado() == EstadoValidacao::VALIDO, "consulta vigente");
    exigir(v.validar(registro("99999999")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "ausencia no snapshot");
    exigir(v.validar(registro("010101012")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "nao remover zeros");
    exigir(v.validar(registro("40311511")).getEstado() == EstadoValidacao::REJEITADO, "inicio futuro");
    exigir(v.validar(registro("40311511", "22", "2025-02")).getEstado() == EstadoValidacao::VALIDO, "mes inicial completo");
    exigir(v.validar(registro("40309223", "22", "2025-04")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "fronteira do fim");
    exigir(v.validar(registro("40309223", "22", "2025-05")).getEstado() == EstadoValidacao::REJEITADO, "apos fim");
    exigir(v.validar(registro("018", "63")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "catalogo outra tabela");
    exigir(v.validar(registro("A1", "00", "2025-01", "0", "1")).getEstado() == EstadoValidacao::VALIDO, "propria alfanumerica");
    exigir(v.validar(registro("A1", "90", "2025-01", "1")).getEstado() == EstadoValidacao::VALIDO, "pacote alfanumerico");
    exigir(v.validar(registro("A1", "98", "2025-01", "0")).getEstado() == EstadoValidacao::REJEITADO, "pacote incoerente");
    exigir(v.validar(registro("ABC")).getEstado() == EstadoValidacao::REJEITADO, "procedimento nao numerico");
    exigir(v.validar(registro("10101012", "22", "2025-99")).getEstado() == EstadoValidacao::REJEITADO, "mes invalido");
    exigir(ValidadorRegistroTiss{}.validar(registro("10101012")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "compatibilidade sem catalogo explicita limite");

    const auto diretorio = std::filesystem::temp_directory_path() /
        ("legatech-tuss-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directory(diretorio);
    struct Limpeza { std::filesystem::path p; ~Limpeza() { std::error_code e; std::filesystem::remove_all(p, e); } } limpeza{diretorio};
    const auto arquivo = diretorio / "catalogo.csv";
    const std::string cabecalho = "versao_catalogo;cd_tabela_referencia;cd_procedimento;termo;descricao_detalhada;inicio_vigencia;fim_vigencia;fim_implantacao\n";
    auto deveFalhar = [&](const std::string& conteudo) {
        { std::ofstream out(arquivo); out << conteudo; }
        bool falhou = false;
        try { CatalogoTuss c(arquivo.string(), "202501", "22"); }
        catch (const std::exception&) { falhou = true; }
        exigir(falhou, "catalogo incorreto rejeitado");
    };
    auto referenciasDevemFalhar = [&](const std::vector<ReferenciaCatalogoTuss>& referencias) {
        bool falhou = false;
        try { CatalogoTuss c(referencias, "202501"); }
        catch (const std::exception&) { falhou = true; }
        exigir(falhou, "conjunto incompleto/incorreto rejeitado");
    };
    referenciasDevemFalhar({});
    referenciasDevemFalhar({{argv[1], "22"}, {argv[1], "22"}});
    referenciasDevemFalhar({{argv[1], "22"}, {argv[1], "63"}});
    referenciasDevemFalhar({{argv[1], "22"}, {arquivo.string() + ".ausente", "63"}});
    {
        std::ofstream out(arquivo);
        out << cabecalho << "202501;63;018;Grupo;;2025-02-01;2025-03-15;\n";
    }
    CatalogoTuss fronteiras(arquivo.string(), "202501", "63");
    ValidadorRegistroTiss gruposFronteiras(&fronteiras);
    exigir(gruposFronteiras.validar(registro("018", "63")).getEstado() == EstadoValidacao::REJEITADO, "grupo antes da vigencia");
    exigir(gruposFronteiras.validar(registro("018", "63", "2025-02")).getEstado() == EstadoValidacao::VALIDO, "grupo mes vigente");
    exigir(gruposFronteiras.validar(registro("018", "63", "2025-03")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "grupo vigencia parcial");
    exigir(gruposFronteiras.validar(registro("018", "63", "2025-04")).getEstado() == EstadoValidacao::REJEITADO, "grupo depois da vigencia");
    for (const auto& codigo : {"18", "0018", "A18"})
    {
        { std::ofstream out(arquivo); out << cabecalho << "202501;63;" << codigo << ";Grupo;;2025-01-01;;\n"; }
        referenciasDevemFalhar({{arquivo.string(), "63"}});
    }
    {
        std::ofstream out(arquivo);
        out << cabecalho << "202501;20;10101012;Colisao;;2025-02-01;;\n";
    }
    CatalogoTuss colisao(std::vector<ReferenciaCatalogoTuss>{{argv[1], "22"}, {arquivo.string(), "20"}}, "202501");
    exigir(colisao.consultar("22", "10101012") && colisao.consultar("20", "10101012"), "pares da colisao carregados");
    exigir(colisao.consultar("22", "10101012")->inicioVigencia != colisao.consultar("20", "10101012")->inicioVigencia,
           "mesmo codigo em tabelas diferentes possui termos independentes");
    deveFalhar("");
    deveFalhar(cabecalho);
    deveFalhar(cabecalho + "202501;22;10101012;X;;2025-02-30;;\n");
    deveFalhar(cabecalho + "202501;22;10101012;X;;2025-02-01;2025-01-01;\n");
    deveFalhar(cabecalho + "202502;22;10101012;X;;2025-01-01;;\n");
    const std::string linha = "202501;22;10101012;X;;2025-01-01;;\n";
    deveFalhar(cabecalho + linha + linha);
    deveFalhar(cabecalho + "202501;22;10101012;\"campo truncado\n");
    { std::ofstream out(arquivo); out << cabecalho << "202501;22;10101012;\"termo; com \"\"aspas\"\"\nlinha\";;2025-01-01;;\n"; }
    exigir(CatalogoTuss(arquivo.string(), "202501", "22").tamanho() == 1, "CSV citado multilinha");
    bool faltou = false;
    try { CatalogoTuss c((diretorio / "ausente.csv").string(), "202501", "22"); }
    catch (const std::exception&) { faltou = true; }
    exigir(faltou, "arquivo ausente");

    // Mesmo validador no processador e na releitura SQLite.
    const auto banco = diretorio / "teste.db";
    SQLiteDAOSaida saida(banco.string());
    ProcessadorTiss processador(saida, &catalogo);
    exigir(processador.processar(registro("10101012")).getEstado() == EstadoValidacao::VALIDO, "processador recebe catalogo");
    exigir(processador.processar(registro("10101012"), "202502").getEstado() == EstadoValidacao::VALIDO_COM_ALERTA,
           "processador recebe competencia do arquivo");
    std::ostringstream captura;
    auto* anterior = std::cout.rdbuf(captura.rdbuf());
    SQLiteDAOEntrada entrada(banco.string(), &catalogo);
    entrada.processarArquivo();
    std::cout.rdbuf(anterior);
    exigir(captura.str().find("Validos:                1") != std::string::npos, "SQLite recebe catalogo");

    // Falha real de UNIQUE deve cancelar tudo e liberar comandos para nova execucao.
    const auto bancoAuditoria = diretorio / "auditoria.db";
    const ContextoAuditoria contexto{"entrada.csv", "202501", "AMB_DET", "202501", "22", "cabecalho"};
    {
        SQLiteAuditoriaValidacao auditoria(bancoAuditoria.string());
        auditoria.iniciar(contexto);
        const auto r = registro("10101012");
        auditoria.gravar(2, "linha", &r, v.validar(r));
        bool duplicou = false;
        try { auditoria.gravar(2, "linha duplicada", &r, v.validar(r)); }
        catch (const std::exception&) { duplicou = true; }
        exigir(duplicou && auditoria.getIdExecucao() == 0, "falha cancela execucao");
        auditoria.iniciar(contexto);
        auditoria.gravar(2, "linha confirmada", &r, v.validar(r));
        auditoria.finalizar();
        auditoria.iniciar(contexto);
        auditoria.gravar(2, "linha abandonada", &r, v.validar(r));
        // O destrutor cancela a terceira tentativa sem afetar a confirmada.
    }
    sqlite3* db = nullptr;
    exigir(sqlite3_open(bancoAuditoria.string().c_str(), &db) == SQLITE_OK, "abrir auditoria para consulta");
    sqlite3_stmt* consulta = nullptr;
    const int preparou = sqlite3_prepare_v2(db,
        "SELECT (SELECT COUNT(*) FROM auditoria_execucao), COUNT(*), MIN(linha_original) FROM auditoria_resultado",
        -1, &consulta, nullptr);
    bool auditoriaIntegra = false;
    if (preparou == SQLITE_OK && sqlite3_step(consulta) == SQLITE_ROW)
        auditoriaIntegra = sqlite3_column_int(consulta, 0) == 1 && sqlite3_column_int(consulta, 1) == 1 &&
            std::string(reinterpret_cast<const char*>(sqlite3_column_text(consulta, 2))) == "linha confirmada";
    sqlite3_finalize(consulta);
    sqlite3_close(db);
    exigir(auditoriaIntegra, "rollback automatico, reuso e cancelamento no destrutor");

    // Normalizacao historica continua preenchendo UNIDADE_MEDIDA vazia.
    ContratoAmbDetAte2022 historico;
    CsvDAOEntrada csv("nao_aberto.csv", historico, "202201", &catalogo);
    const auto normalizado = csv.normalizarCampos({"1","PR","2022-01","10101012","22","1","100","0","0","0"}, historico);
    exigir(normalizado.size() == 11 && normalizado[8].empty(), "contrato historico preservado");
    std::cout << "OK: catalogo, par, temporalidade, falhas de carga, SQLite, auditoria e contrato historico\n";
}
catch (const std::exception& erro)
{
    std::cerr << erro.what() << '\n';
    return 1;
}
