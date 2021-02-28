#include <fstream>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <vector>
#include <unistd.h>
#include <iomanip>
#include "steganography.h"

void Steganography::hide(const char *fileName1, const char *fileName2) {
    // check if the first file exists
    if (access(fileName1, F_OK ) == -1) {
        std::cout << "File '" << fileName1 << "' not found\n";
        return;
    }
    // check if the second file exists
    if (access(fileName2, F_OK ) == -1) {
        std::cout << "File '" << fileName1 << "' not found\n";
        return;
    }

    BMPFileHeader_t bmpFileHeader;
    BMPInfoHeader_t bmpInfoHeader;

    // open up both files (images)
    std::fstream file2{fileName2, std::ios::out | std::ios::in | std::ios::binary};
    std::fstream file1{fileName1, std::ios::binary | std::ios::in | std::ios::ate};

    uint32_t file1Size = file1.tellg(); // get the size of the file we're hiding
    file1.seekg(0);

    // read bmp header
    file2.read((char *)&bmpFileHeader, sizeof(BMPFileHeader_t));
    if (bmpFileHeader.fileType != BMP_MAGIC_NUM) {
        std::cout << "Invalid format of file '" << fileName2 << "'\n";
        return;
    }

    // read bmp info
    file2.read((char *)&bmpInfoHeader, sizeof(BMPInfoHeader_t));
    if (bmpInfoHeader.height < 0) {
        std::cout << "The program can treat only BMP images with the origin in the bottom left corner!\n";
        return;
    }

    uint8_t tag; // how many bits we'll modify per byte
    uint32_t dataSize = bmpInfoHeader.width * bmpInfoHeader.height * bmpInfoHeader.bitDepth / 8;

    // check how many bits we'll need to hide the image
    if (dataSize >= file1Size * 8 + TAG_SIZE) // we store 1 bit to 1 bytes + the tag 
        tag = 1;
    else if (dataSize >= file1Size * 4 + TAG_SIZE) // we store 2 bites to 1 bytes + the tag
         tag = 2;
    else if (dataSize >= file1Size * 2 + TAG_SIZE) // we store 4 bites to 1 bytes + the tag
        tag = 4;
    else {
        // the file is too small to hold the image in it
        std::cout << "File '" << fileName2 << "' is too small\n";
        return;
    }

#ifdef DEBUG
    std::cout << bmpFileHeader << "\n";
    std::cout << bmpInfoHeader << "\n";
#endif

#ifdef INFO
    std::cout << "Starting merging images.\n";
    std::cout << "Based on the size of the file, " << (int)tag;
    if (tag == 1)
        std::cout << " bit";
    else
        std::cout << " bits";
    std::cout << " will be modified.\n";
#endif

    // if all the above validation was okay
    // copy the image into 'merged_image.bmp' so the
    // one does not get overwitten merged_image
    file2.close();
    copyFile(fileName2, MERGED_FILE_NAME);

    uint8_t byte2;  // byte off file 2
    uint8_t byte1;  // byte off file 1
    uint8_t p = 8;  // "pointer" to the current bit withing a byte
    uint32_t hiddenBytes = 0; // how many bytes we've hidden so far

    int bytesPerPixel = bmpInfoHeader.bitDepth / 8;
    int rowPadding = getRowPadding(bmpInfoHeader.width, bytesPerPixel);

    // open up the copied file so we can modify it
    file2 = std::fstream{MERGED_FILE_NAME, std::ios::out | std::ios::in | std::ios::binary};

    // move to the first byte of the data of the image
    file2.seekg(bmpFileHeader.dataOffset);

    // take the first 3 bytes and store the tag - info of how many
    // bit we're about to change in each byte (so it could be used in extraction as well)
    for (uint8_t i = 0; i < TAG_SIZE; i++) {
        file2.read((char *)&byte2, sizeof(uint8_t));
        byte2 &= 0xFE;
        byte2 |= ((tag >> i) & 1);
        file2.seekg(-1, std::ios_base::cur);
        file2.write((char *)&byte2, sizeof(uint8_t));
    }

    for (int i = 0; i < bmpInfoHeader.height; i++) {
        for (int j = (i == 0 ? TAG_SIZE : 0); j < bmpInfoHeader.width; j++) {
            for (int k = 0; k < bytesPerPixel; k++) {
                // test if we just finished storing another byte
                if (p == 8) {
                    #ifdef INFO
                    showProgress(file1Size, hiddenBytes, PROGRESS_STEP);
                    #endif

                    // test if we're completely done
                    if (hiddenBytes == file1Size) {
                        #ifdef INFO
                        std::cout << "Done\n";
                        #endif
                        file1.close();
                        file2.close();
                        #ifdef DEBUG
                        generateDiff(fileName2, MERGED_FILE_NAME);
                        #endif
                        return;
                    }
                    // if we're not done. Read another byte off
                    // the file we're hiding in the image
                    p = 0;
                    file1.read((char *)&byte1, sizeof(uint8_t));
                    hiddenBytes++;
                }
                file2.read((char *)&byte2, sizeof(uint8_t));
                
                // we can either modify 1, 2, or 4 bits
                // of every byte (depends on the size)
                switch (tag) {
                    case 1:
                        byte2 &= 0xFE;
                        byte2 |= ((byte1 >> p) & 0x01);
                        break;
                    case 2:
                        byte2 &= 0xFC;
                        byte2 |= ((byte1 >> p) & 0x03);
                        break;
                    case 4:
                        byte2 &= 0xF0;
                        byte2 |= ((byte1 >> p) & 0x0F);
                        break;
                }
                p += tag;

                // after we change the byte,
                // store it back into the image again
                file2.seekg(-1, std::ios_base::cur);
                file2.write((char *)&byte2, sizeof(uint8_t));
            }
        }
        // skip the blank space (padding)
        for (int j = 0; j < rowPadding; j++)
            file2.read((char *)&byte2, sizeof(uint8_t));
    }
}

#ifdef INFO
    void Steganography::showProgress(uint32_t total, uint32_t st, uint32_t step) {
        static bool drawProgress = true;
        if (drawProgress) {
            std::cout << "progress:\n";
            for (int i = 1; i <= 100 / (int)step; i++)
                std::cout << std::setfill('0') << std::setw(2) << (i * step) << " ";
            std::cout << "[%]\n";
            drawProgress = false;
        }
        static int previous = 0;
        int percentage = (int)(((double)st / total) * 100);
        if (previous + (int)step == percentage) {
            std::cout << "## " << std::flush;
            previous = percentage;
            if (previous == 100)
                std::cout << "\n";
        }
    }
#endif

int Steganography::getRowPadding(int width, int bytesPerPixel) {
    int bytes = width * bytesPerPixel;
    if (bytes % ROW_BYTES_ALIGN == 0)
        return 0;
    return ROW_BYTES_ALIGN - (bytes % ROW_BYTES_ALIGN);
}

void Steganography::copyFile(const char *fileName1, const char *fileName2) {
    std::ifstream src(fileName1, std::ios::binary);
    std::ofstream dst(fileName2, std::ios::binary);
    dst << src.rdbuf();
}

void Steganography::extract(const char *fileName) {
    // test if the file exists
    if (access(fileName, F_OK ) == -1) {
        std::cout << "File '" << fileName << "' not found\n";
        return;
    }
    // copy the file image obr2_separated.bmp so
    // the original image will not be changed
    copyFile(fileName, SEPARATED_FILE_NAME_2);
    std::fstream file{SEPARATED_FILE_NAME_2, std::ios::out | std::ios::in | std::ios::binary};

    BMPFileHeader_t bmpFileHeader;
    BMPInfoHeader_t bmpInfoHeader;

    // read the BMP file header and test the magic number
    file.read((char *)&bmpFileHeader, sizeof(BMPFileHeader_t));
    if (bmpFileHeader.fileType != BMP_MAGIC_NUM) {
        std::cout << "Invalid format of file '" << SEPARATED_FILE_NAME_2 << "'\n";
        return;
    }
    // read the BMP file info
    file.read((char *)&bmpInfoHeader, sizeof(BMPInfoHeader_t));
    if (bmpInfoHeader.height < 0) {
        std::cout << "The program can treat only BMP images with the origin in the bottom left corner!\n";
        return;
    }

#ifdef DEBUG
    std::cout << bmpFileHeader << "\n";
    std::cout << bmpInfoHeader << "\n";
#endif

    uint32_t size = sizeof(BMPFileHeader_t);
    uint8_t tag = 0; // how many bits were changed when hiding the image
    uint8_t p = 0;   // current position within a byte
    uint8_t byte;    // byte read off the file   
    uint8_t byteOriginal = 0; // one byte of the hidden image

    int bytesPerPixel = bmpInfoHeader.bitDepth / 8;
    int rowPadding = getRowPadding(bmpInfoHeader.width, bytesPerPixel);

    std::vector<uint8_t> outputData; // content of the extracted image

    // move to the first byte of the data of the image
    file.seekg(bmpFileHeader.dataOffset);

    // read the tag off the first 3 bytes
    for (uint8_t i = 0; i < TAG_SIZE; i++) {
        file.read((char *)&byte, sizeof(uint8_t));
        tag |= (byte & 1) << i;
    }
    // validate the tag
    if (!(tag == 1 || tag == 2 || tag == 4)) {
        std::cout << "Invalid tag '" << (int)tag << "'\n";
        remove(SEPARATED_FILE_NAME_2);
        return;
    }

#ifdef INFO
    std::cout << "Starting extracting the original file.\n";
    std::cout << (int)tag;
    if (tag == 1)
        std::cout << " bit was used to hide the image.\n";
    else std::cout << " bits were used to hide the image.\n";
#endif

    for (int i = 0; i < bmpInfoHeader.height; i++) {
        for (int j = (i == 0 ? TAG_SIZE : 0); j < bmpInfoHeader.width; j++) {
            for (int k = 0; k < bytesPerPixel; k++) {
                // read a byte from the file
                file.read((char *)&byte, sizeof(uint8_t));
                // according to the tag, extract either 1,2 or 4 last
                // least significant bits and add it to the byte of 
                // the extracted image
                switch (tag) {
                    case 1:
                        byteOriginal |= (byte & 0x01) << p;
                        byte &= 0xFE;   // clear the last bit
                        break;
                    case 2:
                        byteOriginal |= (byte & 0x03) << p;
                        byte &= 0xFC;  // clear the last 2 bits
                        break;
                    case 4:
                        byteOriginal |= (byte & 0x0F) << p;
                        byte &= 0xF0;  // clear the last 4 bits
                        break;
                }
                p += tag; 

                // once we cleared the last bits,
                // store the byte back into the file
                file.seekg(-1, std::ios_base::cur);
                file.write((char *)&byte, sizeof(uint8_t));

                // test if we reached the end of a byte
                if (p == 8) {
                    outputData.push_back(byteOriginal);
                    byteOriginal = 0;
                    #ifdef INFO
                    if (size > sizeof(BMPFileHeader_t))
                        showProgress(size, outputData.size(), PROGRESS_STEP);
                    #endif
                    p = 0;
                    // test if we've read just enough bytes to read
                    // the header of the hidden image (we need the size)
                    if (outputData.size() == sizeof(BMPFileHeader_t)) {
                        if (((BMPFileHeader_t *)&outputData[0])->fileType != BMP_MAGIC_NUM) {
                            std::cout << "Esither there is no hidded image in '" << fileName << "' or it was damaged!\n";
                            remove(SEPARATED_FILE_NAME_2);
                            return;
                        }
                        size = ((BMPFileHeader_t *)&outputData[0])->fileSize;
                    }
                    // if we've reached the end of the hiddne image,
                    // store the content into a new file on the disk
                    if (size == (uint32_t)outputData.size()) {
                        std::ofstream output(SEPARATED_FILE_NAME_1, std::ios::binary);
                        output.write((const char *)&outputData[0], outputData.size());
                        #ifdef INFO
                        std::cout << "Done\n";
                        #endif
                        return;
                    }
                }
            }
        }
        // skip the blank space (padding)
        for (int j = 0; j < rowPadding; j++)
            file.read((char *)&byte, sizeof(uint8_t));
    }
}

#ifdef DEBUG
void Steganography::generateDiff(const char *fileName1, const char *fileName2) {
    std::cout << "Creating a difference between the two images\n";
    copyFile(fileName2, MERGED_DIFF_FILE_NAME);
    std::fstream file1{fileName1, std::ios::in | std::ios::binary};
    std::fstream file2{MERGED_DIFF_FILE_NAME, std::ios::out | std::ios::in | std::ios::binary};

    uint8_t byte1;
    uint8_t byte2;
    BMPFileHeader_t bmpFileHeader;
    BMPInfoHeader_t bmpInfoHeader;

    file1.read((char *)&bmpFileHeader, sizeof(BMPFileHeader_t));
    file1.read((char *)&bmpInfoHeader, sizeof(BMPInfoHeader_t));

    int bytesPerPixel = bmpInfoHeader.bitDepth / 8;
    int rowPadding = getRowPadding(bmpInfoHeader.width, bytesPerPixel);

    file1.seekg(bmpFileHeader.dataOffset);
    file2.seekg(bmpFileHeader.dataOffset);

    for (int i = 0; i < bmpInfoHeader.height; i++) {
        for (int j = 0; j < bmpInfoHeader.width; j++) {
            for (int k = 0; k < bytesPerPixel; k++) {
                file1.read((char *)&byte1, sizeof(uint8_t));
                file2.read((char *)&byte2, sizeof(uint8_t));
                byte2 = std::abs(byte1 - byte2);
                file2.seekg(-1, std::ios_base::cur);
                file2.write((char *)&byte2, sizeof(uint8_t));
            }
        }
         for (int j = 0; j < rowPadding; j++) {
            file1.read((char *)&byte1, sizeof(uint8_t));
            file2.read((char *)&byte2, sizeof(uint8_t));
         }
    }
}
#endif

#ifdef DEBUG
std::ostream& operator<<(std::ostream& out, const BMPFileHeader_t& bmpFileHeader) {
    out << "file type     : " << "0x" << std::hex << bmpFileHeader.fileType << std::dec << "\n";
    out << "file size [B] : " << bmpFileHeader.fileSize << "\n";
    out << "data offset   : " << bmpFileHeader.dataOffset << "\n";
    return out;
}

std::ostream& operator<<(std::ostream& out, const BMPInfoHeader_t& bmpInfoHeader) {
    out << "width         : " << bmpInfoHeader.width << "\n";
    out << "height        : " << bmpInfoHeader.height << "\n";
    out << "bit depth     : " << bmpInfoHeader.bitDepth << "\n";
    out << "size image    : " << bmpInfoHeader.sizeImage << "\n";
    return out;
}
#endif