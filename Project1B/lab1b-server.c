/**
 * NAME: Dhanush Nadella
 * EMAIL: dnadella787@gmail.com
 * UID: 205150583
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
#include <sys/socket.h>
#include <zlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int global_pid;
int global_newsockfd;


void sigpipe_handler(){
    int status;
    int wait_retval = waitpid(global_pid, &status, 0);
    if (wait_retval < 0){
        fprintf(stderr, "Error when monitoring state changes of child process with waitpid.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }

    int SIGNAL = status & 0xff;
    int STATUS = (status / 256);
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", SIGNAL, STATUS);

    shutdown(global_newsockfd, SHUT_RDWR);

    exit(0);
}



int main(int argc, char** argv){
    int port_flag = 0;
    char* port_num_char;
    int port_num;

    int shell_flag = 0;
    char* shell_program;
    
    int compression_flag = 0;

    int c;

    while (1) //parse cli flags
    {
        static struct option long_options[] = {
            {"port", required_argument, 0, 'p'},
            {"shell", required_argument, 0, 's'},
            {"compress", no_argument, 0, 'c'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long(argc, argv, "", long_options, &option_index);

        if (c == -1) //end of argv reached
            break;

        switch(c){
            case 'p':
                port_flag = 1;
                port_num_char = optarg;
                port_num = atoi(port_num_char);
                break;
            case 's':
                shell_flag = 1;
                shell_program = optarg;
                break;
            case 'c':
                compression_flag = 1;
                break;
            default: //case where c = ? because an unidentified argument was passed.
                fprintf(stderr, "Improper flag usage. Allowed flags: --compress, --shell=PROGRAM, --port=PORT#\n");
                exit(1);
        }
    }


    //initialize zlib streams
    z_stream write_socket, read_socket;
    if (compression_flag == 1) {
        write_socket.zalloc = Z_NULL;
        write_socket.zfree = Z_NULL;
        write_socket.opaque = Z_NULL;
        read_socket.zalloc = Z_NULL;
        read_socket.zfree = Z_NULL;
        read_socket.opaque = Z_NULL;
    }

    if (shell_flag == 0){
        shell_program = "/bin/bash";
    }

    if (port_flag == 0){
        fprintf(stderr, "Improper flag usage, --port=PORT# must be specified.\n");
        exit(1);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        fprintf(stderr, "Error while trying to open socket from server side.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }

    struct sockaddr_in server_addr, client_addr;

    bzero((char *)&server_addr, sizeof(server_addr));  //initialized entire struct to 0 (found this in the socket tutorial)
    server_addr.sin_family = AF_INET;                  //set sin_family var in sockaddr_in struct to AF_INET(ipv4)
    server_addr.sin_port = htons(port_num);            //set the port in_port_t struct
    server_addr.sin_addr.s_addr = INADDR_ANY;          //set the address in the in_addr struct

    int bind_retval = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_retval < 0){
        fprintf(stderr, "Error binding name to socket.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }

    int listen_retval = listen(sockfd, 5);
    if (listen_retval < 0){
        fprintf(stderr, "Error listening for incoming connections.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }

    int client_len = sizeof(client_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *)&client_addr,(socklen_t *)&client_len);
    global_newsockfd = newsockfd;
    if (newsockfd < 0){
        fprintf(stderr, "Error accepting connection on socket.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }

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
    global_pid = pid;

    if (pid < 0){
        fprintf(stderr, "Error when trying to fork to create child process.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }

    else if (pid == 0){              //pid has value 0 in child process

        close(fd_from_tprocess[1]);  //close the write pipe from the terminal process
        close(fd_to_tprocess[0]);    //close the read pipe to the terminal process

        close(0);                    //close stdin and dup the read pipe from terminal process to stdin
        dup(fd_from_tprocess[0]);    //then close the pipe, so STDIN = read pipe for shell
        close(fd_from_tprocess[0]);

        close(1);                    //close stdout and dup the write pipe from terminal process to stdout
        close(2);                    //close stderr and dup the write pipe from terminal process to stderr
        dup(fd_to_tprocess[1]);
        dup(fd_to_tprocess[1]);      
        close(fd_to_tprocess[1]);    //then close the pipe, so STDOUT,STDERR = write pipe from shell

        int exec_retval = execl(shell_program, shell_program, (char *)NULL);
        if (exec_retval < 0){
            fprintf(stderr, "Error when trying to execute shell program in child process.\n");
            fprintf(stderr, "Error: %s.\n", strerror(errno));
            exit(1);
        }

    }

    else {
        signal(SIGPIPE, sigpipe_handler);//if sigpipe received, harvest shell completition status and log to stderr
        close(fd_from_tprocess[0]);      //close off the read pipe for the shell from server
        close(fd_to_tprocess[1]);        //close off the write pipe from the server to the shell

        struct pollfd fds[] = {
            {newsockfd, (POLLIN | POLLHUP | POLLERR), 0},
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
                //input from the socket
                if (fds[0].revents & POLLIN){
                    char buffer[2048];

                    int bytes_read = read(newsockfd, buffer, 2048);
                    if (bytes_read < 0){
                        fprintf(stderr, "Error trying to read input from client through socket.\n");
                        fprintf(stderr, "Error: %s.\n", strerror(errno));
                        exit(1);
                    }


                    char decompressed_buffer[2048];
                    if (compression_flag == 1){
                        read_socket.avail_in = (uInt)bytes_read;
                        read_socket.next_in = (Bytef *)buffer;
                        read_socket.avail_out = (uInt)(sizeof(decompressed_buffer));
                        read_socket.next_out = (Bytef *)decompressed_buffer;

                        if (inflateInit(&read_socket) != Z_OK){
                            fprintf(stderr, "Error with inflateInit() on server side.\n");
                            exit(1);
                        }

                        do {
                            if (inflate(&read_socket, Z_SYNC_FLUSH) != Z_OK){
                                fprintf(stderr, "Error with inflate() on server side.\n");
                                exit(1);
                            }
                        } while(read_socket.avail_in > 0);
                        bytes_read = read_socket.total_out;

                        if (inflateEnd(&read_socket) != Z_OK){
                            fprintf(stderr, "Error with inflateEnd() on server side.\n");
                            exit(1);
                        }
                    }
            

                    char curr_char;
                    for (int i = 0; i < bytes_read; i++) {
                        if (compression_flag == 1){
                            curr_char = decompressed_buffer[i];
                        }
                        else {
                            curr_char = buffer[i];
                        }

                        if (curr_char == 0x04){      //EOF reached
                            close(fd_from_tprocess[1]);
                        }

                        else if (curr_char == 0x03){ //interrrupt character (^C) received in socket, kill terminal
                            int kill_retval = kill(pid, SIGINT);
                            if (kill_retval < 0){
                                fprintf(stderr, "Error trying to send SIGINT to shell process.\n");
                                fprintf(stderr, "Error: %s.\n", strerror(errno));
                                exit(1);
                            }
                        }

                        else if (curr_char == '\r'){ //client --<cr>--> server --<lf>--> shell
                            char map2 = '\n';
                            int write_retval = write(fd_from_tprocess[1], &map2, 1);
                            if (write_retval < 0){
                                fprintf(stderr, "Error writing to shell.\n");
                                fprintf(stderr, "Error: %s\n", strerror(errno));
                                exit(1);
                            }
                        }

                        else {                      //normal character from client, just write to shell
                            int write_retval = write(fd_from_tprocess[1], &curr_char, 1);
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

                //input from the shell
                if (fds[1].revents & POLLIN){
                    char buffer[2048];

                    int bytes_read = read(fd_to_tprocess[0], buffer, 2048);
                    if (bytes_read < 0){
                        fprintf(stderr, "Error trying to read input from terminal.\n");
                        fprintf(stderr, "Error: %s.\n", strerror(errno));
                        exit(1);
                    }

                    if (bytes_read == 0) //EOF reached
                        stop_polling = 1;
                    
                    //if --compress, compress contents and write those to socket
                    if (compression_flag == 1){
                        char compressed_buffer[2048];

                        write_socket.avail_in = (uInt)bytes_read;
                        write_socket.next_in = (Bytef *)buffer;
                        write_socket.avail_out = (uInt)(sizeof(compressed_buffer));
                        write_socket.next_out = (Bytef *)compressed_buffer;

                        if (deflateInit(&write_socket, Z_DEFAULT_COMPRESSION) != Z_OK){
                            fprintf(stderr, "Error with deflateInit() on server side.\n");
                            exit(1);
                        }

                        do {
                            if (deflate(&write_socket, Z_SYNC_FLUSH) != Z_OK){
                                fprintf(stderr, "Error with deflate() on server side.\n");
                                exit(1);
                            }
                        } while(write_socket.avail_in > 0);
                        bytes_read = write_socket.total_out;

                        if (deflateEnd(&write_socket) != Z_OK){
                            fprintf(stderr, "Error with deflateEnd() on server side.\n");
                            exit(1);
                        }

                        int write_retval = write(newsockfd, compressed_buffer, bytes_read);
                        if (write_retval < 0){
                            fprintf(stderr, "Error writing to socket from server.\n");
                            fprintf(stderr, "Error: %s\n", strerror(errno));
                            exit(1);
                        }
                    }
                    //otherwise just write the read in contents from the shell as is
                    else {
                        int write_retval = write(newsockfd, buffer, bytes_read);
                        if (write_retval < 0){
                            fprintf(stderr, "Error writing to socket from server.\n");
                            fprintf(stderr, "Error: %s\n", strerror(errno));
                            exit(1);
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

        shutdown(newsockfd, SHUT_RDWR);

        exit(0);
    }
}