#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <windows.h>
#include <stdbool.h>
#include "useful.h"
const double PI=3.141592653/180;
typedef  byte colorbyte;
typedef struct {
    colorbyte *buf;
    DWORD size;
    int nWidth,height,mwidth,mheight;
    BITMAPINFO binfo;
} BMP;
typedef BMP *LPBMP;
typedef struct {
    int now,walknow,money;
    int x,y,nowx,nowy,show;
    bool toward,drop,fire,walktoward,reload,zhuang;
    double guna,v0,v1,t,jiao,showangle,up,canup,hp;
    COLORREF showcolor;
    byte tans;
    LARGE_INTEGER nowtime,lasttime;
} people;
typedef struct {
    int now,x,y;
} girlnode;
typedef struct {
    int x,y,father,act,ed;
    double time,t;
} listnode;
typedef struct {
    int now,walknow;
    int x,y,nowx,nowy,lei,hp,showtime;
    bool drop,walktoward,zhuang;
    double vx,vy,t,jiao,acttime,sumacttime;
    bool flag;
    int trans;
    listnode next;
    LARGE_INTEGER nowtime,lasttime;
} zombie;
typedef struct {
    int sx,sy,x,y,damage;
    double jiao,length;
    bool flag;
} bullnode;
typedef struct {
    int nowx,nowy,x,y,damage;
    double vx,vy,t,jiao;
    bool flag,boom;
} granode;
typedef struct {
    int nowx,nowy,x,y;
    double vx,vy,t,lefttime;
    bool flag,drop;
    LARGE_INTEGER nowtime,lasttime;
} bloodnode;
typedef struct {
    int nowx,nowy,x,y,trans;
    double vx,vy,t,lefttime;
    bool flag;
    LARGE_INTEGER nowtime,lasttime;
} smognode;
typedef struct {
    tbmp image,simage;
    bool running;
    int lei,left,now,danjia,reltime,reloadtime,damage,speed,play[5];
    int routex,routey,price;
    double up,diameter;
    char name[128],imagepath[256],simagepath[256],sound[5][256],shoot[256];
} weanode;
typedef struct {
    int x,y;
    double now;
    int boomflag;
} boomnode;
//主角
const int  TheGirl=1;
const int  TheMan=0;
const int  Nothing=-1;
const int  Walk=0;
const int  Jump=1;
const int  Attack=2;
  //主角女友
const int  LieOnTheBox=1;
const int  LieOnGround=2;
  //武器代码
const int  Gun=101;
const int  Gre=102;
const int  SameBlt=0,HengBlt=1,ShuBlt=2,HengShuBlt=3,Left90=4,Right90=5;
const int  GI_OUTOFRANGE=0;
const int  GI_DOOR1=1;
const int  GI_DOOR2=2;
const int  GI_DOOR3=3;
const int  GI_LEFT=4;
const int  GI_RIGHT=5;
const int  GI_TOP=6;
const int  GI_BOTTOM=7;
const int  GI_NOTHING=8;

const int  ACT_NOTHING=0;
const int  ACT_LEFT=1;
const int  ACT_RIGHT=2;
const int  ACT_STAIR=3;
const int  ACT_DROP=4;
const int  zhen[5][2]={{0,0},{-1,-1},{1,-1},{1,1},{-1,1}};

const int MenuKeepY=240-125+22+15+10;
const int MenuBackY=240-125+22+15+60;
const int MenuSaveY=240-125+22+15+110;
const int MenuExitY=240-125+22+15+160;
const int BoxCancelX=320-171/2+3;
const int BoxCancelY=240-65/2+42;
const int BoxOKX=320-171/2+3+82;
const int BoxOKY=240-65/2+42;
const int ShopWeaWindowTop=30;
const int ShopWeaWindowLeft=150;
const int ShopWeaWindowBottom=300;
const int ShopWeaWindowRight=600;

double degsin(double x);
double degcos(double x);
POINT getlinepixel(int x,int y,int length,double a);
double getdis(double x1,double y1,double x2,double y2);
double getjiao(int nowx,int nowy,int x,int y);
void SetPixelAtBmp(LPBMP save,int x,int y,char red,char green,char blue);
COLORREF GetPixelAtBmp(LPBMP save,int x,int y);
void DrawCircle(HDC dc,int x1,int y1,int r,COLORREF color,COLORREF color2);
void DrawLine(int x1,int y1,int x2,int y2,COLORREF color,UINT flags,int cusi);
void AlphaBlt(HDC dc2,int x2,int y2,int width2,int height2,HDC dc1,int x1,int y1,int width1,int height1,char trans);
HWND WinCreate(HINSTANCE hInstance);
void openbmp(tbmp image,LPBMP save);
void closebmp(tbmp iamge,LPBMP save);
void print();
int GetIt(int x,int y);
int GetItAtZombie(int x,int y,int which,int walknow);
#endif
