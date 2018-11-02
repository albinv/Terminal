#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <sched.h>
#include <fcntl.h> 
#include <time.h>

int clone_child(void *arg){
    char *cmd = (char *)arg;
    if (strstr(cmd, "cd") != NULL){
        chdir((cmd+3));
    }
    else{
        execl("/bin/sh", "sh", "-c", cmd, (char *) 0);
        _exit(127);
    }
}

int my_system(char *cmd){
    int status;
    pid_t pid;

    #ifdef FORK
        if (cmd == 0)
            return(1);

        if ((pid = fork()) == 0) {
            execl("/bin/sh", "sh", "-c", cmd, (char *) 0);
            _exit(127);
        }
        if (pid == -1) {
            status = -1;
        }

        waitpid(pid, &status, 0);

        return status;

    #elif VFORK
        if (cmd == 0)
            return(1);

        if ((pid = vfork()) == 0) {
            execl("/bin/sh", "sh", "-c", cmd, (char *) 0);
            _exit(127);
        }
        if (pid == -1) {
            status = -1;
        }

        waitpid(pid, &status, 0);

        return status;

    #elif CLONE
        char *stack;
        char *stackHead;
        int stackSize = 1024 * 1024;

        stack = (char *) malloc(stackSize);

        if (!stack){
        printf("Unable to create stack!!");
            status = -1;
        }

        stackHead = stack + stackSize -1;

        pid = clone(clone_child, stackHead, CLONE_FS | SIGCHLD, cmd);

        if (pid == -1){
            free(stack);
            status = -1;
        }

        waitpid(pid, &status, 0) ;

        return status;

    #elif PIPE
	if ((pid = fork()) == 0){
	    execl("/bin/sh", "sh", "-c", cmd, (char *) 0);
            close(0);
	    close(1);
	    _exit(127);
	}
	if (pid == -1) {
            status = -1;
        }

        waitpid(pid, &status, 0);
	

    #else
        printf("\nPlease specify the mode you would like to run my_system in...\n");

    #endif
}


int tshell(int argc, char** argv){
    
    clock_t start = clock(), diff;
    while (1){
	clock_t begin = clock();
        char line[500];
        scanf("%[^\n]s", line);
        if (strlen(line) > 1){
	    #ifdef PIPE
	        int status;
	        pid_t pid;
		if (strstr(argv[2],"0")){
		    if((pid = fork()) == 0){
            	    my_system(strcat(strcat(line, " "), argv[1]));
               	    close(0);
                    close(1);
                    _exit(127);
          	    }
          	    else{
		        status = -1;
		    }	
		}
		else if(strstr(argv[2], "1")){
		    if((pid = fork()) == 0){
		        int fd = open (argv[1], O_RDWR, S_IRUSR | S_IWUSR);
			close(1);
			dup(fd);
			close(fd);
			my_system(line);
			close(0);
			close(1);
			_exit(127);
		    }
		}
		else{
		    status = -1;
		}
		waitpid(pid, &status, 0);
	    #else
		my_system(line);
	    #endif
	}
	else{
	    return 0;
	}

	if(strstr(line, "exit") != NULL){
	    diff = clock() - start;
            int msec = diff / CLOCKS_PER_SEC;
            printf("Time taken %f seconds %d milliseconds", msec, msec);

	    clock_t end = clock();
    	    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
            printf("Total time to execute is : %f",time_spent);    
	    return 0;
	}
	while ((getchar()) != '\n');
	memset(line, 0, sizeof line);
    }
    
}


int main (int argc, char** argv){
    
    tshell(argc, argv);
    return 0;
}
