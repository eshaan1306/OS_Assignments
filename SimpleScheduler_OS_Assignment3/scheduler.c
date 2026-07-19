#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

void my_handler(int signum){
    if (signum==SIGTERM){
        exit(1);
    }
}

struct node{
    int data;
    struct node* next;
};

typedef struct queue{
    struct node* front;
    struct node* rear;
    int count;
}ReadyQueue;

ReadyQueue* createQueue(){
    ReadyQueue* q=(ReadyQueue*)malloc(sizeof(ReadyQueue));
    q->front=NULL;
    q->rear=NULL;
    q->count=0;
    return q;
}

void enqueue(ReadyQueue* q,int val){
    struct node* newNode=(struct node*)malloc(sizeof(struct node));
    newNode->data=val;
    newNode->next=NULL;
    if (q->rear==NULL){
        q->front=q->rear=newNode;
    } else{
        q->rear->next=newNode;
        q->rear=newNode;
    }
    q->count++;
}

int dequeue(ReadyQueue* q){
    if (q->front==NULL){
        return -1;
    }
    struct node* temp=q->front;
    int val=temp->data;
    q->front=q->front->next;
    if (q->front==NULL){
        q->rear=NULL;
    }
    free(temp);
    q->count--;
    return val;
}

void freeQueue(ReadyQueue* q){
    while (q->front!=NULL){
        dequeue(q);
    }
    free(q);
}

void stop_and_requeue_jobs(pid_t running_jobs[],int* running_count,ReadyQueue* q){
    int current_running=*running_count;
    *running_count=0;
    for (int i=0;i<current_running;i++){
        pid_t pid=running_jobs[i];
        if (kill(pid, SIGSTOP)==0){
            enqueue(q, pid);
        } else{
            if (errno!=ESRCH){
                perror("some process terminated due to error");
                exit(1);
            }
        }
    }
}

void check_for_new_jobs(int pipe_fd,ReadyQueue* q){
    pid_t new_job_pid;
    while (read(pipe_fd, &new_job_pid, sizeof(pid_t))>0){
        enqueue(q, new_job_pid);
    }
}

void schedule_and_run_jobs(pid_t running_jobs[],int* running_count, int ncpu, ReadyQueue* q){
    while (*running_count<ncpu && q->count>0){
        pid_t pid_to_run=dequeue(q);
        if (pid_to_run!=-1) {
            running_jobs[*running_count]=pid_to_run;
            (*running_count)++;
            kill(pid_to_run, SIGCONT);
        }
        else{
            perror("invalid pid");
            exit(1);
        }
    }
}

int main(int argc, char *argv[]){
    if (argc!=4){
        fprintf(stderr, "scheduler internal error: incorrect arguments\n");
        exit(1);
    }
    int ncpu=atoi(argv[1]);
    int tslice_ms=atoi(argv[2]);
    int pipe_read_fd=atoi(argv[3]);
    pid_t running_jobs[ncpu];
    int running_count=0;
    ReadyQueue* readyQueue=createQueue();
    fcntl(pipe_read_fd, F_SETFL, O_NONBLOCK);
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = my_handler;
    sigaction(SIGINT, &sig, NULL);
    while (1){
        usleep(tslice_ms*1000);
        stop_and_requeue_jobs(running_jobs, &running_count, readyQueue);
        check_for_new_jobs(pipe_read_fd, readyQueue);
        schedule_and_run_jobs(running_jobs, &running_count, ncpu, readyQueue);
    }
    
    return 0;
}
