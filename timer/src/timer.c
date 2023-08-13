#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <signal.h>
#include <bits/sigaction.h>
#include <unistd.h>

#define  CLOCKID   CLOCK_REALTIME

void sig_handler(int signo)
{
    prctl(PR_SET_NAME, "sig_handler");
    printf ("timer_signal function! %d\n", signo);
}

void timer_thread(union sigval v)
{
    prctl(PR_SET_NAME, "timer_thread");
    printf ("timer_thread function! %d\n", v.sival_int);
}

//int main(void)
//{
//    timer_t timerid;
//    struct sigevent evp;
//    memset(&evp, 0, sizeof(struct sigevent));
//
//    evp.sigev_value.sival_int = 111;
//    evp.sigev_notify = SIGEV_THREAD;
//    evp.sigev_notify_function = timer_thread;
//
//    if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1) {
//        perror("fail to timer_create");
//        exit(-1);
//    }
//
//    struct itimerspec it;
//    it.it_interval.tv_sec = 1;
//    it.it_interval.tv_nsec = 0;
//
//    it.it_value.tv_sec = 3;
//    it.it_value.tv_nsec = 0;
//
//    if (timer_settime(timerid, 0, &it, NULL) == -1) {
//        perror("fail to timer_settime");
//        exit(-1);
//    }
//    while (1);
//    return 0;
//}

int main()
{
    timer_t timerid;
    struct sigevent evp;

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = sig_handler;
    act.sa_flags = 0;

    sigemptyset(&act.sa_mask);

    if (sigaction(SIGUSR1, &act, NULL) == -1) {
        perror("fail to sigaction");
        exit(-1);
    }

    memset(&evp, 0, sizeof(struct sigevent));
    evp.sigev_signo = SIGUSR1;
    evp.sigev_notify = SIGEV_SIGNAL;
    if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1) {
        perror("fail to timer_create");
        exit(-1);
    }

    struct itimerspec it;
    it.it_interval.tv_sec = 2;
    it.it_interval.tv_nsec = 0;
    it.it_value.tv_sec = 10;
    it.it_value.tv_nsec = 0;

    if (timer_settime(timerid, 0, &it, 0) == -1) {
        perror("fail to timer_settime");
        exit(-1);
    }

//    pause();
    while (1);
    return 0;
}