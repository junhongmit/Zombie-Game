#include "sound.h"
#include "useful.h"
//extern int playnow;
bool play[6];
char path[6][256],oldpath[6][256];
extern HANDLE Handle1,Handle2,Handle3,Handle7,Handle8;
DWORD ThreadID1,ThreadID2,ThreadID3,ThreadID7,ThreadID8;
DWORD WINAPI Thread1_sound (LPVOID lpParam)
{
    char temp[256];
    while (1) {
        if (play[1]) {
            if (!strcmp(oldpath[1],path[1])) {
                mciSendString("seek mysong to start",NULL,0,0);
            } else {
                mciSendString("close mysong",NULL,0,0);
                sprintf(temp,"open \"%s\" alias mysong",path[1]);
                mciSendString(temp,NULL,0,0);
            }
            mciSendString("play mysong",NULL,0,0);
            play[1]=false;
        }
        Sleep(50);
    }
}
DWORD WINAPI Thread2_sound(LPVOID pParam)
{
    char temp[256];
    while (1) {
        if (play[2]) {
            if (!strcmp(oldpath[2],path[2])) {
                mciSendString("seek mysong to start",NULL,0,0);
            } else {
                mciSendString("close mysong",NULL,0,0);
                sprintf(temp,"open \"%s\" alias mysong",path[2]);
                mciSendString(temp,NULL,0,0);
            }
            mciSendString("play mysong",NULL,0,0);
            play[2]=false;
        }
        Sleep(50);
    }
}
DWORD WINAPI Thread3_sound(LPVOID pParam)
{
    char temp[256];
    while (1) {
        if (play[3]) {
            if (!strcmp(oldpath[3],path[3])) {
                mciSendString("seek mysong to start",NULL,0,0);
            } else {
                mciSendString("close mysong",NULL,0,0);
                sprintf(temp,"open \"%s\" alias mysong",path[3]);
                mciSendString(temp,NULL,0,0);
            }
            mciSendString("play mysong",NULL,0,0);
            play[3]=false;
        }
        Sleep(50);
    }
}
DWORD WINAPI Thread4_sound(LPVOID pParam)
{
    char temp[256];
    while (1) {
        if (play[4]) {
            if (!strcmp(oldpath[4],path[4])) {
                mciSendString("seek mysong to start",NULL,0,0);
            } else {
                mciSendString("close mysong",NULL,0,0);
                sprintf(temp,"open \"%s\" alias mysong",path[4]);
                mciSendString(temp,NULL,0,0);
            }
            mciSendString("play mysong",NULL,0,0);
            play[4]=false;
        }
        Sleep(50);
    }
}
DWORD WINAPI Thread5_sound(LPVOID pParam)
{
    char temp[256];
    while (1) {
        if (play[5]) {
            if (!strcmp(oldpath[5],path[5])) {
                mciSendString("seek mysong to start",NULL,0,0);
            } else {
                mciSendString("close mysong",NULL,0,0);
                sprintf(temp,"open \"%s\" alias mysong",path[5]);
                mciSendString(temp,NULL,0,0);
            }
            mciSendString("play mysong",NULL,0,0);
            play[5]=false;
        }
        Sleep(50);
    }
}
int playnow;
void PlaySound(char *st)
{
    strcpy(oldpath[playnow],path[playnow]);
    strcpy(path[playnow],st);
    play[playnow]=true;
    playnow++; if (playnow>5) playnow=1;
}
void Sound_Init()
{
    playnow=1;
    Handle1=CreateThread(NULL,0,Thread1_sound,NULL,0,&ThreadID1);
    Handle2=CreateThread(NULL,0,Thread2_sound,NULL,0,&ThreadID2);
    Handle3=CreateThread(NULL,0,Thread3_sound,NULL,0,&ThreadID3);
    Handle7=CreateThread(NULL,0,Thread4_sound,NULL,0,&ThreadID7);
    Handle8=CreateThread(NULL,0,Thread5_sound,NULL,0,&ThreadID8);
}
