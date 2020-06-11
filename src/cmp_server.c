#include<math.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdio_ext.h>
#include"cmp_server.h"

int preprocess(int argc, char* argv[], unsigned long long int ignore_initial_value[], int options_chosen[], 
	           int filename_indices[], int *filenames, unsigned long long int *limit) {
	// Initializing all options
	char *options[10] = {"-b", "-i", "-l", "-n", "-s", "--help", "-v", "--version", "--silent", "--quiet"}; 
	int type = 1; // For varying output for warnings as per cmp
	
	if(argc == 1) {
		printf("cmp: missing operand after '%s'\ncmp: Try 'cmp --help' for more information.\n", argv[0]);
		return missing_operand;
	}
	// To know if user wants to compare input text with file or file with input text (which would change the orientation of the output)
	int blank_first = 0; 
	int out; // To catch output of the function used to know if user has entered an invalid option
		
	if(argc == 2) {
		if(argv[1][0] != '-') { // Which means its a filename 			
			FILE *check = fopen(argv[1], "r");				
			if(check == NULL) {		
				printf("cmp: %s:  No such file or directory\n", argv[1]); // Same warning as shown by actual cmp
				return file_absent;
			}
			else fclose(check);
			
			printf("Please enter byte limit\n"); 
			return terminate; // Original cmp never terminates. I added this to prevent that.
		}
		
		else { // Means some option has been selected
			if(((argv[1][1] == 'n') || (argv[1][1] == 'i')) && (strlen(argv[1]) == 2)) {				
				printf("cmp: option requires an argument -- '%c'\ncmp: Try 'cmp --help' for more information.\n", argv[1][1]);
				return missing_argument;
			}					
					
			out = options_checker(argc, options_chosen, options, argv, limit, ignore_initial_value);
			
			if(out == invalid_option)           return invalid_option;
			else if(out == unrecognized_option) return unrecognized_option;
			else if(out == terminate)           return terminate;
			else if(out == invalid_byte_value)  return invalid_byte_value;
			else if((!strcmp(argv[1], "--bytes")) || (!strcmp(argv[1], "--ignore-initial"))) {
				printf("cmp: option '%s' requires an argument\ncmp: Try 'cmp --help' for more information.\n", argv[1]);
				return missing_argument;
			}	
			else {
				printf("cmp: missing operand after '%s'\ncmp: Try 'cmp --help' for more information.\n", argv[1]);
				return missing_operand;
			}
		}
	}
	
	else { // When more than 2 arguments are entered
		int flag_for_n = 0, flag_for_i = 0; // To keep a track of finding -n and -i
		
		out = options_checker(argc, options_chosen, options, argv, limit, ignore_initial_value);
			
		if(out == invalid_option)           return invalid_option;			
		else if(out == unrecognized_option) return unrecognized_option;	
		else if(out == terminate)           return terminate;
		else if(out == invalid_byte_value)  return invalid_byte_value;
		else if((options_chosen[4] || options_chosen[8] || options_chosen[9]) && options_chosen[2]) {
				printf("cmp: options -l and -s are incompatible\ncmp: Try 'cmp --help' for more information.\n");
				return invalid_use;
		}
		
		char c = '\0'; // Later used for testing the first character of each string
		
		// Separating names of files from the options as the options can be anywhere in the set of arguments	
		
		int check_invalid = 0; // To check for invalid number of bytes
		unsigned long long int temporary = 0; // Needed to check if a number is entered
		
		// Loop starts from 1 as we need not worry about the 0th element in argv which will always be name of executable
		for(int i = 1; i<argc; i++) {
			if(flag_for_n) { // If -n had been entered
				temporary = isnumber(argv[i], 0, &check_invalid);
				
				if(check_invalid) {
					printf("cmp: invalid --bytes value '%s'\ncmp: Try 'cmp --help' for more information.\n", argv[i]);
					return invalid_byte_value;
				}
				else flag_for_n = 0; type = 1;
				
				*limit = min(temporary, *limit); // According to observation 2
			}
			
			else if(flag_for_i) { // If -i has been entered
				isproperignore(argv[i], &check_invalid, ignore_initial_value);
						
				if(check_invalid == invalid_byte_value) {
					printf("cmp: invalid --ignore-initial value '%s'\ncmp: Try 'cmp --help' for more information.\n", argv[i]);
					return invalid_byte_value;
				}		
				else flag_for_i = 0; type = 1;
			}
			
			else if((argv[i][0] == '-') && (strcmp(argv[i], "--")) && (strcmp(argv[i], "-"))) {
			// Which means that an option has been chosen
				if(((argv[i][1] == 'n') && (strlen(argv[i]) == 2)) || (!strcmp(argv[i], "--bytes"))) {
					flag_for_n = 1;
					if(!strcmp(argv[i], "--bytes"))	type = 0;
				}
				else if(((argv[i][1] == 'i') && (strlen(argv[i]) == 2)) || (!strcmp(argv[i], "--ignore-initial"))) {
					flag_for_i = 1;
					if(!strcmp(argv[i], "--ignore-initial")) type = 0;
				}
				// -s has least priority hence I have put it here and not in options checker
				else if((!strcmp("--silent", argv[i]))||
				   (!strcmp("-s", argv[i]))||
				   (!strcmp("--quiet", argv[i]))||
				   (!strcmp("--s", argv[i]))) return quiet;
			}
					
			else { // The string is not an option
				if(*filenames == 2) { // Then this element is considered as skip value for first file
					temporary = isnumber(argv[i], 0, &check_invalid);
				
					if(check_invalid) {
						printf("cmp: invalid --ignore-initial value '%s'\ncmp: Try 'cmp --help' for more information.\n", argv[i]);
						return invalid_byte_value;
					}
								
					ignore_initial_value[0] = max(temporary, ignore_initial_value[0]); // According to observation 3
				
				}
				else {
					if((!strcmp(argv[i], "-")) && ((*filenames) == 0)) blank_first = 1;
					filename_indices[*filenames] = i;
					*filenames = (*filenames)+1; // Incrementing count of filenames
				}
			}

/*
 I have structured the above part of the program as such to handle the special cases eg: "-n -n" wherein the second -n becomes an invalid 
 byte and many such similar cases. 
*/
		}
		
// These are to take care of cases when there are -n and -i options at the end of the argument list which don't have their arguments
		
		// As according to cmp manual, '-' for a filename implies text input must be taken from the standard input
		if(blank_first) *filenames = (*filenames)-1;
		
		if(flag_for_n || flag_for_i) {
			if(type) printf("cmp: option requires an argument -- '%s'\ncmp: Try 'cmp --help' for more information.\n", argv[argc-1]);
			else printf("cmp: option '%s' requires an argument\ncmp: Try 'cmp --help' for more information.\n", argv[argc-1]);
			return missing_argument;
		}
		
		switch(*filenames) { // Based on number of filenames entered
			case 1:
				if(!options_chosen[3]) { // Which means limit isn't mentioned
					printf("Please enter byte limit\n"); 
					// Original cmp never terminates. I added this to prevent that and segmentation errors
					return terminate;
				}
				// For swapping the ignore initial values based on whether '-' is entered before or after or not entered at all	
				unsigned long long int temp; 
				if(blank_first) {
					temp = ignore_initial_value[1];
					ignore_initial_value[1] = ignore_initial_value[0];
					ignore_initial_value[0] = temp; 
				}
				break;				
			
			case 2: break; // Do nothing here (ideal case)
				
			default:
				printf("cmp: missing operand after '%s'\ncmp: Try 'cmp --help' for more information.\n", argv[argc-1]); 
				// Which means no filenames given
				return missing_operand;
		}
	}
	return no_errors;
}

int output(char* argv[], int filename_indices[], unsigned long long int ignore_initial_value[], int options_chosen[], 
	   	   unsigned long long int limit, int *same, int filenames) {
	FILE *file1;
	char byte1 = '\0', byte2 = '\0';
	
	int blank_first = 0;
	
	unsigned long long int byte_count = 1, line = 1;
	
	// Opening the files
	int file1_index = filename_indices[0];
	
	if(!strcmp(argv[filename_indices[0]], "-")) { // Meaning the first file must be taken as text input from standard input
		file1_index = filename_indices[1];
		blank_first = 1;
	}
	file1 = fopen(argv[file1_index], "r");
	if(file1 == NULL) {
		printf("cmp: %s:  No such file or directory\n", argv[filename_indices[0]]); // Same warning as shown by actual cmp
		return file_absent;
	}
	
	if(filenames == 2) { // 2 filenames entered
		FILE *file2; 
	       	
		file2 = fopen(argv[filename_indices[1]], "r");
		if(file2 == NULL) {
			printf("cmp: %s:  No such file or directory\n", argv[filename_indices[1]]); // Same warning as shown by actual cmp
			return file_absent;
		}

		// Skipping the given number of bytes (by default 0)
		// I am not using fseek as it has a limit of 32 bit integral offset
		while((byte1 != EOF) && (ignore_initial_value[0])) {
			byte1 = fgetc(file1);
			ignore_initial_value[0]--;
		}
		if(byte1 == EOF) {
			printf("cmp: EOF on %s which is empty\n", argv[filename_indices[0]]);		
			return 0;
		}
		
		while((byte2 != EOF) && (ignore_initial_value[1])) {
			byte2 = fgetc(file2);
			ignore_initial_value[1]--;
		}
		if(byte2 == EOF) {
			printf("cmp: EOF on %s which is empty\n", argv[filename_indices[1]]);		
			return 0;
		}
			
		// Now comparison starts
		if(!(options_chosen[2])) { // -l is not chosen
			byte1 = fgetc(file1);
			byte2 = fgetc(file2);
			
			while((byte1 != EOF) && (byte2 != EOF) && (byte_count<=limit)) {	   
				if(byte1 != byte2) {
					*same = 0;
					break;
				}
		    	
				// Checking only one is enough as, if program reaches this point, it means that both bytes are same
				if(byte1 == '\n') ++line;
				
				byte_count++;
				byte1 = fgetc(file1);
				byte2 = fgetc(file2);
			}
			    		
			if(!(*same)) { // Which means the loop exitted due to dissimilarity
				printf("%s %s differ: byte %llu, line %llu", argv[filename_indices[0]], argv[filename_indices[1]],
						byte_count, line);
				
				if(options_chosen[0]) // -b is chosen
					caret_notation1(byte1, byte2);
					
				printf("\n");
			}
		
			else if(byte1 == byte2); 
			// If byte1 is still equal to byte2 then both files are same. Hence no output.
			
			else if(byte1 == EOF) {
				printf("cmp: EOF on %s after byte %llu, line %llu\n", argv[filename_indices[0]], byte_count - 1, line);		
				return 0;
			}
			
			else if(byte2 == EOF) {
				printf("cmp: EOF on %s after byte %llu, line %llu\n", argv[filename_indices[1]], byte_count - 1, line);		
				return 0;
			}
		}
		else { // -l is chosen
			byte1 = fgetc(file1);
			byte2 = fgetc(file2);

			while((byte1 != EOF) && (byte2 != EOF) && (byte_count<=limit)) {					   
				if(byte1 != byte2) {
					*same = 0;
					if(options_chosen[0]) caret_notation2(byte1, byte2, byte_count); // -b is chosen
					else printf("%2llu %3o %3o\n", byte_count, byte1, byte2); 
				}
				byte_count++;			    	
				byte1 = fgetc(file1);
				byte2 = fgetc(file2);
			}
			
			if(byte_count < limit) { // Meaning early exit of loop
				if(byte1 == EOF) {
					printf("cmp: EOF on %s after byte %llu\n", argv[filename_indices[0]], byte_count - 1);		
					return 0;
				}
				
				else if(byte2 == EOF) {
					printf("cmp: EOF on %s after byte %llu\n", argv[filename_indices[1]], byte_count - 1);		
					return 0;
				}
			}
		}
		fclose(file2);
	}
	
	else { // Case of comparing with text input from user
		char *buffer = (char *)malloc(sizeof(char)*(limit+ignore_initial_value[1])); 
				
		for(unsigned long long int i = 0; i<(limit+ignore_initial_value[1]); i++) buffer[i] = fgetc(stdin); // Taking user string input
					
		while((byte1 != EOF) && (ignore_initial_value[0])) {
			byte1 = fgetc(file1);
			ignore_initial_value[0]--;
		}
		
		if(byte1 == EOF) {
			printf("cmp: EOF on %s which is empty\n", argv[file1_index]);		
			return 0;
		}
		
		// Now comparison starts
		if(!(options_chosen[2])) { // -l is not chosen
			byte1 = fgetc(file1);
			byte2 = buffer[ignore_initial_value[1]];
			char temp; // To know previous state of byte1 for accurate line number readings
			
			while((byte1 != EOF) && (byte_count<=limit)) {   
				if(byte1 != byte2) {
					*same = 0;
					break;
				}
			    	
			    	// Checking only one is enough as if program reaches this point, it means that both bytes are same
				if(byte1 == '\n') ++line;
				
				byte_count++;
				temp = byte1;
				byte1 = fgetc(file1);
				byte2 = buffer[ignore_initial_value[1] + byte_count - 1];		    	
			}
			
			if(byte_count > limit); // This means there was no differences in the given range, hence no output
			
			else if(byte1 == EOF) {
				if(temp == '\n') line--;
				printf("cmp: EOF on %s after byte %llu, line %llu\n", argv[file1_index], byte_count-1, line);		
				return 0;
			}
			
			else if(blank_first) {
				printf("- %s differ: byte %llu, line %llu", argv[file1_index], byte_count, line);
				if(options_chosen[0]) caret_notation1(byte2, byte1); // -b is chosen
				printf("\n");
			}
				
			else {
				printf("%s - differ: byte %llu, line %llu", argv[file1_index], byte_count, line);
				if(options_chosen[0]) caret_notation1(byte1, byte2); // -b is chosen
				printf("\n");
			}
		}
		else { // -l is chosen
			byte1 = fgetc(file1);
			byte2 = buffer[ignore_initial_value[1]];
			
			while((byte1 != EOF) && (byte_count<=limit)) {	   
				if(byte1 != byte2) {
					*same = 0;
					if(options_chosen[0]) { // -b is chosen
						if(blank_first) caret_notation2(byte2, byte1, byte_count);
						else caret_notation2(byte1, byte2, byte_count);
					}
					else {
						if(blank_first) printf("%llu %3o %3o\n", byte_count, byte2, byte1);
						else printf("%llu %3o %3o\n", byte_count, byte1, byte2);
					}
				};
				
				byte_count++;
			    byte1 = fgetc(file1);
				byte2 = buffer[ignore_initial_value[1] + byte_count - 1];
			}
			
			if(byte_count > limit); // This means there was no differences in the given range, hence no output 
	
			else if(byte_count <= limit) { // Meaning early exit of loop
				if(byte1 == EOF) {
					printf("cmp: EOF on %s after byte %llu\n", argv[file1_index], byte_count-1);		
					return 0;
				}
			}
		}
		free(buffer);
	}
	return no_errors;
}


int options_checker(int argc, int options_chosen[], char *options[], char *argv[], unsigned long long int *limit, 
			unsigned long long int ignore_initial_value[]) {	
	int j = 1; // Used only for the loop
	char *options2[4] = {"--print-bytes", "--ignore-initial", "--verbose", "--bytes"}; // Another set of options
	int check_invalid = 0;
	unsigned long long int temporary = 0; //These are used for -i and -n
		
	while(j < argc) {
		int flag = 0; // To check if an option matches 	
		
		if(argv[j][0] == '-') {	
			if(!strcmp("--help", argv[j])) {
				help();
				return terminate;
			}
			else if((!strcmp(argv[j], "--")) || (!strcmp(argv[j], "-"))); // Needs to ignore these
			else if((!strcmp("-v", argv[j])) || (!strcmp("--version", argv[j]))) {
				version();
				return terminate;
			}
			else if ((argv[j][1] == 'n') && (strlen(argv[j]) > 2)) {
				char *num = (char*)malloc(sizeof(char)*(strlen(argv[j]) - 2));
				num = strcpy(num, argv[j]+2); // This will now hold only the number
				
				temporary = isnumber(num, 0, &check_invalid);
				
				if(check_invalid) {
					printf("cmp: invalid --bytes value '%s'\ncmp: Try 'cmp --help' for more information.\n", 
							num);
					free(num);
					return invalid_byte_value;
				}

				free(num);
				*limit = min(temporary, *limit); // According to observation 2
				options_chosen[3]++;
			}
			else if ((strlen(argv[j]) >= 8) && (!strncmp(argv[j], "--bytes=", 8))) {
				char *num = (char*)malloc(sizeof(char)*(strlen(argv[j]) - 8));
				num = strcpy(num, argv[j]+8); // This will now hold only the number
				
				if(strlen(argv[j]) > 8) temporary = isnumber(num, 0, &check_invalid);
				else check_invalid = invalid_byte_value;
						
				if(check_invalid) {
					printf("cmp: invalid --bytes value '%s'\ncmp: Try 'cmp --help' for more information.\n", num);
					free(num);
					return invalid_byte_value;
				}

				free(num);
				*limit = min(temporary, *limit); // According to observation 2
				options_chosen[3]++;
			}
			else if((argv[j][1] == 'i') && (strlen(argv[j]) > 2)) {
				char *ignores = (char*)malloc(sizeof(char)*(strlen(argv[j] - 2)));
				ignores = strcpy(ignores, argv[j]+2); // This will now hold only the number:number part

				isproperignore(ignores, &check_invalid, ignore_initial_value);
							
				if(check_invalid == invalid_byte_value) {		
					printf("cmp: invalid --ignore-initial value '%s'\ncmp: Try 'cmp --help' for more information.\n",
					        ignores);
					free(ignores);
					return invalid_byte_value;
				}

				options_chosen[1]++;
				free(ignores);	
			}
			else if((strlen(argv[j]) >= 17) && (!strncmp(argv[j], "--ignore-initial=", 17))) {
				char *ignores = (char*)malloc(sizeof(char)*(strlen(argv[j] - 17)));
				ignores = strcpy(ignores, argv[j]+17); // This will now hold only the number:number part
				
				if(strlen(argv[j])>17)	isproperignore(ignores, &check_invalid, ignore_initial_value);
				else check_invalid = invalid_byte_value;
								
				if(check_invalid == invalid_byte_value) {	
					printf("cmp: invalid --ignore-initial value '%s'\ncmp: Try 'cmp --help' for more information.\n", ignores);
					free(ignores);
					return invalid_byte_value;
				}

				options_chosen[1]++;
				free(ignores);	
			}
			
			else {			
				for(int i = 0; i<10; i++) {
					if(!strcmp(options[i], argv[j])) { // To check which option the user has chosen
						options_chosen[i]++; // The user can enter same option multiple times in the real cmp
						flag = 1;
					}
					if(flag) break;
				}
				
				for(int i = 0; i<4; i++) {
					if(!strcmp(options2[i], argv[j])) { // To check which option the user has chosen
						options_chosen[i]++; // The user can enter same option multiple times in the real cmp
						flag = 1;
					}
					if(flag) break;
				}
				
				if(!flag) { // Which means invalid option has been entered
					if(argv[j][1] == '-') { // This means that the option has "--" in the beginning
						printf("cmp: unrecognized option '%s'\ncmp: Try 'cmp --help' for more information.\n", argv[j]);
						return unrecognized_option;
					}
					else {
						printf("cmp: invalid option -- '%c'\ncmp: Try 'cmp --help' for more information.\n", argv[j][1]);
						return invalid_option;
					}
				}
			}
		}
		j++;
	}
	
	return 0;
}
	
unsigned long long int isnumber(char string[], int start, int *check_invalid) {
					// Max byte value = 18446744073709551615 (largest 64 bit number)
					//i.e, 0b1111111111111111111111111111111111111111111111111111111111111111
	char c = string[start];
	char last = string[strlen(string) - 1];
	
	// Now for handling the suffixes k, KB, M, MB, etc.
	char multipliers[] = "kKMGTPE"; // cmp doesnt take in Z and Y though it says so in the man page
	unsigned long long int multiplier = 1;
	int multiplier_count = 0; 
	
	if(c == '-') { // As negative byte value is not allowed
		*check_invalid = invalid_byte_value;
		return invalid_byte_value;
	}
	else if(c == '+') start++;
	if(last == 'B') { // Which means that the suffix is two letters like kB, MB, GB, etc.
		int power = strchr(multipliers, string[strlen(string) - 2]) - multipliers; 
		// This gives index of the first letter of the suffix (k, M, G, etc.) as in the character array multipliers 
		
		if(power<0) { // Which means the first letter of the suffix is invalid
			*check_invalid = invalid_byte_value;
			return invalid_byte_value;
		}
		else if(power == 0 || power == 1) multiplier = 1000;  
		else multiplier = (unsigned long long int)powl(10, 3*power); //Might have to write own function
		
		multiplier_count = 2;
	}
		
	else if(last > '9') { // Which means single letter suffixes are present
		int power = strchr(multipliers, last) - multipliers;
		
		if(power<0) {
			*check_invalid = invalid_byte_value;
			return invalid_byte_value;
		}
		else if(power == 0 || power == 1) multiplier = 1024;  
		else multiplier = (unsigned long long int)powl(1024, power); //Might have to write own function

		multiplier_count = 1;
	}
	
	int number_of_digits = strlen(string) - start - multiplier_count;	
	
	char *number = (char*)calloc(number_of_digits, sizeof(char));

	for(int i = start; i<(strlen(string) - multiplier_count); i++) {
		c = string[i];

		if (c>'9' || c<'0') {
			*check_invalid = invalid_byte_value;
			return invalid_byte_value;	
		}
		else number[i] = c;
	}
	
	unsigned long long int return_value = (unsigned long long int)atoll(number) * multiplier;
	unsigned long long int max = 0xffffffffffffffff;
	free(number);

	if(return_value > max) {
		*check_invalid = invalid_byte_value;
		return invalid_byte_value;
	}
	else 	
		return return_value;
}	

void isproperignore(char string[], int *check_invalid, unsigned long long int ignore_initial_value[]) {
	unsigned long long int temporary; // For checking observation 3
		
	if(strchr(string, '-') != NULL) { // Negative numbers aren't allowed
		*check_invalid = invalid_byte_value;
		return;
	}
	char *check = strchr(string, ':');
	int split_position = 0;

	// eg of input +45k:+546
	if(check != NULL) split_position = strchr(string, ':') - string; // Gives the position of ':' in the argument
	
	int start = 0; // Needed for using the function isnumber	
	
	if(split_position > 0) { // Which means that the argument has ':'
		char *num1 = (char*)calloc(split_position, sizeof(char));					
		num1 = strncpy(num1, string, split_position);
		
		char *num2 = (char*)calloc(strlen(string) - split_position - 1, sizeof(char));
		num2 = strcpy(num2, string + split_position + 1); 

		if(num1[0] == '+')
			start = 1;
		
		temporary = isnumber(num1, start, check_invalid);
		if(*check_invalid) {
			free(num1);
			free(num2);
			return;
		}
		ignore_initial_value[0] = max(ignore_initial_value[0], temporary); 
		
		
		if(num2[0] != '+') start = 0;
			
		temporary = isnumber(num2, start, check_invalid);		
		if(*check_invalid) {
			free(num1);
			free(num2);
			return;
		}
		ignore_initial_value[1] = max(ignore_initial_value[1], temporary);

		free(num1);
		free(num2);
	}
	
	else { // Only 1 value is given implies both skip1 and skip2 are the same
		temporary = isnumber(string, start, check_invalid);
		if(check_invalid) return;
		else {
			ignore_initial_value[0] = max(temporary, ignore_initial_value[0]);
			ignore_initial_value[1] = ignore_initial_value[0];
		}
	}
	return;
}

void caret_notation1(char byte1, char byte2) {
	int emac_count = -1; // For caret notation
	char byte1_temp = byte1, byte2_temp = byte2;
					
	if(byte1 == '\n') {
		emac_count++;
		byte1_temp = 'J';
	}
	else if(byte1 == '\t') {
		emac_count++;
		byte1_temp = 'I';
	}
	
	if(byte2 == '\n') {
		emac_count += 2;
		byte2_temp = 'J';
	}
	else if(byte2 == '\t') {
		emac_count += 2;
		byte2_temp = 'I';
	}
		
	switch(emac_count) {
		case 0:
			printf(" is %3o ^%c %3o %c", byte1, byte1_temp, byte2, byte2_temp);
			break;
		
		case 1:
			printf(" is %3o %c  %3o ^%c", byte1, byte1_temp, byte2, byte2_temp);
			break;
			
		case 2:
			printf(" is %3o ^%c %3o ^%c", byte1, byte1_temp, byte2, byte2_temp);
			break;
		
		default: printf(" is %3o %c  %3o %c", byte1, byte1_temp, byte2, byte2_temp);
	}
}

void caret_notation2(char byte1, char byte2, unsigned long long int byte_count) {						
	int emac_count = -1; // For caret notation
	char byte1_temp = byte1, byte2_temp = byte2;
		
	if(byte1 == '\n') {
		emac_count++;
		byte1_temp = 'J';
	}
	else if(byte1 == '\t') {
		emac_count++;
		byte1_temp = 'I';
	}
						
	if(byte2 == '\n') {
		emac_count += 2;
		byte2_temp = 'J';
	}
	else if(byte2 == '\t') {
		emac_count += 2;
		byte2_temp = 'I';
	}
						
	switch(emac_count) {
		case 0:
			printf("%2llu %3o ^%c   %3o %c\n", byte_count, byte1, byte1_temp, byte2, byte2_temp);
			break;
						
		case 1:
			printf("%2llu %3o %c    %3o ^%c\n", byte_count, byte1, byte1_temp, byte2, byte2_temp);
			break;
							
		case 2:
			printf("%2llu %3o ^%c   %3o ^%c\n", byte_count, byte1, byte1_temp, byte2, byte2_temp);
			break;
		
		default:printf("%2llu %3o %c    %3o %c\n", byte_count, byte1, byte1, byte2, byte2);
	}
}

void help() {
	printf("Usage: cmp [OPTION]... FILE1 [FILE2 [SKIP1 [SKIP2]]]\nCompare two files byte by byte.\n\nThe optional SKIP1 and SKIP2 specify the number of bytes to skip\nat the beginning of each file (zero by default).\n\nMandatory arguments to long options are mandatory for short options too.\n  -b, --print-bytes          print differing bytes\n  -i, --ignore-initial=SKIP         skip first SKIP bytes of both inputs\n  -i, --ignore-initial=SKIP1:SKIP2  skip first SKIP1 bytes of FILE1 and\n                                      first SKIP2 bytes of FILE2\n  -l, --verbose              output byte numbers and differing byte values\n  -n, --bytes=LIMIT          compare at most LIMIT bytes\n  -s, --quiet, --silent      suppress all normal output\n      --help                 display this help and exit\n  -v, --version              output version information and exit\n\nSKIP values may be followed by the following multiplicative suffixes:\nkB 1000, K 1024, MB 1,000,000, M 1,048,576,\nGB 1,000,000,000, G 1,073,741,824, and so on for T, P, E, Z, Y.\n\nIf a FILE is '-' or missing, read standard input.\nExit status is 0 if inputs are the same, 1 if different, 2 if trouble.\n\nReport bugs to: bug-diffutils@gnu.org\nGNU diffutils home page: <http://www.gnu.org/software/diffutils/>\nGeneral help using GNU software: <http://www.gnu.org/gethelp/>\n");
}

void version() {
	printf("This is an attempt to replicate:\n\ncmp (GNU diffutils) 3.6\nCopyright (C) 2017 Free Software Foundation, Inc.\nLicense GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\nWritten by Torbjörn Granlund and David MacKenzie.\n");	
}