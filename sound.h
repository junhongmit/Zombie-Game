#ifndef _SOUND_H_
#define _SOUND_H_

#include <windows.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
DWORD WINAPI Thread1_sound(LPVOID pParam);
DWORD WINAPI Thread2_sound(LPVOID pParam);
DWORD WINAPI Thread3_sound(LPVOID pParam);
DWORD WINAPI Thread4_sound(LPVOID pParam);
DWORD WINAPI Thread5_sound(LPVOID pParam);
void PlaySound(char *st);
void Sound_Init();
#endif
