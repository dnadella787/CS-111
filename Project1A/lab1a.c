/**
 * NAME: Dhanush Nadella 
 * EMAIL: dnadella787@gmail.com
 * UID: 205150583
 * 
 * 
 * relavent research:
 * 
 * pipe system call:
 * https://www.geeksforgeeks.org/pipe-system-call/
 * 
 * poll and how the timeout argument works for infinite timeout 
 * https://www.tutorialspoint.com/unix_system_calls/poll.htm
 * https://man7.org/linux/man-pages/man2/poll.2.html
 * https://stackoverflow.com/questions/58875489/linux-poll-behaviour-with-infinite-timeout?rq=1
 * 
 * 
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

struct termios original_mode;      //termios struct that stores the original terminal values at startup

void startup(){

    struct termios new_mode;       //termios struct that gets original values and then changes it for tcsetattr

    int getattr_retval = tcgetattr(0, &new_mode);
    if (getattr_retval < 0){
        fprintf(stderr, "Error when attempting to get current terminal mode at startup with tcgetattr.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }

    original_mode = new_mode;      //set the original mode global variable

    //set into non-canonical input mode with no echo
    new_mode.c_iflag = ISTRIP; //only lower 7 bits
    new_mode.c_oflag = 0;      //no processing
    new_mode.c_lflag = 0;      //no processing

    int setattr_retval = tcsetattr(0, TCSANOW, &new_mode);
    if (setattr_retval < 0){
        fprintf(stderr, "Error when attempting to set new terminal mode at startup with tcsetattr.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }
}



int main(int argc, char** argv){

    int shell_flag = 0;
    char *shell_program;
    int c;

    while (1) //parse cli flags
    {
        static struct option long_options[] = {
            {"shell", required_argument, 0, 's'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long(argc, argv, "", long_options, &option_index);

        if (c == -1) //end of argv reached
            break;

        switch(c){
            case 's': //--shell flag used
                shell_program = optarg;
                shell_flag = 1;
                break;

            default: //case where c = ? because an unidentified argument was passed.
                fprintf(stderr, "Improper flag usage. Only --shell=PROGRAM allowed.\n");
                exit(1);
            }
    }

    //setup changes to terminal mode
    startup();

    //shell flag not used
    if (shell_flag == 0){
        //read input from keyboard, processs it and then write to display
        char buffer[100];             //buffer to read input
        int bytes_read;                //return value for read syscall
    
        while (1) {

            bytes_read = read(0, buffer, 100);
            if (bytes_read < 0){
                fprintf(stderr, "Error trying to read keyboard input.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }

            char curr_char;
            for (int i = 0; i < bytes_read; i++) { //iterate through buffer to process each character
                curr_char = buffer[i];

                if (curr_char == 0x04){       //EOF reached, reset terminal mode and exit                
                    int setattr_retval = tcsetattr(0, TCSANOW, &original_mode);
                    if (setattr_retval < 0){
                        fprintf(stderr, "Error when attempting to restore terminal back to original mode before exitting.\n");
                        fprintf(stderr, "Error: %s.\n", strerror(errno));
                        exit(1);
                    }
                    exit(0);    
                }

                else if ((curr_char == '\r') || (curr_char == '\n')){  //<cr> or <lf> reached, map to <cr><lf>
                    char map[2] = {'\r', '\n'};
                    int write_retval = write(1, &map, 2);
                    if (write_retval < 0){
                        fprintf(stderr, "Error when writing to display after processing.\n");
                        fprintf(stderr, "Error: %s.\n", strerror(errno));
                        exit(1);
                    }
                }

                else {                      //normal character, just write to display immediately
                    int write_retval = write(1, &curr_char, 1);
                    if (write_retval < 0){
                        fprintf(stderr, "Error when writing to display after processing.\n");
                        fprintf(stderr, "Error: %s.\n", strerror(errno));
                        exit(1);
                    }
                }
            }
        }
    }

    //--shell flag used
    else {
        //setup pipes
        int fd_from_tprocess[2];
        int fd_to_tprocess[2];

        int pipe1_retval = pipe(fd_from_tprocess);
        if (pipe1_retval < 0){
            fprintf(stderr, "Error trying to create pipe.\n");
            fprintf(stderr, "Error: %s.\n", strerror(errno));
            exit(1);
        }

        int pipe2_retval = pipe(fd_to_tprocess);
        if (pipe2_retval < 0){
            fprintf(stderr, "Error trying to create pipe.\n");
            fprintf(stderr, "Error: %s.\n", strerror(errno));
            exit(1);
        }

        //create child process
        int pid = fork();
        if (pid < 0){
            fprintf(stderr, "Error when trying to fork to create child process.\n");
            fprintf(stderr, "Error: %s.\n", strerror(errno));
            exit(1);
        }

        else if (pid == 0){              //pid has value 0 in child process

            close(fd_from_tprocess[1]);  //close the write pipe from the terminal process
            close(fd_to_tprocess[0]);    //close the read pipe to the terminal process

            close(0);                    //close stdin and dup the read pipe from terminal process to stdin
            dup(fd_from_tprocess[0]);    //then close the pipe
            close(fd_from_tprocess[0]);

            close(1);                    //close stdout and dup the write pipe from terminal process to stdout
            close(2);                    //close stderr and dup the write pipe from terminal process to stderr
            dup(fd_to_tprocess[1]);
            dup(fd_to_tprocess[1]);      
            close(fd_to_tprocess[1]);    //then close the pipe

            int exec_retval = execl(shell_program, shell_program, (char *)NULL);
            if (exec_retval < 0){
                fprintf(stderr, "Error when trying to execute shell program in child process.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }

        }

        else {                           //otherwise pid = pid of parent/calling process
            close(fd_from_tprocess[0]);
            close(fd_to_tprocess[1]);

            struct pollfd fds[] = {
                {0, (POLLIN | POLLHUP | POLLERR), 0},
                {fd_to_tprocess[0], (POLLIN | POLLHUP | POLLERR), 0}
            };

            int poll_retval;
            int stop_polling = 0;

            while (1) {
                poll_retval = poll(fds, 2, -1);
                if (poll_retval < 0){
                    fprintf(stderr, "Error when polling available fill descriptors.\n");
                    fprintf(stderr, "Error: %s.\n", strerror(errno));
                    exit(1);
                }
                
                if (poll_retval > 0){  //poll returns positive number if any of the elements in the pollfd
                                       //struct have their revent fields set to a non-zero value (something happened)
                    if (fds[0].revents & POLLIN){
                        char buffer[10];

                        int bytes_read = read(0, buffer, 10);
                        if (bytes_read < 0){
                            fprintf(stderr, "Error trying to read input from terminal.\n");
                            fprintf(stderr, "Error: %s.\n", strerror(errno));
                            exit(1);
                        }

                        char curr_char;
                        for (int i = 0; i < bytes_read; i++) {
                            curr_char = buffer[i];

                            if (curr_char == 0x04){      //EOF reached
                                close(fd_from_tprocess[1]);
                            }

                            else if (curr_char == 0x03){ //interrrupt character (^C) reached
                                char map[2] = {'^', 'C'};
                                int write_retval = write(1, &map, 2);
                                if (write_retval < 0){
                                    fprintf(stderr, "Error writing interrupt character to stdout.\n");
                                    fprintf(stderr, "Error: %s.\n", strerror(errno));
                                    exit(1);
                                }

                                int kill_retval = kill(pid, SIGINT);
                                if (kill_retval < 0){
                                    fprintf(stderr, "Error trying to send SIGINT to shell process.\n");
                                    fprintf(stderr, "Error: %s.\n", strerror(errno));
                                    exit(1);
                                }
                            }

                            else if ((curr_char == '\r') || (curr_char == '\n')){ //<lf> or <cr> reached
                                char map[2] = {'\r', '\n'};
                                int write_retval = write(1, &map, 2);
                                if (write_retval < 0){
                                    fprintf(stderr, "Error writing to stdout.\n");
                                    fprintf(stderr, "Error: %s\n", strerror(errno));
                                    exit(1);
                                }

                                char map2 = '\n';
                                write_retval = write(fd_from_tprocess[1], &map2, 1);
                                if (write_retval < 0){
                                    fprintf(stderr, "Error writing to shell.\n");
                                    fprintf(stderr, "Error: %s\n", strerror(errno));
                                    exit(1);
                                }
                            }

                            else {                     //normal character, just echo and forward to shell
                                int write_retval = write(1, &curr_char, 1);
                                if (write_retval < 0){
                                    fprintf(stderr, "Error writing to stdout.\n");
                                    fprintf(stderr, "Error: %s\n", strerror(errno));
                                    exit(1);
                                }

                                write_retval = write(fd_from_tprocess[1], &curr_char, 1);
                                if (write_retval < 0){
                                    fprintf(stderr, "Error writing to shell.\n");
                                    fprintf(stderr, "Error: %s\n", strerror(errno));
                                    exit(1);
                                }
                            }
                        }
                    }
                    if (fds[0].revents & (POLLHUP | POLLERR)){
                        stop_polling = 1;
                    }
                    if (fds[1].revents & POLLIN){
                        char buffer[256];

                        int bytes_read = read(fd_to_tprocess[0], buffer, 256);
                        if (bytes_read < 0){
                            fprintf(stderr, "Error trying to read input from terminal.\n");
                            fprintf(stderr, "Error: %s.\n", strerror(errno));
                            exit(1);
                        }

                        char curr_char;
                        for (int i = 0; i < bytes_read; i++) {
                            curr_char = buffer[i];

                            if (curr_char == 0x04){          //EOF reached from shell pipe
                                stop_polling = 1;            //set flag to 1 but keep processing input
                            }

                            else if (curr_char == '\n'){     //<lf> reached from shell, maps to <cr><lf>
                                char map[2] = {'\r', '\n'};
                                int write_retval = write(1, map, 2);
                                if (write_retval < 0){
                                    fprintf(stderr, "Error writing to stdout after reading from shell.\n");
                                    fprintf(stderr, "Error: %s.\n", strerror(errno));
                                    exit(1);
                                }
                            }

                            else {                           //otherwise normal character, just echo out
                                int write_retval = write(1, &curr_char, 1);
                                if (write_retval < 0){
                                    fprintf(stderr, "Error writing to stdout after reading from shell.\n");
                                    fprintf(stderr, "Error: %s.\n", strerror(errno));
                                    exit(1);
                                }
                            }
                        }
                    }
                    if (fds[1].revents & (POLLHUP | POLLERR)){
                        stop_polling = 1;
                    }
                }

                if (stop_polling == 1)
                    break;
            }

            close(fd_from_tprocess[1]);

            int status;
            int wait_retval = waitpid(pid, &status, 0);
            if (wait_retval < 0){
                fprintf(stderr, "Error when monitoring state changes of child process with waitpid.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }

            int SIGNAL = status & 0xff;
            int STATUS = (status / 256);
            fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", SIGNAL, STATUS);

            //reset mode of terminal before exiting
            int setattr_retval = tcsetattr(0, TCSANOW, &original_mode);
            if (setattr_retval < 0){
                fprintf(stderr, "Error when attempting to restore terminal back to original mode before exitting.\n");
                fprintf(stderr, "Error: %s.\n", strerror(errno));
                exit(1);
            }
            exit(0); 
        }
    }
}
