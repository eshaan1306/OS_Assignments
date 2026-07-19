#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

int scheduler_pipe_write_fd;
int completed_job_count=0;
int scheduler_pid;

typedef struct{
    char name[256];
    pid_t pid;
    int completion_time;
    int wait_time; 
    int finished;       
} JobInfo;

JobInfo jobs[100];
int job_count=0;
int TSLICE;

typedef struct{
    char command[1024];
    pid_t pid;
    time_t start_time;
    double duration;
} HistoryEntry;

HistoryEntry history[100];
int indx=0;
int cnt=0;
char history_arr[100][1024];
void reap_zombie_jobs(){
    int status;
    pid_t terminated_pid;
    while ((terminated_pid=waitpid(-1, &status, WNOHANG))>0){
        for (int i=0; i<job_count; i++){
            if (jobs[i].pid==terminated_pid && jobs[i].finished==0){
                jobs[i].finished=1;
                jobs[i].completion_time=1;
                jobs[i].wait_time=0; 
                completed_job_count++;
            }
        }
    }
}

void my_handler(int signum){
    if (signum==SIGINT){
        while (completed_job_count<job_count){
            reap_zombie_jobs();
        }
        if (scheduler_pid>0){
            kill(scheduler_pid, SIGTERM);
            waitpid(scheduler_pid, NULL, 0);
        }
        printf("\n--- Scheduler Summary ---\n");
        printf("%-20s %-10s %-20s %-20s\n", "Job Name", "PID", "Completion Time (xTSLICE)", "Wait Time (xTSLICE)");
        for (int i=0; i<job_count; i++){
            int comp=jobs[i].completion_time ? jobs[i].completion_time : 1;
            printf("%-20s %-10d %-20d %-20d\n", jobs[i].name, jobs[i].pid, comp, jobs[i].wait_time);
        }
        printf("\n");
        close(scheduler_pipe_write_fd);
        exit(0);
    }
}

// assignment 2 handler
// void my_handler(int signum){
//     if (signum==SIGINT){
//         printf("\n--- Command History ---\n");
//         for (int j=0;j<indx;j++){
//             printf("[%d] PID=%d | Command=%s | Started=%s | Duration=%.3f sec\n",
//                    j+1,
//                    history[j].pid,
//                    history[j].command,
//                    ctime(&history[j].start_time),
//                    history[j].duration);
//         }
//         exit(0);
//     }
// }

void parse_args(char *cmd,char *args[]){
    if (cmd==NULL || args==NULL) {
        fprintf(stderr, "error, NULL argument\n");
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

int submit(char *cmd){
    char *args[100];
    char cmd_copy[1024];
    strncpy(cmd_copy, cmd, sizeof(cmd_copy));
    parse_args(cmd_copy, args);
    if (args[1]==NULL) {
        fprintf(stderr, "usage: submit <executable_path>\n");
        return 1;
    }

    pid_t job_pid=fork();
    if (job_pid<0){
        perror("fork for job failed");
        exit(1);
    }
    if (job_pid==0){
        execvp(args[1], &args[1]);
        perror("execvp for job failed");
        exit(1);
    } else{
        if (write(scheduler_pipe_write_fd, &job_pid, sizeof(pid_t))==-1){
            perror("failed to write to scheduler pipe");
        }
        strcpy(jobs[job_count].name, args[1]);
        jobs[job_count].pid=job_pid;
        jobs[job_count].completion_time=0;
        jobs[job_count].wait_time=0;
        jobs[job_count].finished=0;
        job_count++;
    }
    return 0;
}

int norm_exec(char *cmd){
    strncpy(history[indx].command, cmd, 1024);
    pid_t pid;
    int status;
    struct timeval start2, end;
    gettimeofday(&start2, NULL);
    pid=fork();
    if (pid<0){
        perror("fork failed");
        return -1;
    }
    else if (pid==0){
        char *args[100];
        parse_args(cmd, args);
        execvp(args[0], args);
        perror("exec failed");
        exit(1);
    }
    else{
        history[indx].pid=pid;
        history[indx].start_time=start2.tv_sec;
        wait(&status);
        gettimeofday(&end, NULL);
        history[indx].duration=(end.tv_sec-start2.tv_sec)+(end.tv_usec-start2.tv_usec)/1000000.0;
        if (WIFEXITED(status)){
            indx++;
            return 0;
        }else{
            perror("error executing process");
        }
    }
}


int launch(char *cmd){
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

int check(char *cmd){
    int count=0;
    for (int i=0; cmd[i]!='\0'; i++){
        if (cmd[i]=='|') count++;
    }

    if (count==0) return launch(cmd);
    else return launchpipe(cmd, count);
}

void history_call(){
    for (int k=0;k<cnt;k++){
        printf("Command %d: %s", k+1,history_arr[k]);
    }

}

void shell_loop(){
    char cmd[1024];
    while (1) {
        reap_zombie_jobs();
        printf("Enter your command:~$ ");
        if (fgets(cmd, sizeof(cmd), stdin)==NULL) {
            perror("Error reading cmd");
            break;
        }
        if (cmd[0]=='\n') continue;
        strcpy(history_arr[cnt++],cmd);
        if (strncmp(cmd,"submit ",7)==0){
            submit(cmd);
            continue;
        }
        if (cnt>=100 || indx>=100){
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

int main(int argc, char *argv[]) {
    if (argc!=3){
        fprintf(stderr,"Usage: %s <NCPU> <TSLICE_in_ms>\n",argv[0]);
        exit(1);
    }
    int ncpu=atoi(argv[1]);
    int tslice=atoi(argv[2]);
    int fd[2];
    if (pipe(fd)==-1){
        perror("pipe failed");
        exit(1);
    }
    pid_t scheduler_pid1=fork();
    if (scheduler_pid1<0){
        perror("fork for scheduler failed");
        exit(1);
    }
    if (scheduler_pid1==0){
        close(fd[1]);
        char ncpu_str[16];
        char tslice_str[16];
        char pipe_read_fd_str[16];
        sprintf(ncpu_str, "%d", ncpu);
        sprintf(tslice_str, "%d", tslice);
        sprintf(pipe_read_fd_str, "%d",fd[0]);
        execlp("./scheduler", "scheduler",ncpu_str,tslice_str,pipe_read_fd_str,NULL);
        perror("execlp for scheduler failed");
        exit(1);
    } else{
        close(fd[0]);
        scheduler_pid=scheduler_pid1;
        scheduler_pipe_write_fd=fd[1];
    }
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = my_handler;
    sigaction(SIGINT, &sig, NULL);
    shell_loop();
    return 0;
}
