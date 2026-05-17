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
extern tbmp man,bul,lie;
extern people mann;
extern girlnode girl;
extern LARGE_INTEGER frequent;
int bloodnow;
extern bool pause,quit;
void PutBlood(int x,int y,int ward)
{
    const double v0=5;
    int num,angle;
    for (num=rand()%5+3; num>=0; num--) {
        while (blood[bloodnow].flag) bloodnow=bloodnow>=4999?0:bloodnow+1;
        blood[bloodnow].drop=true;
        blood[bloodnow].x=blood[bloodnow].nowx=x;
        blood[bloodnow].y=blood[bloodnow].nowy=y;
        blood[bloodnow].lefttime=60;
        angle=rand()%181;
        switch (ward) {
            case GI_BOTTOM:
                blood[bloodnow].vx=v0*cos(angle*PI);
                blood[bloodnow].vy=-v0*sin(angle*PI);
                break;
            case GI_LEFT:
                blood[bloodnow].vx=-v0*sin(angle*PI);
                blood[bloodnow].vy=v0*cos(angle*PI);
                break;
            case GI_RIGHT:
                blood[bloodnow].vx=v0*sin(angle*PI);
                blood[bloodnow].vy=v0*cos(angle*PI);
                break;
            case GI_TOP:
                blood[bloodnow].vx=v0*cos(angle*PI);
                blood[bloodnow].vy=v0*sin(angle*PI);
                break;
        }
        QueryPerformanceCounter(&blood[bloodnow].lasttime);
        blood[bloodnow].flag=true;
    }
}
int GetFloorBlood(int y)
{
    if (y<=191) return 4;
    if (y>191&&y<=271) return 3;
    if (y>271&&y<=351) return 2;
    if (y>351&&y<=431) return 1;
    return 1;
}
int GetFloorPixelBlood(int y)
{
    if (y==4) return 191;
    else if (y==3) return 271;
    else if (y==2) return 351;
    else if (y==1) return 431;
    return 431;
}
DWORD ThreadID5;
HANDLE Handle5;
DWORD WINAPI Thread5_blood(LPVOID pParam)
{
    int i,nowx,nowy;
    double deltat;
    while (1) {
        if (!pause&&!quit)
        for (i=0; i<5000; i++)
            if (blood[i].flag) {
                QueryPerformanceCounter(&blood[i].nowtime);
                deltat=(double)(blood[i].nowtime.QuadPart-blood[i].lasttime.QuadPart)/frequent.QuadPart;
                blood[i].t=blood[i].t+deltat;
                blood[i].lasttime=blood[i].nowtime;
                if (blood[i].drop) {
                    nowy=blood[i].nowy-round((blood[i].vy*blood[i].t-4.9*blood[i].t*blood[i].t)*100/5);
                    nowx=blood[i].nowx+round(blood[i].vx*blood[i].t*100/5);
                    if (nowx<0||nowx>1072) {blood[i].flag=false; continue;}
                    if (GetPixelAtBmp(&formmark,nowx,nowy)==RGB(255,255,0)) {
                        blood[i].drop=false; blood[i].vy=0; blood[i].t=0;
                        blood[i].y=GetFloorPixelBlood(GetFloorBlood(blood[i].y));
                        blood[i].x=nowx; blood[i].nowy=blood[i].y; blood[i].nowx=blood[i].x;
                    } else {
                        blood[i].y=nowy;
                        blood[i].x=nowx;
                    }
                } else {
                    nowx=blood[i].nowx+round(blood[i].vx*blood[i].t*100/5);
                    if (fabs(blood[i].vx)>0.2) {
                        if (blood[i].vx>0) blood[i].vx-=2*deltat;
                        else blood[i].vx+=2*deltat;
                    } else {
                        nowx=blood[i].nowx;
                        blood[i].vx=0;
                    }
                    if (nowx<0||nowx>1072) {blood[i].flag=false; continue;}
                    if (GetPixelAtBmp(&formmark,nowx,blood[i].y+1)==RGB(255,255,255)) {
                        blood[i].drop=true; blood[i].vy=0; blood[i].t=0;
                        blood[i].y=GetFloorPixelBlood(GetFloorBlood(blood[i].y));
                        blood[i].nowy=blood[i].y; blood[i].nowx=blood[i].x;
                    }
                    blood[i].x=nowx; blood[i].nowx=blood[i].x; blood[i].t=0;
                }
                blood[i].lefttime-=deltat;
                if (blood[i].lefttime<0) blood[i].flag=false;
            }
        Sleep(50);
    }
}
void Blood_Init()
{
    bloodnow=0;
    memset(blood,0,sizeof(blood));
    Handle5=CreateThread(NULL,0,Thread5_blood,NULL,0,&ThreadID5);
}
