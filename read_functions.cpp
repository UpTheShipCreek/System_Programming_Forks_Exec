#include "read_functions.h"

void find_and_write_urls(char* line, int file_descriptor){ //this function searches a buffer for all the urls and writes them in the file given in the file descriptor
    int bytes;
    char* re = line; //get a pointer to the start of the buffer
    char* end = strchr(re, ' '); //get a pointer to the first space 
    bytes = end - re; //starting at re and ending at end, is the first word and it is re - end bytes long
    if(memcmp(re, "http://www.", 11) == 0){ //check to see if the word starting at re is a url of http://www type
        if(write(file_descriptor, re+11, bytes-11) == -1){ //if it is, write it, excluding the first 11 bytes (http://www), to the output file
            perror("write");
            exit(1);
        }
        if(write(file_descriptor, "\n", 1) == -1){ //new line after every tempt to write
            perror("write");
            exit(1);
        }
        //printf("%.*s\n", bytes-11, re+11);
    }
    else if(memcmp(re, "http://", 7) == 0){//check to see if the word starting at re is a url of http:// type
        if(write(file_descriptor, re+7, bytes-7) == -1){ //if it is, write it, excluding the first 7 bytes (http://), to the output file
            perror("write");
            exit(1);
        }
        if(write(file_descriptor, "\n", 1) == -1){ //new line after every tempt to write
            perror("write");
            exit(1);
        }
        //printf("%.*s\n", bytes-7, re+7);
    }
    while(end != NULL){  //same job until the end
        re = end+1; //the start of the next word is the next byte of the space
        if((end = strchr(re, ' ')) == NULL){ //if there is no space
            if((end = strchr(re, '\n')) == NULL){ //search for new line
                return; //if there is no new line return
            }
        }
        bytes = end - re; //the # of bytes of the new word
        if(memcmp(re, "http://www.", 11) == 0){ //same process
            if(write(file_descriptor, re+11, bytes-11) == -1){
                perror("write");
                exit(1);
            }
            if(write(file_descriptor, "\n", 1) == -1){
                perror("write");
                exit(1);
            }
            //printf("%.*s\n", bytes-11, re+11);
        }
        else if(memcmp(re, "http://", 7) == 0){
            if(write(file_descriptor, re+7, bytes-7) == -1){
                perror("write");
                exit(1);
            }
            if(write(file_descriptor, "\n", 1) == -1){
                perror("write");
                exit(1);
            }
            //printf("%.*s\n", bytes-7, re+7);
        }
    }
}

char* filter_path(char* buffer){ //this function filters extracts the path from the inotifywait notification
    char* token1, *token2, *token3, *re;
 
    token1 = strtok(buffer, " ");
    token2 = strtok(NULL, " ");
    token3 = strtok(NULL, " ");
    
    re = strcat(token1, token3); //skip the string that's in-between the two spaces
    re[strcspn(re, "\n")] = '\0'; //replace the new line with end of string
    return re; //this is the actual path
}

int buf_number_lines(char* string){ //this function counts the new lines of a buffer
    int i, counter = 0;
    for(i = 0; i <= strlen(string); i++){
        if(string[i] == '\n'){
            counter++;
        }
    }
    return counter;
}

char* read_line_from_buf(char* buf, int line){ //this function finds and returns the line-nth line of a buffer
    int counter = 1;
    char* eol, *buffer;
    int no = buf_number_lines(buf);
    buffer = buf;
    eol = (char*)memchr(buf, '\n', strlen(buf));
    buffer[eol-buffer] = '\0';
    while(counter<no && counter != line){
        eol++;
        buffer = eol;
        eol = (char*)memchr(buffer, '\n', strlen(buffer));
        buffer[eol-buffer] = '\0';
        counter++;
    }
    return buffer;
} 