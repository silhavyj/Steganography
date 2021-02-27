#include <iostream>
#include "steganography.h"

int main(int argc, char *argv[]) {
    Steganography stg;
    switch (argc) {
        case 2: 
            stg.extract(argv[1]);
            break;
        case 3:
            stg.hide(argv[1], argv[2]);
            break;
        default:
            std::cout << "invalid number of arguments!\n";
            return 1;
    }
    return 0;
}