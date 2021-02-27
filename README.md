# KIV/BIT task 01 - Steganography!

# manual

## Compilation
The compilation process is done thorough the `make`command that's supposed to be executed in the root folder of the project structure. Once the process has completed, a file called `stg` will be generated. This file represents the executable file of the application. 
## Execution
### help
```bash
./stg --help
```
This command prints out a short info about how the program is meant to be treated.
### hiding files
```bash
./stg <file1> <file2>
```
This command hides file1 into file2 using a steganography technique described down below. If everything goes well, a file called `merged_image.bmp` will be produced. This file is essentially file2 having file1 buried in it.
#### generating a file showing the difference
If required, definition `#define DEBUG` can be added to `steganography.h`. Having this enabled, a file called `merged_image_diff.bmp` will be generated along with the file `merged_image.bmp`. The content of this file is the difference between the original image and the image already holding a picture in it.

### extracting the original file
```bash
./stg <file1>
```
Parameter file1 represents a file having another image hidden in it. This could be the file mentioned in the section above, `merged_image.bmp`. Upon successful extraction, a file called `obr1_separated.bmp` will be generated. This image should not differ from the original image being hidden into the "wrapper" image. To test this out, we can use the `cmp` command on a UNIX machine.

## Testing
For testing purposes, there's a script `test/test_01.sh`, which automatically tests the functionality of the program using the images provided along with the task itself.

# Steganography technique
## changing the least significant bits
Depending on the file size ratio, either 1, 2, or 4 last bits of a byte will be modified. The program will check the size of the file we want to hide, and if it turns out that changing the least significant bit will be enough to hide the entire file, it will do so. If not, it will use the same approach, but this time, with the last two least significant bits. If this is still enough, it will try to use the last four least significant bits. If none of these options works, the program will print out an error message saying the file is not big enough to hold a file of this size.
