#include <stdio.h> //perror
#include <stdlib.h> //exit
#include <string.h> //string manipulations
#include <unistd.h> //write _exit, close, dup2, execl, fork, pipe, STDOUT_FILENO, getcwd

void find_and_write_urls(char* line, int file_descriptor);

char* filter_path(char* buffer);

int buf_number_lines(char* string);

char* read_line_from_buf(char* buf, int line);