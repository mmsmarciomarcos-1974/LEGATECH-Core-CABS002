#ifndef SQLITEAUDITORIAVALIDACAO_H
#define SQLITEAUDITORIAVALIDACAO_H

#include "AuditoriaValidacao.h"
#include "sqlite3.h"
#include <array>

// Conexao e comandos preparados reutilizados durante todo o arquivo.
// Separado de registro_tiss: a auditoria preserva os tres estados da validacao.
class SQLiteAuditoriaValidacao final : public AuditoriaValidacao
{
    sqlite3* banco = nullptr;
    sqlite3_stmt* inserirResultado = nullptr;
    sqlite3_stmt* inserirOcorrencia = nullptr;
    sqlite3_int64 execucao = 0;
    bool ativa = false;
    std::array<sqlite3_int64, 3> contagens{};
    sqlite3_int64 divergentes = 0;

    void executar(const char* sql);
    sqlite3_stmt* preparar(const char* sql);
    void verificar(int codigo);
    void texto(sqlite3_stmt* comando, int posicao, const std::string& valor);
    void numero(sqlite3_stmt* comando, int posicao, sqlite3_int64 valor);
    void concluir(sqlite3_stmt* comando);

public:
    explicit SQLiteAuditoriaValidacao(const std::string& caminhoBanco);
    ~SQLiteAuditoriaValidacao() override;
    SQLiteAuditoriaValidacao(const SQLiteAuditoriaValidacao&) = delete;
    SQLiteAuditoriaValidacao& operator=(const SQLiteAuditoriaValidacao&) = delete;
    void iniciar(const ContextoAuditoria& contexto) override;
    void gravar(std::size_t linhaFisica, const std::string& linhaOriginal,
                const RegistroTissBruto* registro,
                const ResultadoValidacao& resultado) override;
    void finalizar() override;
    void cancelar() noexcept override;
    sqlite3_int64 getIdExecucao() const { return execucao; }
};

#endif
