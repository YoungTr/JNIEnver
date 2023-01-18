//
// Created by YoungTr on 2023/1/17.
//

#include <stdlib.h>
#include <string.h>
#include "xcc_signal.h"
#include "xcc_errno.h"
#include "../log.h"

#define SIGNAL_CRASH_STACK_SIZE (1024 * 128)


typedef struct signal_crash_info {
    int signum;
    struct sigaction oldact;
} signal_crash_info_t;

static signal_crash_info_t signal_crash_infos[] = {
        {.signum = SIGABRT},
        {.signum = SIGBUS},
        {.signum = SIGFPE},
        {.signum = SIGILL},
        {.signum = SIGSEGV},
        {.signum = SIGTRAP},
        {.signum = SIGSYS},
        {.signum = SIGSTKFLT}
};

static int signal_crash_unregister(void) {
    int r = 0;
    size_t i;
    for (i = 0; i < sizeof(signal_crash_infos) / sizeof(signal_crash_infos[0]); i++) {
        if (0 != sigaction(signal_crash_infos[i].signum, &(signal_crash_infos[i].oldact), NULL)) {
            LOGD("signal crash unregister error");
            r = XCC_ERRNO_SYS;
        }
    }
    LOGD("signal crash unregister success");
    return r;
}

static void crash_signal_handler(int sig, siginfo_t *si, void *uc) {
    LOGD("signal handler receive: %d", sig);
    signal_crash_unregister();
}

int register_signal(void (*handler)(int, siginfo_t *, void *)) {
    stack_t ss;
    if (NULL == (ss.ss_sp = calloc(1, SIGNAL_CRASH_STACK_SIZE))) return XCC_ERRNO_NOMEM;
    ss.ss_size = SIGNAL_CRASH_STACK_SIZE;
    ss.ss_flags = 0;

    if (0 != sigaltstack(&ss, NULL)) return XCC_ERRNO_SYS;

    struct sigaction act;
    memset(&act, 0, sizeof(act));

    sigfillset(&act.sa_mask);
    act.sa_sigaction = handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;

    size_t i;
    for (i = 0; i < sizeof(signal_crash_infos) / sizeof(signal_crash_infos[0]); i++) {
        if (0 !=
            sigaction(signal_crash_infos[i].signum, &act, &(signal_crash_infos[i].oldact)))
            return XCC_ERRNO_SYS;
    }
    return 0;
}

int register_signal2() {
    return register_signal(crash_signal_handler);
}

void testCrash() {
    int b = 0;
    int a = 100;
    int v = 100 / 0;
    LOGD("testCrash value = %d", v);
}