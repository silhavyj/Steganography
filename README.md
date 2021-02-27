# KIV/BIT task 01 - Steganography!

# manual

## Compilation
The compilation process is done thorough the `make`command that's supposed to be executed in the root folder of the project structure. Once the process has completed, a file called `stg` will be generated. This file represents the executable file of the application. 
## Execution
### help
`./stg --help`
This command prints out a short info about how the program is meant to be treated.
### hiding files
`./stg <file1> <file2>`
This command hides file1 into file2 using a steganography technique described down below. If everything goes well, a file called `merged_image.bmp` will be produced. This file is essentially file2 having file1 buried in it.
### extracting the original file
`./stg <file1>`
