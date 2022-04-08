//
// Created by YoungTr on 2022/4/4.
//
#include <string.h>
#include "zlib_example.h"
#include "log.h"
#include <stdio.h>
#include <errno.h>


FILE *fp;

void init_file(char *path) {
    LOGD("file path: %s", path);
    fp = fopen(path, "w+");
}


void compress_stream(char *source) {
    char dest[1024] = {0};
    size_t dest_len = sizeof(dest);

    size_t source_len = strlen(source);
    write_stream(source);

    LOGD("before compress len %d", source_len);
    compress((Bytef *) dest, &dest_len, (Bytef *) source, source_len);
    LOGD("after compress %s, len: %d", dest, dest_len);
//    write_stream(dest);

    char decode[source_len];
    uLongf decode_len;
    uncompress(decode, &decode_len, dest, dest_len);
    LOGD("after decode %s, len: %d", decode, decode_len);
//    write_stream(decode);
}

void uncompress_stream(char *dest) {
    char in[1024];
    fseek(fp, 0, SEEK_SET);
    size_t size = fread(in, sizeof(char), 1024, fp);
    LOGD("read %s, size %d", in, size);
    int dest_size;
    char s[35];
    uncompress(s, &dest_size, in, size);
    LOGD("after uncompress %s", s);

}


void write_stream(char *stream) {
    size_t size = strlen(stream);
    fwrite(stream, sizeof(char), size, fp);
    fflush(fp);
}
