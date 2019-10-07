#ifndef GIMMESH_CORE_H
#define GIMMESH_CORE_H

#include "common.h"

extern int coreInit(const s8* meshFilePath);
extern void coreDestroy();
extern void coreUpdate(r32 deltaTime);
extern void coreRender();
extern void coreInputProcess(boolean* keyState, r32 deltaTime);
extern void coreMouseChangeProcess(boolean reset, r64 xPos, r64 yPos);
extern void coreMouseClickProcess(s32 button, s32 action, r64 xPos, r64 yPos);
extern void coreScrollChangeProcess(r64 xOffset, r64 yOffset);
extern void coreWindowResizeProcess(s32 width, s32 height);

#endif