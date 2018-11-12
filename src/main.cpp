#include <iostream>

#include "diff.hpp"

int main() {
    auto lhs = std::string("goore wormen");
    auto rhs = std::string("guur feesten");

    std::cout << lhs << std::endl;
    std::cout << rhs << std::endl;

    auto v = diff::diff(lhs, rhs);

    for (auto p : v) {
        std::cout << p << std::endl;
    }

    // system("pause");

    while (true) { }

    int i = 10;
    (void) i;
}