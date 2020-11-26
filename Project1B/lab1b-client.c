/**
 * NAME: Dhanush Nadella
 * EMAIL: dnadella787@gmail.com
 * UID: 205150583
 * 
 * relavent research:
 * 
 * TCP Socket Connection Setup: (the tutorial link in the spec didnt work for me)
 * http://www.linuxhowtos.org/C_C++/socket.htm
 * https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
 * (the two example client and server code files on the rpi.edu site were especially helpful, used them greatly)
 * 
 * Open(2) sys call
 * https://stackoverflow.com/questions/23092040/how-to-open-a-file-which-overwrite-existing-content
 * 
 * zlib compression/decompression:
 * https://gist.github.com/arq5x/5315739
 * 
 * How to write an integer value into the log file:
 * https://stackoverflow.com/questions/43300348/store-an-integer-in-char-array-in-c/43300483
 * http://www.cplusplus.com/reference/cstdio/snprintf/
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
#include <fcntl.h>
#include <zlib.h>
#include <netdb.h>

struct termios original_mode;

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
    int port_flag = 0;
    char *port_num_char;
    int port_num;

    int log_flag = 0;
    int log_fd;
    char *log_file;

    int compression_flag = 0;

    int c;

    while (1) //parse cli flags
    {
        static struct option long_options[] = {
            {"port", required_argument, 0, 'p'},
            {"log", required_argument, 0, 'l'},
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
            case 'l':
                log_flag = 1;
                log_file = optarg;
                break;
            case 'c':
                compression_flag = 1;
                break;

            default: //case where c = ? because an unidentified argument was passed.
                fprintf(stderr, "Improper flag usage. Allowed flags: --log=FILENAME, --port=PORT#, --compress \n");
                exit(1);
            }
    }

    if (port_flag == 0){
        fprintf(stderr, "Improper flag usage. --port=PORT# must be specified.\n");
        exit(1);
    }

    if (log_flag == 1){
        //open the log file (created if doesn't exist) to be written to and completely overwritten if it does exist
        //has permissions rw-rw-rw-
        int log_retval = log_fd = open(log_file, O_WRONLY | O_CREAT | O_TRUNC, 0666); 
        if (log_retval < 0){
            fprintf(stderr, "Error opening file specified by --log flag.\n");
            fprintf(stderr, "Error: %s.\n", strerror(errno));
            exit(1);
        }
    }

    startup();

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        fprintf(stderr, "Error while trying to open socket from client side.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }

    struct hostent *host = gethostbyname("localhost");
    if (host == NULL){
        fprintf(stderr, "Error getting server host.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }

    struct sockaddr_in addr;

    bzero((char *)&addr, sizeof(addr));         //initialized entire struct to 0 (found this in the socket tutorial)
    addr.sin_family = AF_INET;                  //set sin_family var in sockaddr_in struct to AF_INET(ipv4)
    addr.sin_port = htons(port_num);            //set the port in_port_t struct 
    memcpy((char *)&addr.sin_addr.s_addr, (char *)host->h_addr, host->h_length); //set the address in the in_addr struct

    
    int connect_retval = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (connect_retval < 0){
        fprintf(stderr, "Error while connecting to server.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }

    struct pollfd fds[] = {
        {0, (POLLIN | POLLHUP | POLLERR), 0},
        {sockfd, (POLLIN | POLLHUP | POLLERR), 0}
    };

    //
    z_stream write_socket, read_socket;
    if (compression_flag == 1){
        write_socket.zalloc = Z_NULL;
        write_socket.zfree = Z_NULL;
        write_socket.opaque = Z_NULL;
        read_socket.zalloc = Z_NULL;
        read_socket.zfree = Z_NULL;
        read_socket.opaque = Z_NULL;
    }

                    
    int poll_retval;
    int stop_polling = 0;

    while (1) {
        poll_retval = poll(fds, 2, -1);
        if (poll_retval < 0){
            fprintf(stderr, "Error when polling available file descriptors.\n");
            fprintf(stderr, "Error: %s.\n", strerror(errno));
            exit(1);
        }
        
        if (poll_retval > 0){  //poll returns positive number if any of the elements in the pollfd
                                //struct have their revent  fields set to a non-zero value (something happened)
          
            //input to be taken from stdin and written to socket towards server
            if (fds[0].revents & POLLIN){
                char buffer[2048];

                int bytes_read = read(0, buffer, 10);
                if (bytes_read < 0){
                    fprintf(stderr, "Error trying to read input from terminal.\n");
                    fprintf(stderr, "Error: %s.\n", strerror(errno));
                    exit(1);
                }

                //handles processing for display
                char curr_char;
                for (int i = 0; i < bytes_read; i++) {
                    curr_char = buffer[i];
                    
                    //<cr> or <lf> should echo as <cr><lf> but go to the shell as <lf>
                    if ((curr_char == '\r') || (curr_char == '\n')){
                        char map[2] = {'\r', '\n'};
                        int write_retval = write(1, map, 2);
                        if (write_retval < 0){
                            fprintf(stderr, "Error writing to stdout after reading from keyboard.\n");
                            fprintf(stderr, "Error: %s.\n", strerror(errno));
                            exit(1);
                        }
                    }

                    else {
                        int write_retval = write(1, &curr_char, 1);
                        if (write_retval < 0){
                            fprintf(stderr, "Error writing to stdout after reading from keyboard.\n");
                            fprintf(stderr, "Error: %s.\n", strerror(errno));
                            exit(1);
                        }
                    }
                }

                //if --compress, rewrite the original buffer with compressed contents
                if (compression_flag == 1){
                    char compressed_buffer[2048];

                    write_socket.avail_in = (uInt)bytes_read;
                    write_socket.next_in = (Bytef *)buffer;
                    write_socket.avail_out = (uInt)(sizeof(compressed_buffer));
                    write_socket.next_out = (Bytef *)compressed_buffer;

                    if (deflateInit(&write_socket, Z_DEFAULT_COMPRESSION) != Z_OK){
                        fprintf(stderr, "Error with deflateInit() on client side.\n");
                        exit(1);
                    }

                    do {
                        if (deflate(&write_socket, Z_SYNC_FLUSH) != Z_OK){
                            fprintf(stderr, "Error with deflate() on client side.\n");
                            exit(1);
                        }
                    } while(write_socket.avail_in > 0);
                    bytes_read = write_socket.total_out;

                    if (deflateEnd(&write_socket) != Z_OK){
                        fprintf(stderr, "Error with deflateEnd() on client side.\n");
                        exit(1);
                    }


                    for (int i = 0; i < bytes_read; i++){
                        buffer[i] = compressed_buffer[i];
                    }
                }

                //write contents of buffer to the opened socket towards the server
                //dont change anything just send it as is (except compression)
                int write_retval = write(sockfd, &buffer, bytes_read);
                if (write_retval < 0){
                    fprintf(stderr, "Error writing input from keyboard to socket from client.\n");
                    fprintf(stderr, "Error: %s.\n", strerror(errno));
                    exit(1);
                }

                //write to the log file if the --log flag was specified
                if ((bytes_read > 0) && (log_flag == 1)){
                    char message[100];
                    snprintf(message, 100, "SENT %d bytes: ", bytes_read);

                    int write_retval = write(log_fd, message, strlen(message));
                    if (write_retval < 0){
                        fprintf(stderr, "Error writing data to log file.\n");
                        fprintf(stderr, "Error: %s.\n", strerror(errno));
                        exit(1);
                    }

                    write_retval = write(log_fd, buffer, bytes_read);
                    if (write_retval < 0){
                        fprintf(stderr, "Error writing data to log file.\n");
                        fprintf(stderr, "Error: %s.\n", strerror(errno));
                        exit(1);
                    }
                    char newline = '\n';
                    write_retval = write(log_fd, &newline, 1);
                    if (write_retval < 0){
                        fprintf(stderr, "Error writing newline to log file.\n");
                        fprintf(stderr, "Error: %s.\n", strerror(errno));
                        exit(1);
                    }
                }
            }

            if (fds[0].revents & (POLLHUP | POLLERR)){
                stop_polling = 1;
            }
            //input taken through socket from server
            if (fds[1].revents & POLLIN){
            
                char buffer[2048];

                int bytes_read = read(sockfd, buffer, 2048);
                if (bytes_read < 0){
                    fprintf(stderr, "Error trying to read input from socket as client.\n");
                    fprintf(stderr, "Error: %s.\n", strerror(errno));
                    exit(1);
                }

                if (bytes_read == 0) //EOF sent by server
                    stop_polling = 1;

                //log the data read from the socket
                if (log_flag == 1){
                    char message[100];
                    snprintf(message, 100, "RECEIVED %d bytes: ", bytes_read);
                    int write_retval = write(log_fd, message, strlen(message));
                    if (write_retval < 0){
                        fprintf(stderr, "Error writing data to log file.\n");
                        fprintf(stderr, "Error: %s.\n", strerror(errno));
                        exit(1);
                    }

                    write_retval = write(log_fd, buffer, bytes_read);
                    if (write_retval < 0){
                        fprintf(stderr, "Error writing data to log file.\n");
                        fprintf(stderr, "Error: %s.\n", strerror(errno));
                        exit(1);
                    }
                    char newline = '\n';
                    write_retval = write(log_fd, &newline, 1);
                    if (write_retval < 0){
                        fprintf(stderr, "Error writing newline to log file.\n");
                        fprintf(stderr, "Error: %s.\n", strerror(errno));
                        exit(1);
                    }                    
                }

                //decompress the input from socket before writing to screen
                char decompressed_buffer[2048];
                if (compression_flag == 1){
                    read_socket.avail_in = (uInt)(bytes_read);
                    read_socket.next_in = (Bytef *)buffer;
                    read_socket.avail_out = (uInt)(sizeof(decompressed_buffer));
                    read_socket.next_out = (Bytef *)decompressed_buffer;
            
                    if (inflateInit(&read_socket) != Z_OK){
                        fprintf(stderr, "Error with inflateInit() on client side.\n");
                        exit(1);
                    }

                    do {
                        if (inflate(&read_socket, Z_SYNC_FLUSH) != Z_OK){
                            fprintf(stderr, "Error with inflate() on client side.\n");
                            exit(1);
                        }
                    } while(read_socket.avail_in > 0);
                    bytes_read = read_socket.total_out;

                    if (inflateEnd(&read_socket) != Z_OK){
                        fprintf(stderr, "Error with inflateEnd() on client side.\n");
                        exit(1);
                    }
                }

                //actully process each character, char by char, choose buffer based on flag
                char curr_char;
                for (int i = 0; i < bytes_read; i++){
                    if (compression_flag == 1){
                        curr_char = decompressed_buffer[i];
                    }
                    else{
                        curr_char = buffer[i];
                    }
                    if (curr_char == '\n'){
                        char map[2] = {'\r', '\n'};
                        int write_retval = write(1, &map, 2);
                        if (write_retval < 0){
                            fprintf(stderr, "Error writing input from socket to stdout as client.\n");
                            fprintf(stderr, "Error: %s.\n", strerror(errno));
                            exit(1);
                        }
                    }
                    else{
                        int write_retval = write(1, &curr_char, 1);
                        if (write_retval < 0){
                            fprintf(stderr, "Error writing input from socket to stdout as client.\n");
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

    if (log_flag == 1)
        close(log_fd);


    //reset mode of terminal before exiting
    int setattr_retval = tcsetattr(0, TCSANOW, &original_mode);
    if (setattr_retval < 0){
        fprintf(stderr, "Error when attempting to restore terminal back to original mode before exitting.\n");
        fprintf(stderr, "Error: %s.\n", strerror(errno));
        exit(1);
    }
    
    exit(0);
}