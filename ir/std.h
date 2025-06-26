//
// Created by zenglj on 2021/8/23.
//

#ifndef MINIC_STD_H
#define MINIC_STD_H

#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>

/* Input & output functions */
int getint();
int getch();
int getarray(int a[]);

void putint(int a);
void putch(int a);
void putarray(int n, int a[]);

float getfloat();
int getfarray(float a[]);
void putfloat(float a);
void putfarray(int n, float a[]);
void putf(char a[], ...);

#if !defined(WIN32) && defined(OPT_TEST)
/* Timing function implementation */
struct timeval _sysy_start, _sysy_end;
#define starttime() _sysy_starttime(__LINE__)
#define stoptime() _sysy_stoptime(__LINE__)
#define _SYSY_N 1024
int _sysy_l1[_SYSY_N], _sysy_l2[_SYSY_N];
int _sysy_h[_SYSY_N], _sysy_m[_SYSY_N], _sysy_s[_SYSY_N], _sysy_us[_SYSY_N];
int _sysy_idx;
__attribute((constructor)) void before_main();
__attribute((destructor)) void after_main();
void _sysy_starttime(int lineno);
void _sysy_stoptime(int lineno);
#endif

#endif // MINIC_STD_H
