#ifndef DAOENTRADA_H
#define DAOENTRADA_H

class DAOEntrada
{
public:
    virtual ~DAOEntrada() = default;

    virtual void processarArquivo() = 0;
};

#endif
