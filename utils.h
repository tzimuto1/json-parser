#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#ifdef DEBUG
    #define PRINTLN(format, ...) printf("Func: %s ," format "\n", __func__, ## __VA_ARGS__)
    #define LOGFUNC()            printf("Enter func: %s\n", __func__)
#else
    #define PRINTLN(format, ...)
    #define LOGFUNC()
#endif



#endif // UTILS_H