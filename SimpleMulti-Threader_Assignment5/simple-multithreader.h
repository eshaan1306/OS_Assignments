#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <pthread.h> 
#include <chrono>       

int user_main(int argc, char **argv);

void demonstration(std::function<void()> && lambda){
  lambda();
}

typedef struct{
    int low;
    int high;
}thread_args1d;

typedef struct{
    int low1;
    int high1;
    int low2;
    int high2;
}thread_args2d;

std::function<void(int)> global_lambda_1d;

std::function<void(int, int)> global_lambda_2d;

void* thread_func_1d(void* ptr){
    thread_args1d* t=(thread_args1d*)ptr;
    for(int i=t->low; i<t->high; i++){
        global_lambda_1d(i);
    }
    return NULL;
}

void* thread_func_2d(void* ptr){
    thread_args2d* t=(thread_args2d*)ptr;
    for (int i=t->low1; i<t->high1; i++){
        for (int j=t->low2; j<t->high2; j++){
            global_lambda_2d(i, j);
        }
    }
    return NULL;
}


void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads)
{
    global_lambda_1d=lambda;
    auto start=std::chrono::high_resolution_clock::now();
    int total=high-low;
    int chunk=total/numThreads;
    pthread_t threads[numThreads];
    thread_args1d args[numThreads];
    for (int i=0; i<numThreads; i++){
        args[i].low=low+i*chunk;
        if (i==numThreads-1){
            args[i].high=high;
        } else{
            args[i].high=low+(i+1)*chunk;
        }
        pthread_create(&threads[i], NULL, thread_func_1d, (void*)&args[i]);
    }
    for (int i=0; i<numThreads; i++){
        pthread_join(threads[i], NULL);
    }
    auto end=std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff=end-start;
    std::cout << "[parallel_for 1D] execution time=" << diff.count() << " seconds\n";
}
void parallel_for(int low1, int high1, int low2, int high2, std::function<void(int,int)> &&lambda, int numThreads)
{
    global_lambda_2d=lambda;
    auto start=std::chrono::high_resolution_clock::now();
    int totalRows=high1-low1;
    int chunk=totalRows/numThreads;
    pthread_t threads[numThreads];
    thread_args2d args[numThreads];
    for (int t=0; t<numThreads; t++){
        args[t].low1=low1+t*chunk;
        if (t==numThreads-1)
            args[t].high1=high1;
        else
            args[t].high1=low1+(t+1)*chunk;
        args[t].low2 =low2;
        args[t].high2=high2;

        pthread_create(&threads[t], NULL, thread_func_2d, (void*)&args[t]);
    }
    for (int t=0; t<numThreads; t++){
        pthread_join(threads[t], NULL);
    }
    auto end=std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff=end-start;
    std::cout << "[parallel_for 2D] execution time=" << diff.count() << " seconds\n";
}

int main(int argc, char **argv){
    int x=5,y=1;
    auto lambda1=[x, &y](void){
    y=5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
    };
    demonstration(lambda1); 
    int rc=user_main(argc, argv);
    auto lambda2=[](){
        std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
    };
    demonstration(lambda2);
    return rc;
}

#define main user_main
