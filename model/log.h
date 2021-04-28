#pragma once
#ifndef LOG_H
#define LOG_H
#include <stdarg.h>
#ifdef __GNUC__
typedef unsigned long long UINT64;
typedef unsigned short UINT16;
typedef unsigned char UINT8;
#endif
#define ANALYSIS
#ifdef ANALYSIS

#define LOGD(...)
// #define LOG(m,k,v) printf("%f,%s,%s,%d\n",(op_sim_time()),(m),(k),(v))
#define LOG(fmt, ...) printf("@%3.9f" fmt "\n", op_sim_time(), ##__VA_ARGS__)
#else

#define LOG(...)
#define LOGD(fmt, ...) printf("@%3.9f" fmt "\n", op_sim_time(), ##__VA_ARGS__)

#endif
#endif