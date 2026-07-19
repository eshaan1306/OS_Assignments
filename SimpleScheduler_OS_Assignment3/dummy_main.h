#ifndef DUMMY_MAIN_H
#define DUMMY_MAIN_H
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
int dummy_main();
int main(){
    raise(SIGSTOP);
    return dummy_main();
}
#define main dummy_main

#endif
