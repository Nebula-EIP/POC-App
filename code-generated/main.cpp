#include <iostream>
int main() {
    int x = 5;
    std::cout << "H;ello, {World}!" << std::endl;
    if (x > 0) {
        std::cout << "x is positive" << std::endl;
        return 1;
    }
    else {
        std::cout << "x is not positive" << std::endl;
        return 0;
    }
}
