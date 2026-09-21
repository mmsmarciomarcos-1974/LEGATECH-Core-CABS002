#include <iostream>

int main()
{
    std::cout << "LEGATECH Core - CABS002\n";
    std::cout << "Teste inicial do ambiente C++\n\n";

    std::cout << "Valor de __cplusplus: "
              << __cplusplus
              << '\n';

#if __cplusplus >= 202100L
    std::cout << "Status: C++23 ativo.\n";
#else
    std::cout << "ATENCAO: C++23 nao esta ativo.\n";
#endif

    return 0;
}
