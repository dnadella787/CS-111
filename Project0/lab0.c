/**
 * NAME: Dhanush Nadella
 * EMAIL: dnadella787@gmail.com 
 * ID: 205150583
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>


/**
 * Relavent research/links used for the project
 * 
 * signal function explanation:
 * https://www.tutorialspoint.com/c_standard_library/c_function_signal.htm
 * 
 * strerror explanation:
 * https://www.tutorialspoint.com/c_standard_library/c_function_strerror.htm
 * 
 * getopt_long example used for assistance:
 * https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
 * 
 * getopt_long syntax/options/arguments:
 * https://linux.die.net/man/3/getopt_long
 * 
 * input/output redirection for the input and output flags:
 * fd_juggling.html (took the redirection code from here)
 * 
 * GDB stack trace:
 * https://sourceware.org/gdb/current/onlinedocs/gdb/Backtrace.html
 * 
 * 
*/

void segfault_handler(){
    fprintf(stderr, "Segmentation fault has been caught. \n");
    exit(4);
}


int main(int argc, char **argv){
    //flags to be set by getopt_long, actual work done using these later on
    int segfault_flag = 0;
    int catch_flag = 0;

    //I/O files
    char *input_file = NULL;
    char *output_file = NULL;

    int c;

    while(1){
        static struct option long_options[] =
            {
                {"input", required_argument, 0, 'i'},
                {"output", required_argument, 0, 'o'},
                {"segfault", no_argument, 0, 's'},
                {"catch", no_argument, 0, 'c'},
                {0, 0, 0, 0}
            };

        int option_index = 0;

        c = getopt_long(argc, argv, "", long_options, &option_index);

        if (c == -1) //end reached
            break;

        switch(c){
            case 'i':  //--input flag
                input_file = optarg;
                int ifd = open(input_file, O_RDONLY);
                if (ifd < 0){   //open call returned an error
                    fprintf(stderr, "Input file %s could not be opened for --input flag.\n", input_file);
                    fprintf(stderr, "Error: %s\n", strerror(errno));
                    exit(2);
                }
                else {
                    close(0);
                    dup(ifd);
                    close(ifd);
                }
                break;

            case 'o': //--output flag
                output_file = optarg;
                int ofd = creat(output_file, 0666);
                if (ofd < 0){   //creat call returned an error
                    fprintf(stderr, "Output file %s could not be opened/created for --output flag.\n", output_file);
                    fprintf(stderr, "Error: %s\n", strerror(errno));
                    exit(3);
                }
                else {
                    close(1);
                    dup(ofd);
                    close(ofd);
                }
                break;

            case 's': //--segfault flag
                segfault_flag = 1;
                break;

            case 'c': //--catch flag
                catch_flag = 1;
                break;

            default: //? is returned because an incorrect flag was used
                fprintf(stderr, "Incorrect flag usage. Allowed flags:\n --input=FILENAME\n --output=FILENAME\n --segfault\n --catch\n");
                exit(1);
            }
    }

    if (catch_flag == 1){   //when a segmentation fault occurs, the segfault_handler function is called
        signal(SIGSEGV, segfault_handler);
    }

    if (segfault_flag == 1){   //illegal access of memory will cause segmentation fault
        char *ptr = NULL;
        *ptr = 'f';
    }

    //copy from fd 0 to fd 1
    char *buf = malloc(sizeof(char));
    int read_return;
    while ((read_return = read(0, buf, 1)) > 0) {  //read returns a positive value if it works properly
        int write_return = write(1, buf, 1);
        if (write_return < 0){    //write returns a negative value if an error occurs
            if (output_file == NULL){
                fprintf(stderr, "Error writing to stdout.\n");
            }
            else {
                fprintf(stderr, "Error writing to file: %s\n", output_file);
            }
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(3);
        }
    }

    if (read_return < 0){   //negative value is returned by read if an error occurs, 0 if EOF is reached
        if (input_file == NULL){
            fprintf(stderr, "Error reading from stdin.\n");
        }
        else {
            fprintf(stderr, "Error reading from file: %s\n", input_file);
        }
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(2);
    }

    free(buf);
    exit(0); //successful copy, return 0
}