#include <iostream>
#include "../include/RegistroTissBruto.h"

int main()
{
    RegistroTissBruto registro(
        "6450733769",
        "PR",
        "2025-01",
        "10101012",
        "22",
        "1",
        "200",
        "0",
        "",
        "0",
        "0"
    );

    std::cout << "LEGATECH Core - CABS002\n\n";

    std::cout << "ID Evento: "
              << registro.getIdEventoAtencaoSaude()
              << '\n';

    std::cout << "UF Prestador: "
              << registro.getUfPrestador()
              << '\n';

    std::cout << "Data Realizacao: "
              << registro.getDtRealizacao()
              << '\n';

    return 0;
}
