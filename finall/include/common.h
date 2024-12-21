#ifndef COMMON_H
#define COMMON_H

#include <GLUT/glut.h> // 修正后的GLUT头文件路径
#include <cmath>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <vector>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
// macOS-specific includes
#include <unistd.h>
#include <GLUT/glut.h> // 修正后的GLUT头文件路径
#include <signal.h>
#endif

// 全局变量声明
extern float lamp_x, lamp_y;
extern bool lightOn;

#endif // COMMON_H