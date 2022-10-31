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
#include <stdbool.h>
#include <malloc.h>
#include "xdl.h"
#include "../xcd_log.h"
#include "../log.h"

typedef struct xdl {
    char *pathname;
    uintptr_t load_bias;
    const ElfW(Phdr) *dlpi_phdr;
    ElfW(Half) dlpi_phnum;

    struct xdl *next;

    bool dynsym_try_load;
    ElfW(Sym) *dynsym;
    const char *dynstr;

    // .hash (SYSV hash for .dynstr)
    struct {
        const uint32_t *buckets;
        uint32_t buckets_cnt;
        const uint32_t *chains;
        uint32_t chains_cnt;
    } sysv_hash;

    // .gnu.hash (GNU hash for .dynstr)
    struct {
        const uint32_t *buckets;
        uint32_t buckets_cnt;
        const uint32_t *chains;
        uint32_t symoffset;
        const ElfW(Addr) *bloom;
        uint32_t bloom_cnt;
        uint32_t bloom_shift;
    } gnu_hash;


} xdl_t;

size_t xdl_util_trim_ending(char *start) {
    char *end = start + strlen(start);
    while (start < end && isspace((int) (*(end - 1)))) {
        end--;
        *end = '\0';
    }
    return (size_t) (end - start);
}


int xdl_iterate_by_maps(char *file_path) {

    FILE *fp = fopen("/proc/self/maps", "r");
    if (NULL == fp) return -1;

    int r;
    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        XCD_LOG_DEBUG("%s", line);
        uintptr_t base, offset;
        if (2 !=
            sscanf(line, "%" SCNxPTR "-%*" SCNxPTR " r%*cxp %" SCNxPTR " ", &base, &offset))
            continue;
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
    char *filename = (char *) data;

    // info->dlpi_name name of object
    XCD_LOG_DEBUG("Name: \"%s\" (%d segments)", info->dlpi_name, info->dlpi_phnum);
    for (int j = 0; j < info->dlpi_phnum; ++j) {
        p_type = info->dlpi_phdr[j].p_type;
        type = (p_type == PT_LOAD) ? "PT_LOAD" :
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

bool xdl_util_ends_with(const char *str, const char *ending) {
    size_t str_len = strlen(str);
    size_t ending_len = strlen(ending);

    if (ending_len > str_len) return false;

    return 0 == strcmp(str + (str_len - ending_len), ending);
}

static int xdl_iterate_callback(struct dl_phdr_info *info, size_t size, void *data) {
    uintptr_t *pkg = (uintptr_t *) data;
    xdl_t **self = (xdl_t **) *pkg++;
    const char *filename = (const char *) *pkg;
    (void) size;

    // ignore invalid ELF
    if (0 == info->dlpi_addr || NULL == info->dlpi_name || '\0' == info->dlpi_name[0]) return 0;
    // check pathname
    if ('/' == filename[0]) {
        if ('/' == info->dlpi_name[0]) {
            if (0 != strcmp(filename, info->dlpi_name)) return 0;
        } else {
            if (!xdl_util_ends_with(filename, info->dlpi_name)) return 0;
        }
    } else {
        if ('/' == info->dlpi_name[0]) {
            if (!xdl_util_ends_with(info->dlpi_name, filename)) return 0;
        } else {
            if (0 != strcmp(info->dlpi_name, filename)) return 0;
        }
    }
    // we found the target ELF
    LOGD("we found : %s, bias: 0x%x", info->dlpi_name, info->dlpi_addr);
    if (NULL == ((*self) = calloc(1, sizeof(xdl_t)))) return 1; // failed
    if (NULL == ((*self)->pathname = strdup(info->dlpi_name))) {
        free(*self);
        *self = NULL;
        return 1; // failed
    }

    (*self)->load_bias = info->dlpi_addr;
    (*self)->dlpi_phdr = info->dlpi_phdr;
    (*self)->dlpi_phnum = info->dlpi_phnum;
    (*self)->dynsym_try_load = false;
    return 1;
}

static int xdl_dynsym_load(xdl_t *self) {
    // find the dynamic segment
    ElfW(Dyn) *dynamic = NULL;
    for (size_t i = 0; i < self->dlpi_phnum; ++i) {
        const ElfW(Phdr) *phdr = &(self->dlpi_phdr[i]);
        if (PT_DYNAMIC == phdr->p_type) {
            dynamic = (ElfW(Dyn) *) (self->load_bias = phdr->p_vaddr);
            break;
        }

    }

    if (NULL == dynamic) return -1;

    // iterate the dynamic segment
    for (ElfW(Dyn) *entry = dynamic; entry && entry->d_tag != DT_NULL; entry++) {
        switch (entry->d_tag) {
            case DT_SYMTAB: // .dynsym
                self->dynsym = (ElfW(Sym) *) (self->load_bias + entry->d_un.d_ptr);
                break;
            case DT_STRTAB: // .dynstr
                self->dynstr = (const char *) (self->load_bias + entry->d_un.d_ptr);
                break;
            case DT_HASH:
                self->sysv_hash.buckets_cnt = ((const uint32_t *)(self->load_bias + entry->d_un.d_ptr))[0];
                self->sysv_hash.chains_cnt = ((const uint32_t *)(self->load_bias + entry->d_un.d_ptr))[1];
                self->sysv_hash.buckets = &(((const uint32_t *)(self->load_bias + entry->d_un.d_ptr))[2]);
                self->sysv_hash.chains = &(self->sysv_hash.buckets[self->sysv_hash.buckets_cnt]);
                break;
            case DT_GNU_HASH:  //.gnu.hash
                self->gnu_hash.buckets_cnt = ((const uint32_t *)(self->load_bias + entry->d_un.d_ptr))[0];
                self->gnu_hash.symoffset = ((const uint32_t *)(self->load_bias + entry->d_un.d_ptr))[1];
                self->gnu_hash.bloom_cnt = ((const uint32_t *)(self->load_bias + entry->d_un.d_ptr))[2];
                self->gnu_hash.bloom_shift = ((const uint32_t *)(self->load_bias + entry->d_un.d_ptr))[3];
                self->gnu_hash.bloom = (const ElfW(Addr) *)(self->load_bias + entry->d_un.d_ptr + 16);
                self->gnu_hash.buckets = (const uint32_t *)(&(self->gnu_hash.bloom[self->gnu_hash.bloom_cnt]));
                self->gnu_hash.chains = (const uint32_t *)(&(self->gnu_hash.buckets[self->gnu_hash.buckets_cnt]));
                break;
            default:
                break;
        }
    }
    // check
    if (NULL == self->dynstr || NULL == self->dynsym
        || 0 == self->sysv_hash.buckets_cnt
        || 0 == self->gnu_hash.buckets_cnt) {
        self->dynstr = NULL;
        self->dynsym = NULL;
        self->sysv_hash.buckets_cnt = 0;
        self->gnu_hash.buckets_cnt = 0;
        return -1;
    }
    return 0;

}

int xdl_iterate_by_link(const char *file_path) {
    xdl_t *self;
    uintptr_t pkg[2] = {(uintptr_t) &self, (uintptr_t) file_path};
    dl_iterate_phdr(callback, (void *) pkg);
    return 0;
}

void *xdl_open(const char *filename) {
    xdl_t *self;
    uintptr_t pkg[2] = {(uintptr_t) &self, (uintptr_t) filename};
    dl_iterate_phdr(xdl_iterate_callback, (void *) pkg);
    return self;
}

void *xdl_close(void *handle) {

}

void *xdl_sym(void *handle, const char *symbol) {
    if (NULL == handle || NULL == symbol) return NULL;

    xdl_t *self = (xdl_t *) handle;
    // load .dynsym only once
    if (!self->dynsym_try_load) {
        self->dynsym_try_load = true;
        if (0 != xdl_dynsym_load(self)) return NULL;
    }
}