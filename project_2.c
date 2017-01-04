/*
********************************
* Operating Systems Homework 4 *
* Due Date: Sunday October 18  *
*                              *
* Name: Stephen Smart          *
* ID: 113851356                *
********************************
*/

// Includes 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>

// Global variables
int thread_count = 3;
char input[1] = "\0";
char buffer[100] = "\0";
// Flag to signal the input file has been read
int file_read_complete = 0;
// Flag to signal the input file has been processed
int file_process_complete = 0;
// Counter to keep track of the index of the last char in the buffer
int end_of_buffer_index = 0;
// Flag to signal the buffer is ready to print
int buffer_ready = 0;
pthread_mutex_t mutex_t1_t2 = PTHREAD_MUTEX_INITIALIZER;

// Removes the last word from the given string and returns the index of the last char
// in the string after removal
int remove_last_word(char* string) {

    int i = 0;
    int j = 0;
    int k = 0;
    int just_found_space = 0;

    while (string[i] != '\0') {
        
        if (string[i] == ' ') {
            if (!just_found_space) {
                k = j;
            }
            j = i;
            just_found_space = 1;
        } else {
            just_found_space = 0;
        }
        
        i++;
    }

    if (just_found_space) {
        if (k == 0) {
            string[k] = '\0';
            return k;
        } else {
            string[k+1] = '\0';
            return k+1;
        }
    } else {
        if (j == 0) {
            string[j] = '\0';
            return j;
        } else {
            string[j+1] = '\0';
            return j+1;
        }
    }
}


// Thread 1 function: reads input from the file
void* read_input(void* file_name) {

    char* f_name = (char*) file_name;

    int fd = open(f_name, O_RDONLY);    

    int bytes_read = 1;
    while (bytes_read == 1) {

        // Critical section
        pthread_mutex_lock(&mutex_t1_t2);
        
        bytes_read = read(fd, input, 1);

        pthread_mutex_unlock(&mutex_t1_t2);
        // End critical section      

    }

    file_read_complete = 1;

    return NULL;
}

// Thread 2 function: processes the input that thread 1 reads in
void* process_input(void* arg) {

    while (!file_read_complete) {

        // Critical section
        pthread_mutex_lock(&mutex_t1_t2);

        // Waiting on thread 3 if buffer is being printed
        while (buffer_ready);

        switch (input[0]) {
        case '*':
            if (end_of_buffer_index > 0) {
                end_of_buffer_index--;
                buffer[end_of_buffer_index] = '\0';
            }
            break;
        case '@':
            // Signaling thread 3 to go
            buffer_ready = 1;
            break;
        case '\n':
            // Signaling thread 3 to go
            buffer_ready = 1;
            break;
        case '$':
            end_of_buffer_index = remove_last_word(buffer);            
            break;
        case '&':
            buffer[0] = '\0';
            end_of_buffer_index = 0;
            break;
        default:
            strcat(buffer, input);
            end_of_buffer_index++;
        }

        input[0] = '\0';

        pthread_mutex_unlock(&mutex_t1_t2);
        // End critical section

    }

    file_process_complete = 1;
    buffer_ready = 1;

    return NULL;
}

// Thread 3 function: printing the buffer when ready
void* print_buffer(void* arg) {
    
    while (!file_process_complete) {
   
        // Critical section
        // Waiting on thread 2 to process input
        while (!buffer_ready);

        printf("%s\n", buffer);
        buffer[0] = '\0';
        end_of_buffer_index = 0;

        buffer_ready = 0;
        // End critical section

    }

    return NULL;
}


// Main function
int main(int argc, char* argv[]) {

    if (argc != 2) {
        
        printf("Usage: %s file-name\n", argv[1]); 

    } else {

        int rc[thread_count];
        pthread_t threads[thread_count];

        // Creating thread 1
        rc[0] = pthread_create(&threads[0], NULL, read_input, (void*) argv[1]);        
        if (rc[0]) {
            printf("ERROR: Return Code: %d\n", rc[0]);
        }

        // Creating thread 2
        int thread_id = 2;
        rc[1] = pthread_create(&threads[1], NULL, process_input, (void*) thread_id);
        if (rc[1]) {
            printf("ERROR: Return Code: %d\n", rc[1]);
        } 

        // Creating thread 3
        thread_id = 3;
        rc[2] = pthread_create(&threads[2], NULL, print_buffer, (void*) thread_id);
        if (rc[2]) {
            printf("ERROR: Return Code: %d\n", rc[2]);
        }

        // Waiting on threads to finish execution
        int i;
        for (i = 0; i < thread_count; i++) {
            pthread_join(threads[i], NULL);
        }

        // Exiting program
        return 0;

    }
}
