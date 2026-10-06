#ifndef SQLITEDAOENTRADA_H
#define SQLITEDAOENTRADA_H

#include <string>
#include "DAOEntrada.h"
#include "CatalogoTuss.h"

class SQLiteDAOEntrada : public DAOEntrada
{
private:
    std::string caminhoBanco;
    const CatalogoTuss* catalogo;
    std::string competenciaArquivo;

public:
    SQLiteDAOEntrada(const std::string& caminhoBanco,
                     const CatalogoTuss* catalogo = nullptr,
                     const std::string& competenciaArquivo = "");

    void processarArquivo() override;
};

#endif
