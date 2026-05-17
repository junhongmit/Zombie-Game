#include "AI.h"
#include "window.h"
#include "useful.h"
#include <stdbool.h>
#include <stdio.h>
extern HDC windc,bitmapdc;
extern BMP formmark;
extern int bullsum,grasum;
extern int minx,minaim,mousex,mousey,maxx,maxy,weanow,boomsum,zhennow,zhenhan;
extern tbmp sky[6];
extern tbmp backcity[4];
extern tbmp building[4];
extern tbmp bopic[17];
extern boomnode boom[100];
extern weanode weapon[30];
extern tbmp form[4];
extern tbmp zombies[19];
extern bullnode bull[211];
extern granode gr[211];
extern zombie zom[211];
extern bloodnode blood[5000];
extern smognode smog[100];
extern tbmp man,bul,lie;
extern people mann;
extern girlnode girl;
extern LARGE_INTEGER frequent;
extern bool pause,quit;
int smognow;
void PutSmog(int x,int y,double angle,double v0,int num)
{
    double rangle;
    for (; num>=0; num--) {
        while (smog[smognow].flag) smognow=smognow>=99?0:smognow+1;
        smog[smognow].x=smog[smognow].nowx=x;
        smog[smognow].y=smog[smognow].nowy=y;
        smog[smognow].lefttime=1.5;
        rangle=angle+rand()%31-15;
        smog[smognow].vx=v0*cos(rangle*PI);
        smog[smognow].vy=v0*sin(rangle*PI);
        smog[smognow].t=0;
        QueryPerformanceCounter(&smog[smognow].lasttime);
        smog[smognow].flag=true;
    }
}
DWORD ThreadID6;
HANDLE Handle6;
DWORD WINAPI Thread6_smog(LPVOID pParam)
{
    int i,nowx,nowy;
    double deltat;
    while (1) {
        if (!pause&&!quit)
        for (i=0; i<100; i++)
            if (smog[i].flag) {
                QueryPerformanceCounter(&smog[i].nowtime);
                deltat=(double)(smog[i].nowtime.QuadPart-smog[i].lasttime.QuadPart)/frequent.QuadPart;
                smog[i].t=smog[i].t+deltat;
                smog[i].lasttime=smog[i].nowtime;
                nowy=smog[i].y-round((smog[i].vy*deltat)*100/5);
                nowx=smog[i].x+round(smog[i].vx*deltat*100/5);
                if (fabs(smog[i].vx)>0.2) {
                    if (smog[i].vx>0) smog[i].vx-=2*deltat;
                    else smog[i].vx+=2*deltat;
                } else {
                    nowx=smog[i].x;
                    smog[i].vx=0;
                }
                if (fabs(smog[i].vy)>0.2) {
                    if (smog[i].vy>0) smog[i].vy-=2*deltat;
                    else smog[i].vy+=2*deltat;
                } else {
                    nowy=smog[i].y;
                    smog[i].vy=0;
                }
                if (nowx<0||nowx>1072) {smog[i].flag=false; continue;}
                if (GetPixelAtBmp(&formmark,nowx,nowy)==RGB(255,255,0)) {
                    smog[i].vy=0;
                    smog[i].x=nowx; smog[i].nowy=smog[i].y; smog[i].nowx=smog[i].x;
                } else {
                    smog[i].y=nowy;
                    smog[i].x=nowx;
                }
                smog[i].lefttime-=deltat;
                if (smog[i].lefttime<0) smog[i].flag=false;
            }
        Sleep(50);
    }
}
void Smog_Init()
{
    smognow=0;
    memset(smog,0,sizeof(smog));
    Handle6=CreateThread(NULL,0,Thread6_smog,NULL,0,&ThreadID6);
}
