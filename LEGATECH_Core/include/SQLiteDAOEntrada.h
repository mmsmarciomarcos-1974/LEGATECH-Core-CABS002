#ifndef SQLITEDAOENTRADA_H
#define SQLITEDAOENTRADA_H

#include <string>
#include "DAOEntrada.h"

class SQLiteDAOEntrada : public DAOEntrada
{
private:
    std::string caminhoBanco;

public:
    SQLiteDAOEntrada(const std::string& caminhoBanco);

    void processarArquivo() override;
};

#endif
