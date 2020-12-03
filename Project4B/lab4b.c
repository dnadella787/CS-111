/*
NAME: Dhanush Nadella
EMAIL: dnadella787@gmail.com    
UID: 205150583

relavent research:

https://linuxhint.com/gettimeofday_c_language/

https://www.cprogramming.com/tutorial/printf-format-strings.html

https://www.techonthenet.com/c_language/standard_library_functions/string_h/strncmp.php
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <mraa/gpio.h>
#include <signal.h>
#include <poll.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>

//MACROS:
#define AIO_PORT 1
#define GPIO_PORT 60


//GLOBAL VARIABLES
int log_flag = 0;
FILE *log_file = NULL;

int button_press = 0;


//FUNCTIONS
void print_error(char *error, int exit_code, int errno_flag){
    fprintf(stderr, "%s\n", error);
    if (errno_flag == 1){
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }
    exit(exit_code);
}

void button_pressed(){
    struct timeval tv;
    time_t t;

    int time_retval = gettimeofday(&tv, NULL);
    if (time_retval < 0)
        print_error("Error with gettimeofday\n", 1, 1);
    t = tv.tv_sec;

    struct tm* info;
    info = localtime(&t);

    fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", info->tm_hour, info->tm_min, info->tm_sec);

    if (log_flag == 1)
        fprintf(log_file, "%02d:%02d:%02d SHUTDOWN\n", info->tm_hour, info->tm_min, info->tm_sec);

    button_press = 1;
}




//MAIN()
int main(int argc, char** argv){
    int c;

    int period_num = 1;

    char scale_type = 'F';
    
    char *log_filename = NULL;

    while(1){
        static struct option long_options[] =
            {
                {"period", required_argument, 0, 'p'},
                {"scale", required_argument, 0, 's'},
                {"log", required_argument, 0, 'l'},
                {0, 0, 0, 0}
            };

        int option_index = 0;

        c = getopt_long(argc, argv, "", long_options, &option_index);

        if (c == -1) //end reached
            break;

        switch(c){
            case 'p':  //--period flag
                period_num = atoi(optarg);
                break;

            case 's': //--scale flag
                if ((optarg[0] != 'C') && (optarg[0] != 'F')){
                    fprintf(stderr, "Incorrect flag usage. --scale only takes C or F.\n");
                    exit(1);
                }
                if (optarg[1] != '\0'){
                    fprintf(stderr, "Incorrect flag usage, --scale only takes one option, C or F.\n");
                    exit(1);
                }
                scale_type = optarg[0];
                break;

            case 'l': //--log flag
                log_flag = 1;
                log_filename = optarg;
                break;

            default: //? is returned because an incorrect flag was used
                fprintf(stderr, "Incorrect flag usage. Allowed flags: --period=#, --scale=[CF], --log\n");
                exit(1);
            }
    }

    if (log_filename != NULL){
        log_file = fopen(log_filename, "w");
        if (log_file == NULL){
            print_error("Error opening file for --log", 1, 1);
        }
    }

    mraa_aio_context temperature_sensor;
    temperature_sensor = mraa_aio_init(AIO_PORT);
    if (temperature_sensor == NULL){
        mraa_deinit();
        print_error("Initializing aio for temperature sensor failed.", 1, 0);
    }

    mraa_gpio_context button;
    button = mraa_gpio_init(GPIO_PORT);
    if (button == NULL){
        mraa_deinit();
        print_error("Initializing gpio for button failed.", 1, 0);
    }

    mraa_result_t status;
    status = mraa_gpio_dir(button, MRAA_GPIO_IN);
    if (status != MRAA_SUCCESS){
        print_error("Error setting direction for gpio button", 1, 0);
    }
    status = mraa_gpio_isr(button, MRAA_GPIO_EDGE_FALLING, &button_pressed, NULL);
    if (status != MRAA_SUCCESS){
        print_error("Error setting up interrupt on gpio button", 1, 0);
    }
    signal(SIGINT, button_pressed);

    struct pollfd fds[] = {
        {STDIN_FILENO, (POLLIN | POLLHUP | POLLERR), 0}
    };


    time_t next_temp_time = 0;
    struct timeval current_time;
    int stop_reporting = 0;
    int poll_retval = 0;
    while (button_press == 0){
        int time_retval = gettimeofday(&current_time, NULL);
        if (time_retval < 0)
            print_error("Error with gettimeofday\n", 1, 1);

        if ((stop_reporting == 0) && (current_time.tv_sec >= next_temp_time)){
            time_t t = current_time.tv_sec;

            struct tm* info;
            info = localtime(&t);

            float a = (float)mraa_aio_read(temperature_sensor);
            int B = 4275;
            float R_not = 100000;
            float R = 1023.0/a-1.0;
            R = R * R_not;
            float T_not = 298.15;

            float T = 1.0/(log(R/R_not)/B + (1/T_not)) - 273.15;
            if (scale_type == 'F'){
                T = (T * 1.8) + 32;
            }

            fprintf(stdout, "%02d:%02d:%02d %.1f\n", info->tm_hour, info->tm_min, info->tm_sec, T);

            if (log_flag == 1)
                fprintf(log_file, "%02d:%02d:%02d %.1f\n", info->tm_hour, info->tm_min, info->tm_sec, T);


            next_temp_time = current_time.tv_sec + period_num;
        }


        poll_retval = poll(fds, 1, 10);
        if (poll_retval < 0)
            print_error("poll system call failed", 1, 1);

        //poll system call didnt timeout
        if (poll_retval != 0){
            if (fds[0].revents & POLLIN){
                char read_buffer[100];
                char command[25];

                int bytes_read = read(0, read_buffer, 100);
                if (bytes_read < 0)
                    print_error("Error trying to read input from stdin", 1, 1);

                char curr_char;
                int command_i = 0;
                int i;
                for (i = 0; i < bytes_read; i++){
                    curr_char = read_buffer[i];
                    if (curr_char == '\n'){
                        command[command_i] = '\0';
                        
                        if (strcmp(command, "SCALE=F") == 0){
                            scale_type = 'F';
                            if (log_flag == 1)
                                fprintf(log_file, "SCALE=F\n");
                        }
                        else if (strcmp(command, "SCALE=C") == 0){
                            scale_type = 'C';
                            if (log_flag == 1)
                                fprintf(log_file, "SCALE=C\n");
                        }     
                        else if (strncmp(command, "PERIOD=", 7) == 0){
                            period_num = atoi(&command[7]);
                            if (log_flag == 1)
                                fprintf(log_file, "%s\n", command);
                        }
                        else if (strcmp(command, "STOP") == 0){
                            stop_reporting = 1;
                            if (log_flag == 1)
                                fprintf(log_file, "STOP\n");
                        }      
                        else if (strcmp(command, "START") == 0){
                            stop_reporting = 0;
                            if (log_flag == 1)
                                fprintf(log_file, "START\n");
                        }
                        else if (strncmp(command, "LOG ", 4) == 0){
                            if (log_flag == 1)
                                fprintf(log_file, "%s\n", command);
                        }
                        else if (strcmp(command, "OFF") == 0){
                            if (log_flag == 1)
                                fprintf(log_file, "OFF\n");
                            button_pressed();
                        }

                        command_i = 0;
                    }

                    else {
                        command[command_i] = curr_char;
                        command_i++;
                    }
                }
            }   

        }  
    }

    status = mraa_aio_close(temperature_sensor);
    if (status != MRAA_SUCCESS){
        print_error("Error closing aio temperature sensor", 1, 0);
    }
    status = mraa_gpio_close(button);
    if (status != MRAA_SUCCESS){
        print_error("Error closing gpio button", 1, 0);
    }

    if (log_flag == 1)
        fclose(log_file);

    exit(0);
}