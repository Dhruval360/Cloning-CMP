#include<math.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdio_ext.h>
#include"cmp_server.h"

/*
Observations made: 
1) When a single file name is entered after cmp command in the shell, it never terminates if -n is not given. Hence I have handled this case by 
   prompting the user to enter the byte limit. 
2) When -n is entered multiple times it takes the least number of bytes.
3) When -i is entered multiple times it takes the largest individual skip values.
4) When a number is entered without being preceded by an option like -n or -i it is taken as skip initial byte value for the first file.

Known Bugs:
1) I couldn't understand what '--' alone signifies for cmp as sometimes I found that it just ignores it. So this hasn't been implemented properly.
2) Maximum bytes value gets capped at largest 64 bit number with no way of identifying any number larger than that to throw appropriate warning.
3) When a single filename is entered and when text input is taken from the user for comparison, if the user enters a large amount of byte limit, 
   segmentation fault could occur.

Yet to be implemented:
1) Take in numbers in any format other than base 10. (cmp supports octal, binary and hexadecimal byte value input as well).
2) Check for directories. (When a directory is entered in place of a filename, cmp can spot that and tells the user that it’s a directory).
3) Alignment of output in some cases.
4) KiB, GiB, etc. notations couldn't be added. (However the notations stated in the man page have been implemented).

Note: All outputs (including the help page and the version have been copied directly from the output screen) 

Now, the actual cmp in LINUX has a number of options and they are as follows:
 	
       -b, --print-bytes
              print differing bytes

       -i, --ignore-initial=SKIP
              skip first SKIP bytes of both inputs

       -i, --ignore-initial=SKIP1:SKIP2
              skip first SKIP1 bytes of FILE1 and first SKIP2 bytes of FILE2

       -l, --verbose
              output byte numbers and differing byte values

       -n, --bytes=LIMIT
              compare at most LIMIT bytes

       -s, --quiet, --silent
              suppress all normal output

       --help display this help and exit

       -v, --version
              output version information and exit
*/


int main(int argc , char *argv[]) { // This takes input directly from the terminal
/*
Here argc is the number of inputs (including ./a.out given in the terminal and argv contains all the individual string arrays given in the
terminal input). Hence, by default argv[0] will be the name of the executable form of this program i.e ./a.out or any other name we give to it.
*/
	// Setting indices of file names in argv
	int filename_indices[2] = {-1, -1}; 
	// I have just initialized the indices as -1 just to signify that they haven't been updated with the correct indices 
	
	// The following needs to be 64 bit as user can enter a max of Y (yotta byte)
	unsigned long long int limit = -1;  
	/*
	Limit for -n option. By default I have put it to its max (-1 in unsigned terms will be max 64 bit number as -ve numbers are
	represented as 2s compliments of their positive binary representation)
	*/
	unsigned long long int ignore_initial_value[2] = {0, 0}; // For -i option 
	int options_chosen[10] = {0}; // This can be used to keep track of all the options the user has chosen
	int filenames = 0; // Number of filenames entered
	int check = 0, same = 1; // The variable same is inverted if the two files are different	
	
	check = preprocess(argc, argv, ignore_initial_value, options_chosen, filename_indices, &filenames, &limit);
	
	if(check == no_errors) check = output(argv, filename_indices, ignore_initial_value, options_chosen, limit, &same, filenames); 	
	if(check) // Meaning something went wrong
		return 2; // As the original cmp command returns 2 for trouble
	else {
		if(same) return 0; // As original cmp command returns 0 if both files are same
		else return 1;     // As original cmp command returns 1 if the files are different
	}
}