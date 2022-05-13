#include <errno.h> // errno/EINTR
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> //mmap
#include <sys/wait.h> //wait, pid_t
#include <limits.h>
#include <fcntl.h>
#include <queue> //queue
#include <vector> //vector

#include "read_functions.h" //include stdio, stdlib, string.h, unistd.h

int main(void){
    int id, status, listener_fd[2]; 
    char cwd[PATH_MAX];

    if (pipe(listener_fd) == -1) {
        perror("pipe");
        exit(1);
    }   
    
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd()");
        exit(1);
    } 
    id = fork();

    /*----------------LISTENER----------------*/
    if(id == 0){   
        if((dup2(listener_fd[1], STDOUT_FILENO) == -1) && (errno == EINTR)) exit(1);
        close(listener_fd[1]);
        close(listener_fd[0]);
        execlp("inotifywait", "-r", "-q", "-m", "-e", "moved_to", "-e", "create", "--exclude", ".(out|pipe)", cwd, NULL); //monitoring directory
        perror("execlp");
        _exit(1);
    }
    /*----------------LISTENER----------------*/
    
    close(listener_fd[1]);
    char buffer[4096];
    std::queue<int> worker_q;
    std::vector<int> worker_pids;
    int* line_number = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    if(mkfifo("hydra.pipe", 0777) == -1 && errno != EEXIST){ //making the named pipe
        perror("open");
        exit(1);
    }
    while(1){
        printf("Listening...\n");
        ssize_t count = read(listener_fd[0], buffer, sizeof(buffer));
        if (count == -1) {
            if (errno != EINTR){
                perror("listener read");
                exit(1);
            } 
        } 
        else if (count == 0) break;
        else{ //if you read something from the listener
            *line_number = 1; //this is how workers know what line to read from the listener output, it gets incremented every time a child/worker from the buffer
            int no_of_files = buf_number_lines(buffer); //count the lines of the buffer, every line equals to a file name
            printf("Read %s from the listener\n", buffer);

            int no_of_forks = no_of_files - worker_q.size(); //the number of needed forks is equal to how many more files there are than sleeping workers

            if(no_of_forks >= 0){ //if there is not enough, or exactly enough, workers available 
                while(!worker_q.empty()){ //wake up all the sleeping workers
                    printf("Waking up a worker the the job\n");
                    kill(worker_q.front(), SIGCONT); //signal to continue
                    printf("Worker %d up\n", worker_q.front());
                    worker_q.pop(); //remove from worker queue
                }
                printf("I am about to fork %d times\n", no_of_forks);
                for(int i = 0; i < no_of_forks; i++){ //loop forks for all the extra files
                    id = fork();    //make a new worker
                    if(id == 0) break; //break the loop if you are a new worker
                    printf("Forked %d, saving the PID into the vector.\n", id);
                    worker_pids.push_back(id); //saved worker pid
                }
            }
            else{   //else just wake up a worker for every file
                for(int i = 0; i < no_of_files; i++){
                    printf("Waking up a worker the the job\n");
                    kill(worker_q.front(), SIGCONT);
                    printf("Worker %d up\n", worker_q.front());
                    worker_q.pop();
                }
            }
            /*----------------MANAGER----------------*/
            if(id != 0){
                printf("Manager is managing...\n");
                int w, fd;
                for(int i = 0; i < no_of_files; i++){ //for every lfile that will need to be read by a worker
                    fd = open("hydra.pipe", O_WRONLY); //open the pipe
                    if(fd == -1){
                        perror("manager fifo open");
                    }
                    printf("Manager opened fifo\n");
                    if(write(fd, buffer, sizeof(buffer)) == -1){ //write what the listener gave you (I do this for every worker since the read is blocked till the write)
                        perror("manager write");
                        exit(1);
                    }
                    printf("Manager wrote %s\n", buffer);
                    close(fd);
                    if((w = waitpid(0, &status, WUNTRACED)) > 0){ //get the id of the process that has stopped
                        printf("The process with PID %d has stopped, pushing it into the queue\n", w);
                        worker_q.push(w);               //push it into the worker queue
                    }
                }
            }
            /*----------------MANAGER----------------*/
            /*----------------WORKER----------------*/
            else{
                printf("Worker is working...\n");
                char buf[4096]; //the buffer for reading what the manager wrote on the named pipe
                char file_buf[4096]; //reading from the wanted file 4K at the time
                char append[] = ".out"; //worker will append this to the name of the file he will read
                char* file, *out_name; //the full name of the file the worker will need to read and the name of the output file
                int i, outfp, fp, r_no = 1; //file descriptors and counters
                while(1){
                    //GETTING THE PATH AND NAME OF THE FILE
                    printf("In worker while\n");
                    int fd = open("hydra.pipe", O_RDONLY);
                    if(fd == -1){
                        perror("worker fifo open");
                    }
                    printf("Worker opened fifo\n");
                    if(read(fd, buf, sizeof(buf)) == -1){
                        perror("worker read fifo");
                        exit(1);
                    }
                    close(fd);
                    file = read_line_from_buf(buf, *line_number); //reading the correct line
                    printf("Worker read %s\n", file);
                    (*line_number)++; //increasing the line for all the other workers, if there are multiple files written from the listener at the same buffer

                    file = filter_path(file); //getting the path name
                    printf("Worker has to work on file %s\n", file);

                    //EXTRACTING URLS OF THE FILE
                    fp = open(file, O_RDWR); //get the file descriptor
                    if(fp == -1){ //standard error check
                        perror("worker file open");
                        exit(1);
                    }
                    printf("Worker opened file\n");
                    out_name = strcat(file, append); //create the out name for writting
                    outfp = open(out_name, O_CREAT | O_RDWR| O_APPEND, 0777); //open or create it with full privilledges
                    if(outfp == -1){ //standard error check
                        perror("worker output file open");
                        exit(1);
                    }
                    while(1){ //while you are still reading something from the lines continue
                        r_no = read(fp, file_buf, sizeof(file_buf)); //reading 4K at a time
                        if(r_no  == -1){
                            perror("worker read file");
                            exit(1);
                        }
                        else if(r_no == 0){
                            break; //if you reach the end and read nothing break
                        }
                        printf("Worker read %d bytes from file\n", r_no);
                        find_and_write_urls(file_buf, outfp); //get all the URLs from the buffer and write them in your output file
                        memset(file_buf, 0, strlen(file_buf)); //resetting buffer //reset the buffer so that you don't get any overlapping buffer memory
                    }
                    close(outfp); //closing file descriptors
                    close(fp);
                    printf("Worker is done here, about to signal stop\n");
                    kill(getpid(), SIGSTOP); //stop yourself till the manager calls you again
                }
            }
            /*----------------WORKER----------------*/    
        }
        memset(buffer, 0, strlen(buffer)); //resetting the initial buffer, the listener output
    }
    close(listener_fd[0]); //close the read end of the listener
    wait(NULL); //wait for all the processes
}