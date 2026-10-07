#ifndef SQLITEDAOENTRADA_H
#define SQLITEDAOENTRADA_H

#include <string>
#include "DAOEntrada.h"
#include "CatalogoTuss.h"
#include "AuditoriaValidacao.h"

class SQLiteDAOEntrada : public DAOEntrada
{
private:
    std::string caminhoBanco;
    const CatalogoTuss* catalogo;
    std::string competenciaArquivo;
    AuditoriaValidacao* auditoria;

public:
    SQLiteDAOEntrada(const std::string& caminhoBanco,
                     const CatalogoTuss* catalogo = nullptr,
                     const std::string& competenciaArquivo = "",
                     AuditoriaValidacao* auditoria = nullptr);

    void processarArquivo() override;
};

#endif
