#include "../include/SQLiteAuditoriaValidacao.h"
#include <limits>
#include <stdexcept>

void SQLiteAuditoriaValidacao::verificar(int codigo)
{
    if (codigo != SQLITE_OK)
        throw std::runtime_error(std::string("Auditoria SQLite: ") + sqlite3_errmsg(banco));
}

void SQLiteAuditoriaValidacao::executar(const char* sql)
{
    verificar(sqlite3_exec(banco, sql, nullptr, nullptr, nullptr));
}

sqlite3_stmt* SQLiteAuditoriaValidacao::preparar(const char* sql)
{
    sqlite3_stmt* comando = nullptr;
    const int codigo = sqlite3_prepare_v2(banco, sql, -1, &comando, nullptr);
    if (codigo != SQLITE_OK) sqlite3_finalize(comando);
    verificar(codigo);
    return comando;
}

void SQLiteAuditoriaValidacao::texto(sqlite3_stmt* comando, int posicao, const std::string& valor)
{
    if (valor.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::runtime_error("Auditoria SQLite: texto excede limite de tamanho");
    verificar(sqlite3_bind_text(comando, posicao, valor.data(),
                               static_cast<int>(valor.size()), SQLITE_TRANSIENT));
}

void SQLiteAuditoriaValidacao::numero(sqlite3_stmt* comando, int posicao, sqlite3_int64 valor)
{
    verificar(sqlite3_bind_int64(comando, posicao, valor));
}

void SQLiteAuditoriaValidacao::concluir(sqlite3_stmt* comando)
{
    if (sqlite3_step(comando) != SQLITE_DONE)
        throw std::runtime_error(std::string("Auditoria SQLite: ") + sqlite3_errmsg(banco));
    verificar(sqlite3_reset(comando));
    verificar(sqlite3_clear_bindings(comando));
}

SQLiteAuditoriaValidacao::SQLiteAuditoriaValidacao(const std::string& caminhoBanco)
{
    // SQLITE_OPEN_READWRITE nao substitui um arquivo existente nem cria diretorios.
    const int codigo = sqlite3_open_v2(caminhoBanco.c_str(), &banco,
                                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    try
    {
        verificar(codigo);
        verificar(sqlite3_busy_timeout(banco, 5000));
        executar("PRAGMA foreign_keys = ON;");
        // Criacao atomica do esquema; nomes separados do DAO operacional existente.
        executar("BEGIN;"
            "CREATE TABLE IF NOT EXISTS auditoria_execucao ("
            "id INTEGER PRIMARY KEY, inicio_utc TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%fZ','now')),"
            "fim_utc TEXT, arquivo TEXT NOT NULL, competencia TEXT NOT NULL, contrato TEXT NOT NULL,"
            "versao_catalogo TEXT NOT NULL, tabela_catalogo TEXT NOT NULL, cabecalho TEXT NOT NULL,"
            "versao_esquema INTEGER NOT NULL DEFAULT 1 CHECK(versao_esquema=1),"
            "total INTEGER NOT NULL DEFAULT 0, validos INTEGER NOT NULL DEFAULT 0,"
            "alertas INTEGER NOT NULL DEFAULT 0, rejeitados INTEGER NOT NULL DEFAULT 0,"
            "estrutura_divergente INTEGER NOT NULL DEFAULT 0);"
            "CREATE TABLE IF NOT EXISTS auditoria_resultado ("
            "id INTEGER PRIMARY KEY, execucao_id INTEGER NOT NULL REFERENCES auditoria_execucao(id),"
            "linha_fisica INTEGER NOT NULL CHECK(linha_fisica>=2), linha_original TEXT NOT NULL,"
            "id_evento TEXT, cd_tabela_referencia TEXT, cd_procedimento TEXT,"
            "estado TEXT NOT NULL CHECK(estado IN ('VALIDO','VALIDO_COM_ALERTA','REJEITADO')),"
            "estrutura_divergente INTEGER NOT NULL CHECK(estrutura_divergente IN (0,1)),"
            "UNIQUE(execucao_id,linha_fisica));"
            "CREATE TABLE IF NOT EXISTS auditoria_ocorrencia ("
            "resultado_id INTEGER NOT NULL REFERENCES auditoria_resultado(id),"
            "tipo TEXT NOT NULL CHECK(tipo IN ('MOTIVO','ALERTA')), ordem INTEGER NOT NULL,"
            "mensagem TEXT NOT NULL, PRIMARY KEY(resultado_id,tipo,ordem));"
            "COMMIT;");
        inserirResultado = preparar("INSERT INTO auditoria_resultado "
            "(execucao_id,linha_fisica,linha_original,id_evento,cd_tabela_referencia,cd_procedimento,estado,estrutura_divergente) "
            "VALUES(?,?,?,?,?,?,?,?);");
        inserirOcorrencia = preparar("INSERT INTO auditoria_ocorrencia VALUES(?,?,?,?);");
    }
    catch (...)
    {
        sqlite3_finalize(inserirResultado);
        sqlite3_finalize(inserirOcorrencia);
        if (banco) sqlite3_exec(banco, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(banco);
        throw;
    }
}

SQLiteAuditoriaValidacao::~SQLiteAuditoriaValidacao()
{
    cancelar();
    sqlite3_finalize(inserirResultado);
    sqlite3_finalize(inserirOcorrencia);
    sqlite3_close(banco);
}

void SQLiteAuditoriaValidacao::iniciar(const ContextoAuditoria& contexto)
{
    if (ativa) throw std::logic_error("Auditoria: execucao ja iniciada");
    executar("BEGIN IMMEDIATE;");
    ativa = true;
    contagens.fill(0);
    divergentes = 0;
    execucao = 0;
    sqlite3_stmt* comando = nullptr;
    try
    {
        comando = preparar("INSERT INTO auditoria_execucao "
            "(arquivo,competencia,contrato,versao_catalogo,tabela_catalogo,cabecalho) VALUES(?,?,?,?,?,?);");
        texto(comando, 1, contexto.arquivo);
        texto(comando, 2, contexto.competencia);
        texto(comando, 3, contexto.contrato);
        texto(comando, 4, contexto.versaoCatalogo);
        texto(comando, 5, contexto.tabelaCatalogo);
        texto(comando, 6, contexto.cabecalho);
        concluir(comando);
        execucao = sqlite3_last_insert_rowid(banco);
        sqlite3_finalize(comando);
    }
    catch (...) { sqlite3_finalize(comando); cancelar(); throw; }
}

void SQLiteAuditoriaValidacao::gravar(std::size_t linhaFisica, const std::string& linhaOriginal,
                                    const RegistroTissBruto* registro, const ResultadoValidacao& resultado)
try
{
    if (!ativa) throw std::logic_error("Auditoria: execucao nao iniciada");
    const char* estado = nullptr;
    std::size_t indice = 0;
    switch (resultado.getEstado())
    {
        case EstadoValidacao::VALIDO: estado = "VALIDO"; indice = 0; break;
        case EstadoValidacao::VALIDO_COM_ALERTA: estado = "VALIDO_COM_ALERTA"; indice = 1; break;
        case EstadoValidacao::REJEITADO: estado = "REJEITADO"; indice = 2; break;
    }
    if (!estado || (!registro && resultado.getEstado() != EstadoValidacao::REJEITADO))
        throw std::logic_error("Auditoria: estado incompativel com registro");
    numero(inserirResultado, 1, execucao);
    numero(inserirResultado, 2, static_cast<sqlite3_int64>(linhaFisica));
    texto(inserirResultado, 3, linhaOriginal);
    if (registro)
    {
        texto(inserirResultado, 4, registro->getIdEventoAtencaoSaude());
        texto(inserirResultado, 5, registro->getCdTabelaReferencia());
        texto(inserirResultado, 6, registro->getCdProcedimento());
    }
    // Parametros nao preenchidos permanecem NULL quando a estrutura nao e confiavel.
    texto(inserirResultado, 7, estado);
    numero(inserirResultado, 8, registro ? 0 : 1);
    concluir(inserirResultado);
    const auto idResultado = sqlite3_last_insert_rowid(banco);
    auto ocorrencias = [&](const auto& mensagens, const char* tipo) {
        sqlite3_int64 ordem = 0;
        for (const auto& mensagem : mensagens)
        {
            numero(inserirOcorrencia, 1, idResultado);
            texto(inserirOcorrencia, 2, tipo);
            numero(inserirOcorrencia, 3, ++ordem);
            texto(inserirOcorrencia, 4, mensagem);
            concluir(inserirOcorrencia);
        }
    };
    ocorrencias(resultado.getMotivos(), "MOTIVO");
    ocorrencias(resultado.getAlertas(), "ALERTA");
    ++contagens[indice];
    if (!registro) ++divergentes;
}
catch (...) { cancelar(); throw; }

void SQLiteAuditoriaValidacao::finalizar()
{
    if (!ativa) throw std::logic_error("Auditoria: execucao nao iniciada");
    sqlite3_stmt* comando = nullptr;
    try
    {
        comando = preparar("UPDATE auditoria_execucao SET fim_utc=strftime('%Y-%m-%dT%H:%M:%fZ','now'),"
            "total=?,validos=?,alertas=?,rejeitados=?,estrutura_divergente=? WHERE id=?;");
        numero(comando, 1, contagens[0] + contagens[1] + contagens[2]);
        for (int i = 0; i < 3; ++i) numero(comando, i + 2, contagens[i]);
        numero(comando, 5, divergentes);
        numero(comando, 6, execucao);
        concluir(comando);
        sqlite3_finalize(comando);
        comando = nullptr;
        executar("COMMIT;");
        ativa = false;
    }
    catch (...) { sqlite3_finalize(comando); cancelar(); throw; }
}

void SQLiteAuditoriaValidacao::cancelar() noexcept
{
    // reset/clear permitem reuso apos falha; rollback preserva execucoes anteriores.
    sqlite3_reset(inserirResultado);
    sqlite3_clear_bindings(inserirResultado);
    sqlite3_reset(inserirOcorrencia);
    sqlite3_clear_bindings(inserirOcorrencia);
    if (ativa)
    {
        sqlite3_exec(banco, "ROLLBACK;", nullptr, nullptr, nullptr);
        ativa = false;
        execucao = 0;
    }
}
