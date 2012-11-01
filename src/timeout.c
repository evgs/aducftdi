#include <stdint.h>

#include <stdio.h>

#ifdef __MINGW32_VERSION

#include <windows.h>

volatile static uint32_t endTime;

void setTimeout(int ms) {
  endTime =  GetTickCount() + ms;
}

int isTimeout() {
  int remaining = endTime - GetTickCount();
  //printf("%d\n", remaining);
  return remaining < 0;
};

void cancelTimeout() {};

#else

// POSIX
#include <signal.h>
#include <unistd.h>
#include <time.h>

static sig_atomic_t alarm_flag = 1;

static void alarm_handler(int arg){
    alarm_flag = 0;
}

void setTimeout(int ms) {
    //setup the signal handler for the alarm
    sigset(SIGALRM, alarm_handler);

    //setup the alarm flag
    alarm_flag = 1;

    //setup the alarm
    ualarm(ms*1000, 0);

}

int isTimeout() {
  return alarm_flag==0;
}

void cancelTimeout() {
  alarm(0);
}

#endif // __MINGW32_VERSION
