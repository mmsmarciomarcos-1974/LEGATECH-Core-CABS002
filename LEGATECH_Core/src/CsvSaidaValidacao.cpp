#include "../include/CsvSaidaValidacao.h"
#include <stdexcept>

std::vector<std::string> camposRegistro(const RegistroTissBruto& r)
{
    return {r.getIdEventoAtencaoSaude(), r.getUfPrestador(), r.getDtRealizacao(),
        r.getCdProcedimento(), r.getCdTabelaReferencia(), r.getQtItemEventoInformado(),
        r.getVlItemEventoInformado(), r.getVlItemPagoFornecedor(),
        r.getQtItemEventoPago(), r.getVlItemEventoPago(), r.getUnidadeMedida(),
        r.getIndPacote(), r.getIndTabelaPropria()};
}
std::string linhaCsv(const std::vector<std::string>& campos)
{
    std::string linha;
    for (const auto& campo : campos)
    {
        if (!linha.empty()) linha += ';';
        linha += '"';
        for (char c : campo) { if (c == '"') linha += '"'; linha += c; }
        linha += '"';
    }
    return linha;
}
// Arrays JSON dentro de celulas CSV: preservam ordem e limites das mensagens.
static std::string mensagensJson(const std::vector<std::string>& mensagens)
{
    const char* hex = "0123456789abcdef";
    std::string valor = "[";
    bool primeira = true;
    for (const auto& mensagem : mensagens)
    {
        if (!primeira) valor += ',';
        primeira = false;
        valor += '"';
        for (unsigned char c : mensagem)
        {
            if (c == '"' || c == '\\') { valor += '\\'; valor += static_cast<char>(c); }
            else if (c < 32) { valor += "\\u00"; valor += hex[c >> 4]; valor += hex[c & 15]; }
            else valor += static_cast<char>(c);
        }
        valor += '"';
    }
    return valor + ']';
}
void CsvSaidaValidacao::iniciar(const ContextoAuditoria& c)
{
    if (ativa) throw std::logic_error("Saida CSV ja iniciada");
    aceitos.exceptions(std::ios::badbit | std::ios::failbit);
    rejeitados.exceptions(std::ios::badbit | std::ios::failbit);
    aceitos.open(caminhoArquivoExportado(pasta, "aceitos", ".csv"), std::ios::binary);
    rejeitados.open(caminhoArquivoExportado(pasta, "rejeitados", ".csv"), std::ios::binary);
    ativa = true;
    const std::string cabecalho = "LINHA_ORIGEM;ESTADO;ID_EVENTO_ATENCAO_SAUDE;UF_PRESTADOR;DT_REALIZACAO;CD_PROCEDIMENTO;CD_TABELA_REFERENCIA;QT_ITEM_EVENTO_INFORMADO;VL_ITEM_EVENTO_INFORMADO;VL_ITEM_PAGO_FORNECEDOR;QT_ITEM_EVENTO_PAGO;VL_ITEM_EVENTO_PAGO;UNIDADE_MEDIDA;IND_PACOTE;IND_TABELA_PROPRIA;MOTIVOS_JSON;ALERTAS_JSON;LINHA_ORIGINAL\n";
    aceitos << cabecalho;
    rejeitados << cabecalho;
    std::ofstream contexto;
    contexto.exceptions(std::ios::badbit | std::ios::failbit);
    contexto.open(caminhoArquivoExportado(pasta, "contexto", ".csv"), std::ios::binary);
    contexto << "ARQUIVO;COMPETENCIA;CONTRATO;VERSAO_CATALOGO;TABELAS_CATALOGO;CABECALHO_ORIGEM\n"
             << linhaCsv({c.arquivo,c.competencia,c.contrato,c.versaoCatalogo,c.tabelaCatalogo,c.cabecalho}) << '\n';
    contexto.close();
}
void CsvSaidaValidacao::gravar(std::size_t linha, const std::string& original,
                              const RegistroTissBruto* registro, const ResultadoValidacao& resultado)
{
    if (!ativa) throw std::logic_error("Saida CSV nao iniciada");
    const auto estado = resultado.getEstado();
    const char* nome = estado == EstadoValidacao::VALIDO ? "VALIDO" :
        estado == EstadoValidacao::VALIDO_COM_ALERTA ? "VALIDO_COM_ALERTA" : "REJEITADO";
    auto campos = registro ? camposRegistro(*registro) : std::vector<std::string>(11);
    campos.insert(campos.begin(), {std::to_string(linha),nome});
    campos.push_back(mensagensJson(resultado.getMotivos()));
    campos.push_back(mensagensJson(resultado.getAlertas()));
    campos.push_back(original);
    auto& arquivo = estado == EstadoValidacao::REJEITADO ? rejeitados : aceitos;
    arquivo << linhaCsv(campos) << '\n';
}
void CsvSaidaValidacao::finalizar()
{
    if (!ativa) throw std::logic_error("Saida CSV nao iniciada");
    aceitos.close(); rejeitados.close(); ativa = false;
}
void CsvSaidaValidacao::cancelar() noexcept
{
    aceitos.exceptions(std::ios::goodbit); rejeitados.exceptions(std::ios::goodbit);
    if (aceitos.is_open()) aceitos.close();
    if (rejeitados.is_open()) rejeitados.close();
    ativa = false;
}
