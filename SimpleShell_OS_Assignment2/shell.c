#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
typedef struct {
    char command[1024];  
    pid_t pid;                  
    time_t start_time;          
    double duration;            
} HistoryEntry;

HistoryEntry history[100];
int indx=0;
int cnt=0;

char history_arr[100][1024];


void my_handler(int signum){
    if (signum==SIGINT){
        printf("\n--- Command History ---\n");
        for (int j=0;j<indx;j++){
            printf("[%d] PID=%d | Command=%s | Started=%s | Duration=%.3f sec\n",
                   j+1,
                   history[j].pid,
                   history[j].command,
                   ctime(&history[j].start_time),
                   history[j].duration);
        }
        exit(0);
    }
}

void parse_args(char *cmd,char *args[]){
    if (cmd == NULL || args == NULL) {
        fprintf(stderr, "Error, NULL argument\n");
        exit(1);
    }
    int i=0;
    while (*cmd==' ')cmd++;
    char *token=strtok(cmd," \n");
    while (token!=NULL){
        args[i++]=token;
        token=strtok(NULL," \n");
    }
    args[i]=NULL;
}

int norm_exec(char *cmd) {
    strncpy(history[indx].command, cmd, 1024);
    pid_t pid;
    int status;
    struct timeval start2, end;

    gettimeofday(&start2, NULL);
    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    else if (pid == 0) {
        char *args[100];
        parse_args(cmd, args);
        execvp(args[0], args);
        perror("exec failed");
        exit(1);
    }
    else {
        history[indx].pid = pid;
        history[indx].start_time = start2.tv_sec;

        wait(&status);
        gettimeofday(&end, NULL); 
        history[indx].duration = (end.tv_sec-start2.tv_sec)+(end.tv_usec-start2.tv_usec)/1000000.0;
        if (WIFEXITED(status)){
            indx++;
            return 0;
        }else{
            perror("error executing process");
        }
    }
}


int launch(char *cmd) {
    return norm_exec(cmd);
}


int launchpipe(char *cmd, int count){
    char *subcmds[count+1];
    int i=0;
    char *token=strtok(cmd, "|");
    while (token!=NULL && i<=count){
        subcmds[i++]=token;
        token=strtok(NULL,"|");
    }
    int fd[count][2];
    for (i=0;i<count;i++){
        if (pipe(fd[i])<0){
            perror("pipe failed");
            return -1;
        }
    }
    for (i=0;i<=count;i++){
        struct timeval start2,end;
        gettimeofday(&start2, NULL);
        pid_t pid=fork();
        if (pid<0){
            perror("fork failed");
            return -1;
        }
        else if (pid==0){
            if (i==0){
                close(fd[i][0]);
                dup2(fd[i][1],STDOUT_FILENO);
                close(fd[i][1]);
            }else if (i==count){
                close(fd[i-1][1]);
                dup2(fd[i-1][0],STDIN_FILENO);
                close(fd[i-1][0]);
            }else{
                close(fd[i-1][1]);
                close(fd[i][0]);
                dup2(fd[i-1][0],STDIN_FILENO);
                dup2(fd[i][1],STDOUT_FILENO);
                close(fd[i-1][0]);
                close(fd[i][1]);
            }
            for (int j=0;j<count;j++){
                close(fd[j][0]);
                close(fd[j][1]);
            }
            char *args[100];
            parse_args(subcmds[i],args);
            execvp(args[0],args);
            perror("exec failed");
            exit(1);
        }
        else{
            history[indx].pid=pid;
            history[indx].start_time=start2.tv_sec;
            strncpy(history[indx].command,subcmds[i], 1024);
            indx++;
        }
    }
    for (i=0;i<count;i++){
        close(fd[i][0]);
        close(fd[i][1]);
    }
    for (i=0;i<=count;i++){
        int status;
        struct timeval end;
        wait(&status);
        gettimeofday(&end,NULL);
        history[indx-1].duration=(end.tv_sec-history[indx-1].start_time)+
                                    (end.tv_usec-0)/1000000.0;
    }
    return 0;
}

int check(char *cmd) {
    int count = 0;
    for (int i = 0; cmd[i] != '\0'; i++) {
        if (cmd[i] == '|') count++;
    }

    if (count == 0) return launch(cmd);
    else return launchpipe(cmd, count);
}

void history_call(){
    for (int k=0;k<cnt;k++){
        printf("Command %d: %s", k+1,history_arr[k]);
    }

}

void shell_loop() {
    char cmd[1024];
    while (1) {
        printf("Enter your command:~$ ");
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            perror("Error reading cmd");
            break;
        }
        if (cmd[0] == '\n') continue;
        strcpy(history_arr[cnt++],cmd);
	if (cnt>=100 || indx>=100) {
    	    printf("History full, Can't store more commands.\n");
     	    continue;
	}
        if (strncmp(cmd,"history\n", 8)==0){
            history_call();
            continue;
        }
        check(cmd);
    }
}

int main() {
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = my_handler;
    sigaction(SIGINT, &sig, NULL);   
    shell_loop();
    return 0;
}
