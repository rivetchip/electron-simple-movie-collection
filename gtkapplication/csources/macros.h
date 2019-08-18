#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#ifdef  __cplusplus
    extern "C" {
#endif


#define array_size(A) (sizeof(A) / sizeof(A[0]))


#define MESSAGE(...) _message(__FILE__, __func__, __LINE__, __VA_ARGS__)

static void _message(const char *file, const char function[], long line, char *message_format, ...) {

    struct timeval tv;
    gettimeofday(&tv, NULL);

    char time_buf[9]; // end \0
    int64_t now = ((int64_t) tv.tv_sec * 1000000) + tv.tv_usec;
    time_t now_secs = (time_t) (now / 1000000);
    struct tm *now_tm = localtime(&now_secs);
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", now_tm);

    char message[] = "** \033[1;32mMessage\033[0m: \033[34m%s.%03d\033[0m: %s:%ld %s() => ";
    printf(message, time_buf, ((now / 1000) % 1000), file, line, function);

    va_list args;
    va_start(args, message_format);

    vprintf(message_format, args);
    printf("%s", "\n");

    va_end(args);
}


#ifdef  __cplusplus
    } // extern c
#endif
