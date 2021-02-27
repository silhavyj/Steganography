#include <iostream>
#include <cstring>
#include "steganography.h"

void printHelp() {
    std::cout << "hidding images: ./stg <file1> <file2>\n";
    std::cout << "<file1> is the file to be hidden into <file2>\n";
    std::cout << "image 'merged_image.bmp' will be generated as a result of merging the files\n\n";

    std::cout << "extracting image: ./stg merged_image.bmp\n";
    std::cout << "image 'obr1_separated.bmp' will be produced matching the original image\n";
}

int main(int argc, char *argv[]) {
    Steganography stg;
    switch (argc) {
        case 2:
            if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-help") == 0)
                printHelp();
            else
                stg.extract(argv[1]);
            break;
        case 3:
            stg.hide(argv[1], argv[2]);
            break;
        default:
            std::cout << "Invalid number of arguments!\n";
            printHelp();
            return 1;
    }
    return 0;
}