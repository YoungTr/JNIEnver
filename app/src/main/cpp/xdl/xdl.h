//
// Created by YoungTr on 2022/10/26.
//

#ifndef JNIEVNER_XDL_H
#define JNIEVNER_XDL_H

#ifdef __cplusplus
extern "C" {
#endif

int xdl_iterate_by_maps(char *file_path);

int xdl_iterate_by_link(const char *file_path);

void *xdl_open(const char *filename);

void *xdl_close(void *handle);

void *xdl_sym(void *handle, const char *symbol);

#endif //JNIEVNER_XDL_H


#ifdef __cplusplus
}
#endif