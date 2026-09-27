#ifndef SQLITEDAOSAIDA_H
#define SQLITEDAOSAIDA_H

#include <string>
#include "DAOSaida.h"

class SQLiteDAOSaida : public DAOSaida
{
private:
    std::string caminhoBanco;

public:
    SQLiteDAOSaida(
        const std::string& caminhoBanco
    );

    void gravar(
        const RegistroTissBruto& registro
    ) override;
};

#endif
