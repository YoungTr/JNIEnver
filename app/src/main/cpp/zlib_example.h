//
// Created by YoungTr on 2022/4/2.
//

#ifndef JNIEVNER_ZLIB_EXAMPLE_H
#define JNIEVNER_ZLIB_EXAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <zlib.h>
#include <stdio.h>

void init_file(char *path);

void compress_stream(char *source);

void uncompress_stream(char *source);

void write_stream(char *stream);

#ifdef __cplusplus
}
#endif
#endif //JNIEVNER_ZLIB_EXAMPLE_H


