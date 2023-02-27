//
// Created by YoungTr on 2023/2/27.
//

#include <stdio.h>
#include <stdlib.h>
#include <unwind.h>
#include <dlfcn.h>
#include "xcc_unwind.h"
#include "../log.h"

#define MAX_FRAMES 64

typedef struct {
    size_t frame_num;
    char *buf;
    size_t buf_len;
    uintptr_t prev_pc;
    uintptr_t prev_sp;
} unwind_clang_t;

static void *xcc_libc_support_memset(void *s, int c, size_t n) {
    char *p = (char *) s;

    while (n--)
        *p++ = (char) c;

    return s;
}


static _Unwind_Reason_Code unwind_clang_callback(struct _Unwind_Context *unw_ctx, void *arg) {
    unwind_clang_t *self = (unwind_clang_t *) arg;
    uintptr_t pc = _Unwind_GetIP(unw_ctx);
    uintptr_t sp = _Unwind_GetCFA(unw_ctx);

    Dl_info info;
    size_t len;
    // dladdr() - 获取某个地址的符号信息
    // 返回值
    // 如果指定的address 不在其中一个加载模块的范围内，则返回0 ；且不修改Dl_info 结构的内容。
    // 否则，将返回一个非零值，同时设置Dl_info 结构的字段。

    /**
     * dli_fname: 加载模块的文件名
     * dli_fbase: 加载模块的句柄，即基地址
     * dli_sname: 符号的名称
     * dli_saddr: 符号的实际地址
     */
    if (0 != dladdr((void *) pc, &info)) {
        LOGD("file: %s, symbal: %s", info.dli_fname, info.dli_sname);
        return _URC_NO_REASON;
    } else {
        LOGD("pc: %u, sp: %u", pc, sp);
        self->frame_num++;
        if (self->frame_num > 10) {
            return _URC_END_OF_STACK;
        }
        return _URC_NO_REASON;
    }


}

void test_unwind1() {
    LOGD("test_unwind1");
    unwind_get();
}

void test_unwind2() {
    test_unwind1();
}

void test_unwind3() {
    test_unwind2();
}

void test_unwind4() {
    test_unwind3();
}

void test_unwind() {
    test_unwind4();
}

void unwind_get() {
    unwind_clang_t self;
    xcc_libc_support_memset(&self, 0, sizeof(unwind_clang_t));
    _Unwind_Backtrace(unwind_clang_callback, &self);
}
