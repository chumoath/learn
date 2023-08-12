#include <stdio.h>
#include <signal.h>

extern int add(int a, int b);
extern void sig_handler(int signo);

int main(int argc, char *argv[])
{
    signal(SIGSEGV, sig_handler);
    add(1, 2);
    return 0;
}