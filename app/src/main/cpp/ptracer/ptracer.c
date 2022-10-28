//
// Created by YoungTr on 2022/10/14.
//

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <unistd.h>
#include "ptracer.h"
#include "../xcd_log.h"
#include <unwind.h>

void test1() {
    pid_t child;
    long origin_eax;
    XCD_LOG_DEBUG("Test1");
    child = fork();
    if (child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls", "ls", NULL);
    } else {
        wait(NULL);
        XCD_LOG_DEBUG("The child made a system call %ld\n", 1L);
        origin_eax = ptrace(PT_READ_U, child, 4 * 12, NULL);
        XCD_LOG_DEBUG("The child made a system call %ld\n", origin_eax);
        ptrace(PTRACE_CONT, child, NULL, NULL);
    }
}




struct sigaction act_old;


static _Unwind_Reason_Code unwind_backtrace_callback(struct _Unwind_Context* context, void* arg) {

    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        XCD_LOG_DEBUG("unwind got pc ...0x%x\n", pc);
    }

    return _URC_NO_REASON;
}

ssize_t unwind_backtrace() {

    _Unwind_Reason_Code rc = _Unwind_Backtrace(unwind_backtrace_callback, 0);

    return rc == _URC_END_OF_STACK ? 0 : -1;
}

void func_1() {
    int ret = unwind_backtrace();
    XCD_LOG_DEBUG("unwind_backtrace return ...%d\n", ret);
}

void func_2() {
    func_1();
}

static void crash_handler_more(int sig, struct siginfo* info, void* buf) {

    unwind_backtrace();

    sigaction(sig, &act_old, 0);
}

void initCrashHandler() {
    struct sigaction act;
    act.sa_sigaction = crash_handler_more;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGKILL, &act, 0);
    sigaction(SIGINT, &act, 0);
    sigaction(SIGQUIT, &act, 0);
    sigaction(SIGILL, &act, 0);
    sigaction(SIGABRT, &act, 0);
    sigaction(SIGBUS, &act, 0);
    sigaction(SIGSEGV, &act, &act_old);
}

void triggerCrash() {
    char *p = 0;
    p[100] = 'a';
}

void test2() {
    initCrashHandler();
    func_2();

    triggerCrash();

}