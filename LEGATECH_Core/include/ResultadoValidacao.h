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

public:
    ResultadoValidacao(
        EstadoValidacao estado
    );

    EstadoValidacao getEstado() const;

    void adicionarMotivo(
        const std::string& motivo
    );

    const std::vector<std::string>& getMotivos() const;

    bool temMotivos() const;
};

#endif
