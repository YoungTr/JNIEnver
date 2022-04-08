//
// Created by YoungTr on 2022/4/5.
//

#ifndef JNIEVNER_ZLIB_UTIL_H
#define JNIEVNER_ZLIB_UTIL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>

/**
* Compress
* @param source
* @param dest
* @param level
* @return
*/
int def(FILE *source, FILE *dest, int level);

/**
 * Decompress
 * @param source
 * @param dest
 * @return
 */
int inf(FILE *source, FILE *dest);


#ifdef __cplusplus
}
#endif
#endif //JNIEVNER_ZLIB_UTIL_H
