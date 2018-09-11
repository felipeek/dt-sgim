#ifndef GIMMESH_MENU_H
#define GIMMESH_MENU_H
#include <GLFW/glfw3.h>
#include "common.h"

typedef void (*RecursiveFilterCallback)(r32, s32);
typedef void (*DistanceFilterCallback)(r32, r32, s32);
typedef void (*CurvatureFilterCallback)(r32, r32, s32, s32, r32, r32, s32, r32, r32);
typedef void (*TextureChangeCallback)(s32);
typedef void (*NoiseGeneratorCallback)(r32);

void menuRegisterNoiseGeneratorCallBack(NoiseGeneratorCallback f);
void menuRegisterRecursiveFilterCallBack(RecursiveFilterCallback f);
void menuRegisterDistanceFilterCallBack(DistanceFilterCallback f);
void menuRegisterCurvatureFilterCallBack(CurvatureFilterCallback f);
void menuRegisterTextureChangeCallBack(TextureChangeCallback f);
void menuCharClickProcess(GLFWwindow* window, u32 c);
void menuKeyClickProcess(GLFWwindow* window, s32 key, s32 scanCode, s32 action, s32 mods);
void menuMouseClickProcess(GLFWwindow* window, s32 button, s32 action, s32 mods);
void menuScrollChangeProcess(GLFWwindow* window, s64 xoffset, s64 yoffset);
void menuInit(GLFWwindow* window);
void menuRender();
void menuDestroy();

#endif