//
// Created by YoungTr on 2023/1/17.
//

#ifndef JNIEVNER_XCC_SIGNAL_H
#define JNIEVNER_XCC_SIGNAL_H

#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

int register_signal(void (*handler)(int, siginfo_t *, void *));

int register_signal2();

void testCrash();

#ifdef __cplusplus
}
#endif

#endif //JNIEVNER_XCC_SIGNAL_H
