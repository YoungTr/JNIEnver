//
// Created by YoungTr on 2022/10/26.
//

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <elf.h>
#include <unistd.h>
#include <ctype.h>
#include <link.h>
#include "xdl.h"
#include "../xcd_log.h"

size_t xdl_util_trim_ending(char *start) {
    char *end = start + strlen(start);
    while (start < end && isspace((int)(*(end - 1)))) {
        end--;
        *end = '\0';
    }
    return (size_t)(end - start);
}


int xdl_iterate_by_maps(char *file_path) {

    FILE *fp = fopen("/proc/self/maps", "r");
    if (NULL == fp) return -1;

    int r;
    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        XCD_LOG_DEBUG("%s", line);
        uintptr_t base, offset;
        if (2 != sscanf(line, "%" SCNxPTR "-%*" SCNxPTR " r%*cxp %" SCNxPTR " ", &base, &offset)) continue;
        if (0 != offset) continue;
        if (memcmp((void *) base, ELFMAG, SELFMAG)) continue;

        // 该函数返回在字符串 str 中第一次出现字符 c 的位置
        char *pathname = strchr(line, '/');
        if (NULL == pathname) continue;
        XCD_LOG_DEBUG("%s", pathname);
        xdl_util_trim_ending(pathname);
        if (0 != memcmp(file_path, pathname, strlen(file_path))) continue;
        // we found the elf
        XCD_LOG_DEBUG("%s", line);

    }

    fclose(fp);
    return 0;
}

static int callback(struct dl_phdr_info *info, size_t size, void *data) {
    char *type;
    uint32_t p_type;
    char *filename = (char *)data;
    XCD_LOG_DEBUG("size: %d", size);

    // info->dlpi_name name of object
    XCD_LOG_DEBUG("Name: \"%s\" (%d segments)", info->dlpi_name, info->dlpi_phnum);
    for (int j = 0; j < info->dlpi_phnum; ++j) {
        p_type = info->dlpi_phdr[j].p_type;
        type =  (p_type == PT_LOAD) ? "PT_LOAD" :
                (p_type == PT_DYNAMIC) ? "PT_DYNAMIC" :
                (p_type == PT_INTERP) ? "PT_INTERP" :
                (p_type == PT_NOTE) ? "PT_NOTE" :
                (p_type == PT_INTERP) ? "PT_INTERP" :
                (p_type == PT_PHDR) ? "PT_PHDR" :
                (p_type == PT_TLS) ? "PT_TLS" :
                (p_type == PT_GNU_EH_FRAME) ? "PT_GNU_EH_FRAME" :
                (p_type == PT_GNU_STACK) ? "PT_GNU_STACK" :
                (p_type == PT_GNU_RELRO) ? "PT_GNU_RELRO" : NULL;

        XCD_LOG_DEBUG("    %2d: [%14p; memsz:%7jx] flags: %#jx; %s", j,
               (void *) (info->dlpi_addr + info->dlpi_phdr[j].p_vaddr),
               (uintmax_t) info->dlpi_phdr[j].p_memsz,
               (uintmax_t) info->dlpi_phdr[j].p_flags, type);
        if (type != NULL)
            printf("%s\n", type);
        else
            printf("[other (%#x)]\n", p_type);
    }
    return 0;

}

int xdl_iterate_by_link(const char *file_path) {
    dl_iterate_phdr(callback, (void *)file_path);
    return 0;
}