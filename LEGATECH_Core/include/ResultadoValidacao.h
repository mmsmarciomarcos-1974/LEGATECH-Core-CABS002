#ifndef RESULTADOVALIDACAO_H
#define RESULTADOVALIDACAO_H

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

public:
    ResultadoValidacao(EstadoValidacao estado);

    EstadoValidacao getEstado() const;
};

#endif
