#include <iostream>
#include <string>

#include "diff.hpp"

int main() {
    Diff a;

    auto lhs = std::string("goore wormen");
    auto rhs = std::string("guur feesten");

    std::cout << lhs << std::endl;
    std::cout << rhs << std::endl;

    auto v = a.diff(lhs, rhs);

    for (auto p : v) {
        std::cout << p << std::endl;
    }

    system("pause");
}