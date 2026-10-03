#ifndef RESULTADOVALIDACAO_H
#define RESULTADOVALIDACAO_H

#include <string>
#include <vector>

enum class EstadoValidacao
{
    VALIDO,
    VALIDO_COM_ALERTA,
    REJEITADO
};

class ResultadoValidacao
{
private:
    EstadoValidacao estado;

    std::vector<std::string> motivos;
    std::vector<std::string> alertas;

public:
    ResultadoValidacao(
        EstadoValidacao estado
    );

    EstadoValidacao getEstado() const;

    void adicionarMotivo(
        const std::string& motivo
    );

    void adicionarAlerta(
        const std::string& alerta
    );

    const std::vector<std::string>&
    getMotivos() const;

    const std::vector<std::string>&
    getAlertas() const;

    bool temMotivos() const;

    bool temAlertas() const;
};

#endif
