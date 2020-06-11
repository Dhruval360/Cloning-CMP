# Cloning-CMP
This is a C program that clones the CMP command in LINUX.  

# Project Structure
```
Cloning-CMP
├── Makefile
├── Project-Report.pdf
├── README.md
└── src
    ├── cmp_client.c [Contains the main function]
    ├── cmp_server.c [Contains the function implementations]
    └── cmp_server.h [Contains the function declarations]
```

# Dependencies
A C compiler like gcc or clang

# Compiling and running
Compile using the make utility:
```
$ make CMP -j2
```

Run the following command to print the help doc for the program:
```
$ ./build/bin/CMP --help
```