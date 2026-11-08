#include "CatalogoTuss.h"
#include "ValidadorRegistroTiss.h"
#include "CsvDAOEntrada.h"
#include "ContratoAmbDetAte2022.h"
#include "SQLiteDAOEntrada.h"
#include "SQLiteDAOSaida.h"
#include "ProcessadorTiss.h"
#include "SQLiteAuditoriaValidacao.h"
#include "SaidasValidacao.h"
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
    exigir(argc == 5, "caminhos dos catalogos 22, 63, diretorio TUSS 19 e catalogo 60");
    CatalogoTuss catalogo(argv[1], "202501", "22");
    exigir(catalogo.tamanho() == 5964, "carga integral");
    exigir(catalogo.consultar("19", "10101012") == nullptr, "par isolado por tabela");
    CatalogoTuss ambos(std::vector<ReferenciaCatalogoTuss>{{argv[1], "22"}, {argv[2], "63"}}, "202501");
    exigir(ambos.tamanho() == 5995, "carga integral das duas tabelas");
    exigir(ambos.contemTabela("22") && ambos.contemTabela("63") && !ambos.contemTabela("19"), "tabelas carregadas");
    exigir(ambos.getTabela() == "22,63", "referencias para auditoria");
    const std::vector<std::size_t> registrosTuss19PorParte{
        184598, 184598, 184598, 184598, 184598, 184598, 184594
    };
    std::vector<ReferenciaCatalogoTuss> referenciasTuss19;
    for (std::size_t i = 0; i < registrosTuss19PorParte.size(); ++i)
    {
        const std::string parte = i + 1 < 10 ? "0" + std::to_string(i + 1) : std::to_string(i + 1);
        referenciasTuss19.push_back({
            (std::filesystem::path(argv[3]) / ("tuss19_202501_part" + parte + ".csv")).string(),
            "19",
            registrosTuss19PorParte[i]
        });
    }
    CatalogoTuss tuss19(referenciasTuss19, "202501");
    exigir(tuss19.tamanho() == 1292182, "carga integral TUSS 19 em sete partes");
    exigir(tuss19.contemTabela("19") && tuss19.getTabela() == "19", "TUSS 19 identificada sem duplicar a tabela");
    const TermoTuss* termo19 = tuss19.consultar("19", "71786074");
    exigir(termo19 && termo19->inicioVigencia == std::chrono::sys_days{std::chrono::year{2017}/4/10},
           "consulta TUSS 19 e vigencia do snapshot");
    ValidadorRegistroTiss validarTuss19(&tuss19);
    exigir(validarTuss19.validar(registro("71786074", "19")).getEstado() == EstadoValidacao::VALIDO,
           "codigo TUSS 19 localizado e vigente");
    exigir(validarTuss19.validar(registro("00000000", "19")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA,
           "codigo ausente na TUSS 19 gera alerta, nao rejeicao");
    exigir(validarTuss19.validar(registro("1234567", "19")).getEstado() == EstadoValidacao::REJEITADO,
           "codigo TUSS 19 com sete digitos rejeitado");
    exigir(validarTuss19.validar(registro("1234567890", "19")).getEstado() == EstadoValidacao::REJEITADO,
           "codigo TUSS 19 com dez digitos rejeitado");
    CatalogoTuss tuss60(argv[4], "202501", "60");
    exigir(tuss60.tamanho() == 61, "carga integral TUSS 60");
    exigir(tuss60.contemTabela("60") && tuss60.consultar("60", "036"),
           "codigo TUSS 60 preserva tres digitos e zero inicial");
    exigir(tuss60.consultar("60", "36") == nullptr, "TUSS 60 nao completa zeros automaticamente");
    ValidadorRegistroTiss validarTuss60(&tuss60);
    auto registroComUnidade = [](const std::string& unidade, const std::string& mes = "2025-01") {
        return RegistroTissBruto("1", "PR", mes, "A1", "00", "1", "100", "0",
                                 unidade, "0", "1");
    };
    auto temOcorrencia = [](const std::vector<std::string>& ocorrencias,
                            const std::string& trecho) {
        for (const auto& ocorrencia : ocorrencias)
            if (ocorrencia.find(trecho) != std::string::npos) return true;
        return false;
    };
    exigir(validarTuss60.validar(registroComUnidade("036")).getEstado() == EstadoValidacao::VALIDO,
           "unidade TUSS 60 valida e vigente");
    exigir(validarTuss60.validar(registroComUnidade("")).getEstado() == EstadoValidacao::VALIDO,
           "unidade vazia aceita por obrigatoriedade condicional");
    const auto unidadeAusente = validarTuss60.validar(registroComUnidade("999"));
    exigir(unidadeAusente.getEstado() == EstadoValidacao::REJEITADO &&
           temOcorrencia(unidadeAusente.getMotivos(), "fora do dominio TUSS 60"),
           "unidade fora da TUSS 60 rejeitada com motivo");
    const auto unidadeFormato = validarTuss60.validar(registroComUnidade("36"));
    exigir(unidadeFormato.getEstado() == EstadoValidacao::REJEITADO &&
           temOcorrencia(unidadeFormato.getMotivos(), "formato invalido"),
           "unidade sem tres digitos rejeitada");
    const auto unidadeAntesDaVigencia = validarTuss60.validar(
        registroComUnidade("057", "2016-05"));
    exigir(unidadeAntesDaVigencia.getEstado() == EstadoValidacao::REJEITADO &&
           temOcorrencia(unidadeAntesDaVigencia.getMotivos(),
                         "UNIDADE_MEDIDA_FORA_DA_VIGENCIA"),
           "unidade TUSS 60 antes da vigencia rejeitada");
    exigir(validarTuss60.validar(registroComUnidade("057", "2016-06")).getEstado() ==
               EstadoValidacao::VALIDO,
           "unidade TUSS 60 valida no mes inicial");
    const auto unidadeInicioNoMes = validarTuss60.validar(
        registroComUnidade("001", "2012-10"));
    exigir(unidadeInicioNoMes.getEstado() == EstadoValidacao::VALIDO_COM_ALERTA &&
           temOcorrencia(unidadeInicioNoMes.getAlertas(), "VIGENCIA_PARCIAL_NO_MES"),
           "unidade TUSS 60 alerta quando mes nao resolve dia de inicio");
    ValidadorRegistroTiss comGrupos(&ambos);
    exigir(comGrupos.validar(registro("018", "63")).getEstado() == EstadoValidacao::VALIDO, "grupo vigente com zeros");
    exigir(comGrupos.validar(registro("004", "63")).getEstado() == EstadoValidacao::VALIDO, "segundo grupo vigente");
    exigir(comGrupos.validar(registro("18", "63")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "nao completar zeros");
    exigir(comGrupos.validar(registro("999", "63")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "grupo ausente no snapshot");
    exigir(comGrupos.validar(registro("10101012", "63")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "codigo 22 nao localizado em 63");
    exigir(comGrupos.validar(registro("018", "22")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "grupo 63 nao localizado em 22");
    exigir(comGrupos.validar(registro("10101012")).getEstado() == EstadoValidacao::VALIDO, "tabela 22 preservada");
    exigir(comGrupos.validar(registro("40311511")).getEstado() == EstadoValidacao::REJEITADO, "vigencia 22 preservada");
    auto validarSemHistorico = [&](const RegistroTissBruto& item) {
        ValidadorRegistroTiss validador(&catalogo);
        return validador.validar(item);
    };
    auto validarComCompetencia = [&](const RegistroTissBruto& item,
                                     const std::string& competencia) {
        ValidadorRegistroTiss validador(&catalogo);
        return validador.validar(item, competencia);
    };
    exigir(validarSemHistorico(registro("10101012")).getEstado() == EstadoValidacao::VALIDO, "consulta vigente");
    exigir(validarSemHistorico(registro("99999999")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "ausencia no snapshot");
    exigir(validarSemHistorico(registro("010101012")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "nao remover zeros");
    exigir(validarSemHistorico(registro("40311511")).getEstado() == EstadoValidacao::REJEITADO, "inicio futuro");
    exigir(validarSemHistorico(registro("40311511", "22", "2025-02")).getEstado() == EstadoValidacao::VALIDO, "mes inicial completo");
    exigir(validarSemHistorico(registro("40309223", "22", "2025-04")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "fronteira do fim");
    exigir(validarSemHistorico(registro("40309223", "22", "2025-05")).getEstado() == EstadoValidacao::REJEITADO, "apos fim");
    exigir(validarSemHistorico(registro("018", "63")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "catalogo outra tabela");
    exigir(validarSemHistorico(registro("A1", "00", "2025-01", "0", "1")).getEstado() == EstadoValidacao::VALIDO, "propria alfanumerica");
    exigir(validarSemHistorico(registro("A1", "90", "2025-01", "1")).getEstado() == EstadoValidacao::VALIDO, "pacote alfanumerico");
    exigir(validarSemHistorico(registro("A1", "98", "2025-01", "0")).getEstado() == EstadoValidacao::REJEITADO, "pacote incoerente");

    // Matriz RF04: indicadores aceitos/rejeitados conforme a referencia.
    auto avaliarIndicadores = [&](const std::string& tabela,
                                  const std::string& pacote,
                                  const std::string& propria) {
        const bool referenciaPropriaOuPacote =
            tabela == "00" || tabela == "90" || tabela == "98";
        const std::string codigo = referenciaPropriaOuPacote ? "COD-1" : "10101012";
        return validarSemHistorico(
            registro(codigo, tabela, "2025-01", pacote, propria));
    };
    auto possuiMotivoDoIndicador = [&](const ResultadoValidacao& r,
                                        const std::string& campo) {
        for (const auto& motivo : r.getMotivos())
            if (motivo.find(campo) != std::string::npos) return true;
        return false;
    };

    // Tabela 00 exige IND_TABELA_PROPRIA=1; IND_PACOTE continua opcional.
    for (const auto& pacote : {"", "0", "1"})
        exigir(avaliarIndicadores("00", pacote, "1").getEstado() == EstadoValidacao::VALIDO,
               "referencia 00 aceita tabela propria e indicador de pacote opcional");
    for (const auto& propria : {"", "0"}) {
        const auto r = avaliarIndicadores("00", "0", propria);
        exigir(r.getEstado() == EstadoValidacao::REJEITADO &&
               possuiMotivoDoIndicador(r, "CD_TABELA_REFERENCIA"),
               "referencia 00 rejeita tabela propria ausente ou desligada");
    }

    // 90/98 exigem IND_PACOTE=1; o indicador de tabela própria não é inferido.
    for (const auto& tabela : {"90", "98"}) {
        for (const auto& propria : {"", "0", "1"})
            exigir(avaliarIndicadores(tabela, "1", propria).getEstado() == EstadoValidacao::VALIDO,
                   "referencia de pacote aceita indicador proprio opcional");
        for (const auto& pacote : {"", "0"}) {
            const auto r = avaliarIndicadores(tabela, pacote, "");
            exigir(r.getEstado() == EstadoValidacao::REJEITADO &&
                   possuiMotivoDoIndicador(r, "IND_PACOTE"),
                   "referencia 90/98 rejeita IND_PACOTE ausente ou desligado");
        }
    }

    // TUSS permite pacote como contexto de componente, mas contradiz tabela própria.
    for (const auto& tabela : {"19", "20", "22", "63"}) {
        for (const auto& pacote : {"", "0", "1"})
            for (const auto& propria : {"", "0"}) {
                const auto r = avaliarIndicadores(tabela, pacote, propria);
                exigir(r.getEstado() != EstadoValidacao::REJEITADO &&
                       r.getMotivos().empty(),
                       "TUSS aceita combinacao sem tabela propria");
            }
        const auto propriaIncompativel = avaliarIndicadores(tabela, "0", "1");
        exigir(propriaIncompativel.getEstado() == EstadoValidacao::REJEITADO &&
               possuiMotivoDoIndicador(propriaIncompativel, "IND_TABELA_PROPRIA"),
               "TUSS rejeita indicador de tabela propria");
    }

    for (const auto& campo : {"IND_PACOTE", "IND_TABELA_PROPRIA"}) {
        const auto r = campo == std::string("IND_PACOTE")
            ? avaliarIndicadores("22", "2", "0")
            : avaliarIndicadores("22", "0", "2");
        exigir(r.getEstado() == EstadoValidacao::REJEITADO &&
               possuiMotivoDoIndicador(r, campo),
               "indicador fora do dominio binario rejeitado");
    }
    std::cout << "OK: combinacoes IND_PACOTE e IND_TABELA_PROPRIA por referencia\n";
    exigir(validarSemHistorico(registro("ABC")).getEstado() == EstadoValidacao::REJEITADO, "procedimento nao numerico");
    exigir(validarSemHistorico(registro("10101012", "22", "2025-99")).getEstado() == EstadoValidacao::REJEITADO, "mes invalido");
    exigir(ValidadorRegistroTiss{}.validar(registro("10101012")).getEstado() == EstadoValidacao::VALIDO_COM_ALERTA, "compatibilidade sem catalogo explicita limite");

    // RF05: competencia mensal, sem fabricar dia ou horario.
    auto temporal = [&](const std::string& mes, const std::string& competencia = "") {
        return validarComCompetencia(registro("A1", "00", mes, "0", "1"), competencia);
    };
    auto temAlertaTemporal = [&](const ResultadoValidacao& r, const std::string& trecho) {
        for (const auto& alerta : r.getAlertas())
            if (alerta.find(trecho) != std::string::npos) return true;
        return false;
    };
    for (const auto& mes : {"", "0000-01", "2025-00", "2025-13", "2025-1", "202501", "2025-01-01", "abcd-01"})
    {
        const auto r = temporal(mes);
        exigir(r.getEstado() == EstadoValidacao::REJEITADO && !r.getMotivos().empty(), "data invalida com motivo");
        exigir(!temAlertaTemporal(r, "Y2K38"), "invalido nao gera alerta temporal");
    }
    for (const auto& mes : {"0001-01", "2024-02", "2037-12"})
        exigir(temporal(mes).getEstado() == EstadoValidacao::VALIDO, "mes valido antes da fronteira");
    for (const auto& mes : {"2038-01", "2038-02", "2040-02", "9999-12"})
    {
        const auto r = temporal(mes);
        exigir(r.getEstado() == EstadoValidacao::VALIDO_COM_ALERTA && r.getMotivos().empty(), "Y2K38 nao rejeita data valida");
        exigir(temAlertaTemporal(r, "COMPATIBILIDADE_Y2K38"), "alerta identificado");
    }
    exigir(temporal("2025-01", "202501").getEstado() == EstadoValidacao::VALIDO, "competencia igual");
    exigir(temAlertaTemporal(temporal("2025-01", "202502"), "difere da competencia"), "divergencia contextual alerta");
    const auto combinado = temporal("2038-01", "203802");
    exigir(combinado.getAlertas().size() == 2, "Y2K38 e competencia coexistem");
    std::cout << "OK: datas mensais, ano zero, competencia e alerta Y2K38\n";

    // RF04: limites textuais e precisao sem conversao para ponto flutuante.
    auto avaliarCampos = [&](const std::string& id, const std::string& quantidade,
                             const std::string& informado, const std::string& pago,
                             const std::string& unidade = "",
                             const std::string& pacote = "0",
                             const std::string& propria = "0") {
        return validarComCompetencia(
            RegistroTissBruto(id,"PR","2025-01","10101012","22",
                              quantidade,informado,pago,unidade,pacote,propria),
            "202501"
        );
    };
    auto temMotivo = [&](const ResultadoValidacao& r, const std::string& trecho) {
        for (const auto& motivo : r.getMotivos()) if (motivo.find(trecho) != std::string::npos) return true;
        return false;
    };
    auto temAlerta = [&](const ResultadoValidacao& r, const std::string& trecho) {
        for (const auto& alerta : r.getAlertas()) if (alerta.find(trecho) != std::string::npos) return true;
        return false;
    };
    exigir(avaliarCampos(std::string(30,'9'),"999999","9999999999999999.99","0").getEstado()
            == EstadoValidacao::VALIDO, "limites exatos aceitos");
    exigir(temMotivo(avaliarCampos(std::string(31,'9'),"1","100","0"),"30 digitos"), "ID acima do limite");
    for (const auto& q : {".5","5.","1,2.3","1e3","-1","1000000","12345.67","0.1234567"})
        exigir(temMotivo(avaliarCampos("1",q,"100","0"),"QT_ITEM_EVENTO_INFORMADO"), "quantidade invalida");
    for (const auto& q : {"0","1.5","1,5","1234.56","0.123456","000001"})
        exigir(avaliarCampos("1",q,"100","0").getEstado() == EstadoValidacao::VALIDO, "quantidade dentro da precisao");
    exigir(avaliarCampos("1","","100","0").getEstado() == EstadoValidacao::VALIDO,
           "quantidade vazia aceita no extrato quando nao ha obrigatoriedade publicada");
    for (const auto& valor : {".5","5.","100.123","100,123","99999999999999999","1e3","NaN"})
    {
        exigir(temMotivo(avaliarCampos("1","1",valor,"0"),"VL_ITEM_EVENTO_INFORMADO"), "valor informado invalido");
        exigir(temMotivo(avaliarCampos("1","1","100",valor),"VL_ITEM_PAGO_FORNECEDOR"), "valor pago invalido");
    }
    const auto valoresVazios = avaliarCampos("1","1","","","");
    exigir(temMotivo(valoresVazios,"VL_ITEM_EVENTO_INFORMADO") &&
           temMotivo(valoresVazios,"VL_ITEM_PAGO_FORNECEDOR"),
           "valores obrigatorios identificados quando vazios fora de pacote");
    exigir(avaliarCampos("1","1","","","","1").getEstado() == EstadoValidacao::VALIDO,
           "valores vazios permitidos em componente de pacote");
    exigir(avaliarCampos("1","1","0","0").getEstado() == EstadoValidacao::VALIDO,
           "zero aceito como valor informado e pago");
    const auto informadoNegativo = avaliarCampos("1","1","-0.01","0");
    exigir(informadoNegativo.getEstado() == EstadoValidacao::REJEITADO &&
           temMotivo(informadoNegativo,"VL_ITEM_EVENTO_INFORMADO"),
           "valor informado negativo rejeitado");
    const auto pagoNegativo = avaliarCampos("1","1","0","-0.01");
    exigir(pagoNegativo.getEstado() == EstadoValidacao::VALIDO_COM_ALERTA &&
           temAlerta(pagoNegativo,"estorno") &&
           temAlerta(pagoNegativo,"saldo acumulado da guia"),
           "valor pago negativo aceito como alerta de estorno sem validar saldo da guia");
    const auto pagoNegativoPrecisao = avaliarCampos("1","1","0","-0.001");
    exigir(pagoNegativoPrecisao.getEstado() == EstadoValidacao::REJEITADO &&
           temMotivo(pagoNegativoPrecisao,"VL_ITEM_PAGO_FORNECEDOR"),
           "valor pago negativo fora da precisao rejeitado");
    exigir(temMotivo(avaliarCampos("1","1","000100,50","0",std::string(40,'A')),
                     "UNIDADE_MEDIDA: formato invalido"),
           "unidade de 40 caracteres deve obedecer ao dominio TUSS 60");
    exigir(temMotivo(avaliarCampos("1","1","100","0",std::string(41,'A')),"UNIDADE_MEDIDA"), "unidade acima de 40");
    exigir(validarSemHistorico(registro(std::string(40,'A'),"00","2025-01","0","1")).getEstado() == EstadoValidacao::VALIDO,
            "codigo proprio tamanho 40 aceito");
    exigir(temMotivo(validarSemHistorico(registro(std::string(41,'A'),"00","2025-01","0","1")),"CD_PROCEDIMENTO: tamanho"),
            "codigo proprio acima de 40");
    exigir(temMotivo(validarSemHistorico(registro("1",std::string(41,'9'))),"CD_TABELA_REFERENCIA: tamanho"), "tabela acima de 40");
    std::cout << "OK: limites de campos, precisao decimal, fronteiras e regras de campos vazios\n";

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
    // Cenário independente: um novo processador representa outra execução de arquivo.
    ProcessadorTiss processadorComCompetencia(saida, &catalogo);
    exigir(processadorComCompetencia.processar(registro("10101012"), "202502").getEstado() == EstadoValidacao::VALIDO_COM_ALERTA,
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
        auditoria.gravar(2, "linha", &r, validarSemHistorico(r));
        bool duplicou = false;
        try { auditoria.gravar(2, "linha duplicada", &r, validarSemHistorico(r)); }
        catch (const std::exception&) { duplicou = true; }
        exigir(duplicou && auditoria.getIdExecucao() == 0, "falha cancela execucao");
        auditoria.iniciar(contexto);
        auditoria.gravar(2, "linha confirmada", &r, validarSemHistorico(r));
        auditoria.finalizar();
        auditoria.iniciar(contexto);
        auditoria.gravar(2, "linha abandonada", &r, validarSemHistorico(r));
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

    // Falha depois de gravar CSV nao deve publicar nenhum dos dois formatos.
    const auto exportacao = diretorio / "exportacao";
    {
        SaidasValidacao saidas(exportacao.string());
        DAOSaidaValidacao* dao = &saidas;
        dao->iniciar(contexto);
        const auto r = registro("10101012");
        dao->gravar(2, "linha", &r, validarSemHistorico(r));
        bool falhou = false;
        try { dao->gravar(2, "duplicada", &r, validarSemHistorico(r)); }
        catch (const std::exception&) { falhou = true; }
        exigir(falhou && !std::filesystem::exists(exportacao), "falha nao publica formatos parciais");
        for (const auto& entrada : std::filesystem::directory_iterator(diretorio))
            exigir(entrada.path().filename().string().find("exportacao.tmp-") != 0, "limpeza apos falha");
        dao->iniciar(contexto);
        dao->gravar(2, "confirmada", &r, validarSemHistorico(r));
        dao->finalizar();
        exigir(std::filesystem::exists(exportacao / "resultados.db") &&
               std::filesystem::exists(exportacao / "aceitos.csv"), "reuso publica os dois formatos");
    }
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
