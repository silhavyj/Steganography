
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

Parameter file1 represents a file having another image hidden in it. This could be the file mentioned in the section above, `merged_image.bmp`. Upon successful extraction, a file called `obr1_separated.bmp` will be generated. This image should not differ from the original image being hidden in the "wrapper" image. To test this out, we can use the `cmp` command on a UNIX machine. Besides this image, there will be another file, `obr2_separated.bmp`, generated, which represents the file used to hide an image in it. However, after this process, there's no image hidden in it anymore.

 
## Testing

For testing purposes, there's a script `test/test_01.sh`, which automatically tests the functionality of the program using the images provided along with the task itself.

  

# Steganography technique

## changing the least significant bits

Depending on the file size ratio, either **1, 2, or 4 last bits of a byte will be modified**. The program will check the size of the file we want to hide, and if it turns out that changing only the least significant bit will be enough to hide the entire file, it will do so. If not, it will use the same approach, but this time, with the last two least significant bits. If this is still not enough, it will try to use the last four least significant bits. **If none of these options works, the program will print out an error message saying the file is not big enough to hold an image of this size**.

In order to be able to yank out the original image afterwards, **we need to store the information about how many bits we have modified into the final merged image as well**. To do so, we use the least significant bits of **the first three bytes** of the data array of the image. When it comes to separating the images, we'll first read these three bits, and then we'll continue reading the actually data accordingly.

# Implementation
In order to be able to process large files, I've decided not to load the content of the files into memory directly. Instead, I used a streaming approach and took advantage of `std::fstream`. This downside of this, though, is that it's rather slow, and as a result, it takes a couple of seconds for the program to finish the process.
To make this a bit more user-friendly, there is a simple progress bar showing the user how many percent  are left. 

### process of merging two images (1)
```bash
./stg images/dwarf.bmp images/weber.bmp
Starting merging images.
Based on the size of the file, 4 bits will be modified.
progress:
10 20 30 40 50 60 70 80 90 100 [%]
## ## ## ## ## ## 
```
### process of merging two images (2)
```bash
./stg images/dwarf.bmp images/weber.bmp
Starting merging images.
Based on the size of the file, 4 bits will be modified.
progress:
10 20 30 40 50 60 70 80 90 100 [%]
## ## ## ## ## ## ## ## ## ##
Done
```

### process of separating two images
```bash
Starting extracting the original file.
4 bits were used to hide the image.
progress:
10 20 30 40 50 60 70 80 90 100 [%]
## ## ## ## ## ## ## ## ## ## 
Done
```
