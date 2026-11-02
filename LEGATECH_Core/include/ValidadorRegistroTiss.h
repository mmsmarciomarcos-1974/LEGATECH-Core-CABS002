#ifndef VALIDADORREGISTROTISS_H
#define VALIDADORREGISTROTISS_H

#include <string>
#include <array>
#include <set>
#include "CatalogoTuss.h"
#include "RegistroTissBruto.h"
#include "ResultadoValidacao.h"

class ValidadorRegistroTiss
{
    const CatalogoTuss* catalogo; // Deve sobreviver ao validador.
    std::set<std::array<std::string, 11>> registrosVistos;
public:
    explicit ValidadorRegistroTiss(const CatalogoTuss* catalogo = nullptr)
        : catalogo(catalogo) {}

    // Validacao sem contexto externo.
    ResultadoValidacao validar(
        const RegistroTissBruto& registro
    );

    // Validacao considerando a competencia
    // identificada no arquivo.
    ResultadoValidacao validar(
        const RegistroTissBruto& registro,
        const std::string& competenciaArquivo
    );
};

#endif
