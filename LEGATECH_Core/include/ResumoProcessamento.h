#ifndef RESUMOPROCESSAMENTO_H
#define RESUMOPROCESSAMENTO_H

#include <cstddef>

class ResumoProcessamento
{
private:
    std::size_t total;
    std::size_t validos;
    std::size_t validosComAlerta;
    std::size_t rejeitados;
    std::size_t estruturaDivergente;

public:
    ResumoProcessamento();

    void incrementarTotal();
    void incrementarValidos();
    void incrementarValidosComAlerta();
    void incrementarRejeitados();
    void incrementarEstruturaDivergente();

    std::size_t getTotal() const;
    std::size_t getValidos() const;
    std::size_t getValidosComAlerta() const;
    std::size_t getRejeitados() const;
    std::size_t getEstruturaDivergente() const;
};

#endif
