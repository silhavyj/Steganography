#ifndef _STEGANOGRAPHY_H_
#define _STEGANOGRAPHY_H_

#include <cstdint>
#include <iostream>

#define BMP_MAGIC_NUM 0x4D42
#define ROW_BYTES_ALIGN 4 // each row must start at an address aligned to 4 bytes
#define TAG_SIZE        3 // we use 3 bytes to store the tag (how many bits we change in terms of LSB)

#define MERGED_FILE_NAME      "merged_image.bmp"
#define SEPARATED_FILE_NAME_1 "obr1_separated.bmp"
#define SEPARATED_FILE_NAME_2 "obr2_separated.bmp"

// for debuggin
// (printing additional info etc)
//#define DEBUG
#ifdef DEBUG
# define MERGED_DIFF_FILE_NAME "merged_image_diff.bmp"
#endif

// enable printing out 
// (verbose)
#define INFO
#ifdef INFO
# define PROGRESS_STEP 10
#endif

struct BMPFileHeader_t {
    uint16_t fileType;   // "magic" number of the bmp format (can be used for validation)
    uint32_t fileSize;   // size of the file
    uint16_t reserver1;
    uint16_t reserver2;
    uint32_t dataOffset; // offset in bytes where the actual data starts
} __attribute__((packed));

struct BMPInfoHeader_t {
    uint32_t size;
    int32_t width;       // width of the image
    int32_t height;      // hight of the image
    uint16_t planes;
    uint16_t bitDepth;   // number of bytes per pixel
    uint32_t compression;
    uint32_t sizeImage;
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
} __attribute__((packed));

class Steganography {
private:
    void copyFile(const char *fileName1, const char *fileName2);
    int getRowPadding(int width, int bytesPerPixel);

public:
    void hide(const char *fileName1, const char *fileName2);
    void extract(const char *fileName);
#ifdef DEBUG
    void generateDiff(const char *fileName1, const char *fileName2);
#endif
#ifdef INFO
    void showProgress(uint32_t total, uint32_t st, uint32_t step);
#endif
};

#ifdef DEBUG
std::ostream& operator<<(std::ostream& out, const BMPFileHeader_t& bmpFileHeader);
std::ostream& operator<<(std::ostream& out, const BMPInfoHeader_t& bmpInfoHeader);
#endif

#endif