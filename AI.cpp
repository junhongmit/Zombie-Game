#include "AI.h"
#include "window.h"
#include "useful.h"
#include <stdbool.h>
#include <stdio.h>
extern HDC windc,bitmapdc;
extern BMP formmark;
extern int bullsum,grasum,killzombie,money;
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
extern tbmp man,bul,lie;
extern people mann;
extern girlnode girl;
extern int head,tail,nowx,nowy,i,minwho;
extern bool bo[5];
extern bool bfloor,found;
extern double s,t,mintime;
extern COLORREF lastcolor;
extern listnode list[1000];
extern bool boo[1080][5];
extern people tman;
extern bool pause;
int GetFloor(int y)
{
    if (y>80&&y<=160) return 4;
    if (y>160&&y<=240) return 3;
    if (y>240&&y<=320) return 2;
    if (y>320&&y<=400) return 1;
    return 1;
}
int GetFloorPixel(int y)
{
    if (y==4) return 160;
    else if (y==3) return 240;
    else if (y==2) return 320;
    else if (y==1) return 400;
    return 400;
}
bool CreateZombie()
{
    int x;
    zombie empty={};
    for (x=1; x<=200; x++)
    if (!zom[x].flag) {
        zom[x]=empty;
        zom[x].lei=rand()%18+1;
        zom[x].hp=100; zom[x].walknow=1; zom[x].trans=255;
        if (rand()%2==1) {
            zom[x].x=1072-18; zom[x].y=480-48-32;
        } else {
            zom[x].x=0; zom[x].y=480-48-32;
        }
        zom[x].showtime=500;
        zom[x].flag=true;
        return true;
    }
    return false;
}
void DamageZombie(zombie *zom,int damage)
{
    if ((zom->hp-damage)>0) zom->hp-=damage;
    else {
        if (zom->hp!=0) {
            zom->hp=0;
            killzombie++;
            mann.money+=50;
        }
    }
}
listnode returnvalue(int who)
{
    int x,last;
    x=who; last=who;
    while (list[x].father!=0) {
        //printf("%d ",list[x].act);
        last=x; x=list[x].father;
    }
    //printf("\n");
    return list[last];
}
bool check(int x,int y,bool updown)
{
    int i;
    if (x<0||x>1072-17) return false;
    if (updown) {
        if (GetPixelAtBmp(&formmark,x,y)==RGB(255,0,0)) return false;
        if (GetPixelAtBmp(&formmark,x,y)==RGB(0,255,0)) return false;
        if (GetPixelAtBmp(&formmark,x,y)==RGB(0,0,255)) return false;
        return true;
    } else {
        if (x<0||x>1072||y<0||y>480) return false;
        if ((x+17>=tman.x)&&(x<=tman.x+17)&&(GetFloor(tman.y)==GetFloor(y))) return false;
        if (x>=girl.x&&x<=girl.x+38&&GetFloor(girl.y-13)==GetFloor(y)) return false;
        bfloor=false;
        for (i=5; i<=12; i++) {
            if (GetPixelAtBmp(&formmark,x+i,y+34)==RGB(255,255,0)) {
                bfloor=true; break;
            }
        }
        return bfloor;
    }
}
int GetTarget(int x,int y)
{
    if (x>=girl.x&&x<=girl.x+38&&GetFloor(girl.y-13)==GetFloor(y)) return TheGirl;
    if (x+17>=tman.x&&x<=tman.x+17&&y<=mann.y+30&&GetFloor(tman.y)==GetFloor(y)) return TheMan;
    return Nothing;
}
bool target(int x,int y)
{
    //printf("%d %d %d\n",x,girl.x,girl.x+38);
    if (x>=girl.x&&x<=girl.x+38&&GetFloor(girl.y-13)==GetFloor(y)) return true;
    if (x+17>=tman.x&&x<=tman.x+17&&GetFloor(tman.y)==GetFloor(y)) return true;
    return false;
}
//#define _SHOW_WAY_
listnode BFS(int x,int y)
{
    double nowtime;
    double t;
    tman=mann; tman.y=GetFloorPixel(GetFloor(tman.y));
    int tempx1=tman.x;
    while (1) {
        bfloor=false;
        for (i=5; i<=12; i++) {
            if (GetPixelAtBmp(&formmark,tempx1+i,tman.y+34)==RGB(255,255,0)) {
                bfloor=true; break;
            }
        }
        if (bfloor) break;
        tempx1--; if (tempx1<0) break;
    }
    int tempx2=tman.x;
    while (1) {
        bfloor=false;
        for (i=5; i<=12; i++) {
            if (GetPixelAtBmp(&formmark,tempx2+i,tman.y+34)==RGB(255,255,0)) {
                bfloor=true; break;
            }
        }
        if (bfloor) break;
        tempx2++; if (tempx2>1072) break;
    }
    if (tempx1<0) tman.x=tempx2;
    else if (tempx2>1072) tman.x=tempx1;
    else if (abs(tempx1-tman.x)<abs(tempx2-tman.x)) tman.x=tempx1; else tman.x=tempx2;
    memset(list,0,sizeof(list)); head=1; tail=1;
    memset(bo,true,sizeof(bo));
    memset(boo,true,sizeof(boo));
    list[1].x=x; list[1].y=y; bo[GetFloor(y)]=false;
    if (target(x,y)) return list[1];
    minwho=1; mintime=100000000;
    #ifdef _SHOW_WAY_
        print();
    #endif // _SHOW_WAY_
    while (head<=tail) {
        nowx=list[head].x; nowy=list[head].y; nowtime=list[head].time;
        boo[nowx][GetFloor(nowy)]=false;
        if (list[head].time>=mintime) {head++; continue;}
        if (GetPixelAtBmp(&formmark,nowx,nowy)==RGB(255,0,0)) {
            found=false;
            if (GetFloor(nowy)==1&&bo[2]) {nowy=GetFloorPixel(2); bo[2]=false; found=true;}
            if (GetFloor(nowy)==2&&bo[1]) {nowy=GetFloorPixel(1); bo[1]=false; found=true;}
            if (found) {
                #ifdef _SHOW_WAY_
                    DrawLine(list[head].x-minx,list[head].y,nowx-minx,nowy,RGB(255,255,0),PS_SOLID,1);
                #endif // _SHOW_WAY_
                tail++; list[tail].x=nowx; list[tail].y=nowy;
                list[tail].act=ACT_STAIR; list[tail].father=head;
                list[tail].time=list[head].time+0.2;
                if (target(list[tail].x,nowy)) {
                    if (list[tail].time<mintime) {
                        mintime=list[tail].time; minwho=tail;
                    }
                }
            }
        }
        if (GetPixelAtBmp(&formmark,nowx,nowy)==RGB(0,255,0)) {
            found=false;
            if (GetFloor(nowy)==2&&bo[3]) {nowy=GetFloorPixel(3); bo[3]=false; found=true;}
            if (GetFloor(nowy)==3&&bo[2]) {nowy=GetFloorPixel(2); bo[2]=false; found=true;}
            if (found) {
                #ifdef _SHOW_WAY_
                    DrawLine(list[head].x-minx,list[head].y,nowx-minx,nowy,RGB(255,255,0),PS_SOLID,1);
                #endif // _SHOW_WAY_
                tail++; list[tail].x=nowx; list[tail].y=nowy;
                list[tail].act=ACT_STAIR; list[tail].father=head;
                list[tail].time=list[head].time+0.2;
                if (target(list[tail].x,nowy)) {
                    if (list[tail].time<mintime) {
                        mintime=list[tail].time; minwho=tail;
                    }
                }
            }
        }
        if (GetPixelAtBmp(&formmark,nowx,nowy)==RGB(0,0,255)) {
            found=false;
            if (GetFloor(nowy)==3&&bo[4]) {nowy=GetFloorPixel(4); bo[4]=false; found=true;}
            if (GetFloor(nowy)==4&&bo[3]) {nowy=GetFloorPixel(3); bo[3]=false; found=true;}
            if (found) {
                #ifdef _SHOW_WAY_
                    DrawLine(list[head].x-minx,list[head].y,nowx-minx,nowy,RGB(255,255,0),PS_SOLID,1);
                #endif // _SHOW_WAY_
                tail++; list[tail].x=nowx; list[tail].y=nowy;
                list[tail].act=ACT_STAIR; list[tail].father=head;
                list[tail].time=list[head].time+0.2;
                if (target(list[tail].x,nowy)) {
                    if (list[tail].time<mintime) {
                        mintime=list[tail].time; minwho=tail;
                    }
                }
            }
        }
        nowy=list[head].y;
        bfloor=false;
        for (i=5; i<=12; i++) {
            if (GetPixelAtBmp(&formmark,nowx+i,nowy+34)==RGB(255,255,0)) {
                bfloor=true; break;
            }
        }
        if (!bfloor) {
            if (GetFloor(nowy)<=GetFloor(tman.y)) {head++; continue;}
            s=abs(nowy-GetFloorPixel(GetFloor(tman.y)));
            t=sqrt(fabs(s/100.0*5)/4.9);
            nowy=GetFloorPixel(GetFloor(tman.y));
            #ifdef _SHOW_WAY_
                DrawLine(list[head].x-minx,list[head].y,nowx-minx,nowy,RGB(255,255,0),PS_SOLID,1);
            #endif // _SHOW_WAY_
            int tempx=nowx;
            do {
                bfloor=false; tempx--; if (tempx<0) break;
                for (i=5; i<=12; i++) {
                    if (GetPixelAtBmp(&formmark,tempx+i,nowy+34)==RGB(255,255,0)) {
                        bfloor=true; break;
                    }
                }
            } while (!bfloor);
            if (tempx>=0) {
                tail++; list[tail].x=tempx; list[tail].y=nowy;
                list[tail].t=t; list[tail].act=ACT_DROP; list[tail].father=head;
                list[tail].time=list[head].time+t;
            }
            tempx=nowx;
            do {
                bfloor=false; tempx++; if (tempx>1072) break;
                for (i=5; i<=12; i++) {
                    if (GetPixelAtBmp(&formmark,tempx+i,nowy+34)==RGB(255,255,0)) {
                        bfloor=true; break;
                    }
                }
            } while (!bfloor);
            if (tempx<=1072) {
                tail++; list[tail].x=tempx; list[tail].y=nowy;
                list[tail].t=t; list[tail].act=ACT_DROP; list[tail].father=head;
                list[tail].time=list[head].time+t;
            }
        } else {
            lastcolor=0; nowx=list[head].x;
            while (1) {
                while (check(nowx,nowy,true)) nowx-=10;
                if (nowx<0) break;
                if (GetPixelAtBmp(&formmark,nowx,nowy)==lastcolor) {
                    nowx--; continue;
                }
                lastcolor=GetPixelAtBmp(&formmark,nowx,nowy);
                if (nowx>=0&&boo[nowx][GetFloor(nowy)]) {
                    #ifdef _SHOW_WAY_
                        DrawLine(list[head].x-minx,list[head].y,nowx-minx,nowy,RGB(255,255,0),PS_SOLID,1);
                    #endif // _SHOW_WAY_
                    tail++; list[tail].x=nowx; list[tail].y=nowy; list[tail].act=ACT_LEFT;
                    list[tail].time=list[head].time+abs(nowx-list[head].x)*0.1; list[tail].father=head;
                    if (target(list[tail].x,nowy)) {
                        if (list[tail].time<mintime) {
                            mintime=list[tail].time; minwho=tail;
                        }
                    }
                } else if (nowx<0) break;
            }
            nowx=list[head].x;
            while (check(nowx,nowy,false)) nowx-=10;
            if (nowx>=0&&boo[nowx][GetFloor(nowy)]) {
                #ifdef _SHOW_WAY_
                    DrawLine(list[head].x-minx,list[head].y,nowx-minx,nowy,RGB(255,255,0),PS_SOLID,1);
                #endif // _SHOW_WAY_
                tail++; list[tail].x=nowx; list[tail].y=nowy; list[tail].act=ACT_LEFT;
                list[tail].time=list[head].time+abs(nowx-list[head].x)*0.1; list[tail].father=head;
                if (target(list[tail].x,nowy)) {
                    if (list[tail].time<mintime) {
                        mintime=list[tail].time; minwho=tail;
                    }
                }
            }
            lastcolor=0; nowx=list[head].x;
            while (1) {
                while (check(nowx,nowy,true)) nowx+=10;
                if (nowx+17>=1072) break;
                if (GetPixelAtBmp(&formmark,nowx,nowy)==lastcolor) {nowx++; continue; }
                lastcolor=GetPixelAtBmp(&formmark,nowx,nowy);
                if (nowx+17<=1072&&boo[nowx][GetFloor(nowy)]) {
                    #ifdef _SHOW_WAY_
                        DrawLine(list[head].x-minx,list[head].y,nowx-minx,nowy,RGB(255,255,0),PS_SOLID,1);
                    #endif // _SHOW_WAY_
                    tail++; list[tail].x=nowx+1; list[tail].y=nowy; list[tail].act=ACT_RIGHT;
                    list[tail].time=list[head].time+abs(nowx-list[head].x)*0.1; list[tail].father=head;
                    if (target(list[tail].x,nowy)) {
                        if (list[tail].time<mintime) {
                            mintime=list[tail].time; minwho=tail;
                        }
                    }
                } else if (nowx+17>1072) break;
            }
            nowx=list[head].x;
            while (check(nowx,nowy,false)) nowx+=10;
            if ((nowx+17<=1072)&&boo[nowx][GetFloor(nowy)]) {
                #ifdef _SHOW_WAY_
                    DrawLine(list[head].x-minx,list[head].y,nowx-minx,nowy,RGB(255,255,0),PS_SOLID,1);
                #endif // _SHOW_WAY_
                tail++; list[tail].x=nowx+1; list[tail].y=nowy;list[tail].act=ACT_RIGHT;
                list[tail].time=nowtime+abs(nowx-list[head].x)*0.1; list[tail].father=head;
                if (target(list[tail].x,nowy)) {
                    if (list[tail].time<mintime) {
                        mintime=list[tail].time; minwho=tail;
                    }
                }
            }
        }
        head++;
        //if (head>=30) break;
    }
    #ifdef _SHOW_WAY_
        BitBlt(windc,zhen[zhennow][0]*zhenhan,zhen[zhennow][1]*zhenhan,640,480,bitmapdc,0,0,SRCCOPY);
    #endif // _SHOW_WAY_
    return returnvalue(minwho);
}
