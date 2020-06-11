#define min(a,b) a < b ? a : b // Macro to give minimum value amongst two numbers
#define max(a,b) a > b ? a : b // Macro to give maximum value amongst two numbers

#define no_errors 0
#define file_absent 1
#define invalid_option 2
#define missing_operand 3
#define invalid_use 4
#define invalid_byte_value -1
#define quiet -2
#define unrecognized_option -3
#define terminate -4
#define missing_argument -5

void help(); // Prints help page
void version(); // Prints version
void isproperignore(char*, int*, unsigned long long int[]); // To check for proper ignore initial arguments
void caret_notation1(char, char); // For printing output of type1 in caret notation (^J for \n and ^I for \t)
void caret_notation2(char, char, unsigned long long int);// For printing output of type2 in caret notation (^J for \n and ^I for \t)
int output(char*[], int[], unsigned long long int[], int[], unsigned long long int, int*, int); // This compares the two files and prints the output
int preprocess(int, char*[], unsigned long long int[], int[], int[], int*, unsigned long long int*); // This does all the preprocessing
int options_checker(int, int[], char*[], char*[], unsigned long long int*, unsigned long long int[]); // Will check which options are chosen
unsigned long long int isnumber(char[], int, int*); // To check if a string entered is a number or not starting from the index number entered