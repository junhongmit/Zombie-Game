#include <windows.h>
#include "window.h"
#include "useful.h"
#include <stdio.h>
#include <math.h>
#pragma comment(lib,msimg32.lib)
extern HWND hWindow;
extern HDC windc,bitmapdc,tempdc,tempdc2,tempdc3,red,black,save;
extern HBITMAP savescreen,savedscreen;
extern HFONT thefont;
extern BMP formmark;
extern BMP formzombie[19];
extern int bullsum,grasum,weasum,wave;
extern int minx,minaim,mousex,mousey,maxx,maxy,weanow,boomsum,zhennow,zhenhan,turnred,turnblack;
extern tbmp sky[6];
extern tbmp backcity[4];
extern tbmp building[4];
extern tbmp bopic[17];
extern tbmp smogpic;
extern tbmp menu,box;
extern boomnode boom[100];
extern weanode weapon[30],hweapon[30];
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
extern bool pause,quit,shop,Box;
extern tbmp weatest;

//shop
extern int choosenow;
extern ImaButton bprevious,bnext,bbuy,bcontinue,bexit;

inline double dectored(double x)
{
    return x*PI;
}
double degsin(double x)
{
    return sin(dectored(x));
}
double degcos(double x)
{
    return cos(dectored(x));
}
POINT getlinepixel(int x,int y,int length,double a)
{
    POINT dn;
    dn.x=x+round(length*degcos(a));
    dn.y=y-round(length*degsin(a));
    return dn;
}
double getdis(double x1,double y1,double x2,double y2)
{
    return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}
double getjiao(int nowx,int nowy,int x,int y)
{
    double i=0,l,r;
    int length;
    if (y==nowy) {
        if (x>nowx) return 0; else return 180;
    } else if (x==nowx) {
        if (y>nowy) return 270; else return 90;
    } else {
        i=atan2((y-nowy),(nowx-x));
        return (i/PI+180);
    }
}
void openbmp(tbmp image,LPBMP save)
{
    int now,noffset,i,j;
    save->size=image.width*image.height*24;
    save->height=image.height;
    save->buf=(colorbyte*)malloc(save->size);
    save->binfo.bmiHeader.biBitCount=24;
    save->binfo.bmiHeader.biCompression=0;
    save->binfo.bmiHeader.biHeight=image.height;
    save->binfo.bmiHeader.biPlanes=1;
    save->binfo.bmiHeader.biSizeImage=0;
    save->binfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    save->binfo.bmiHeader.biWidth=image.width;
    GetDIBits(image.color,image.colorbmp,0,save->binfo.bmiHeader.biHeight,save->buf,&save->binfo,DIB_RGB_COLORS);
    save->nWidth= image.width*3;
    save->nWidth= ((save->nWidth+3)/4)*4;
    save->mwidth=image.width; save->mheight=image.height;
}
void closebmp(tbmp image,LPBMP save)
{
    SetDIBits(image.color,image.colorbmp,0,save->binfo.bmiHeader.biHeight,save->buf,&save->binfo,DIB_RGB_COLORS);
    free(save->buf);
}
void SetPixelAtBmp(LPBMP save,int x,int y,char red,char green,char blue)
{
    int now,noffset;
    if (x>=0&&y>=0&&x<save->mwidth&&y<save->mheight) {
        noffset=(save->height-y-1)*save->nWidth;
        now=noffset+x*3;
        save->buf[now+2]=red;
        save->buf[now+1]=green;
        save->buf[now]=blue;
    }
}
COLORREF GetPixelAtBmp(LPBMP save,int x,int y)
{
    int now,noffset;
    if (x>=0&&y>=0&&x<save->mwidth&&y<save->mheight) {
        noffset=(save->height-y-1)*save->nWidth;
        now=noffset+x*3;
        return RGB(save->buf[now+2],save->buf[now+1],save->buf[now]);
    } else return 0;
}
void DrawCircle(HDC dc,int x1,int y1,int r,COLORREF color,COLORREF color2)
{
    int y;
    HPEN pen,oldpen;
    HBRUSH brush,oldbrush;
    pen=CreatePen(PS_SOLID,1,color);
    oldpen=(HPEN)SelectObject(dc,pen);
    brush=CreateSolidBrush(color2);
    oldbrush=(HBRUSH)SelectObject(dc,brush);
    if (r%2==0) Ellipse(dc,x1-r/2,y1-r/2,x1+r/2,y1+r/2);
    else Ellipse(dc,x1-r/2,y1-r/2,x1+r/2+1,y1+r/2+1);
    DeleteObject(pen); SelectObject(dc,oldpen);
    DeleteObject(brush);SelectObject(dc,oldbrush);
}
void DrawLine(int x1,int y1,int x2,int y2,COLORREF color,UINT flags,int cusi)
{
    int y;
    HPEN pen,oldpen;
    HBRUSH brush,oldbrush;
    pen=CreatePen(flags,cusi,color);
    oldpen=(HPEN)SelectObject(bitmapdc,pen);
    MoveToEx(bitmapdc,x1,y1,NULL);
    LineTo(bitmapdc,x2,y2);
    DeleteObject(pen);
    SelectObject(bitmapdc,oldpen);
}
void AlphaBlt(HDC dc2,int x2,int y2,int width2,int height2,HDC dc1,int x1,int y1,int width1,int height1,char trans)
{
    BLENDFUNCTION a={0,0,0,0};
    a.AlphaFormat=0;
    a.SourceConstantAlpha=trans;
    AlphaBlend(dc2,x2,y2,width2,height2,dc1,x1,y1,width1,height1,a);
}
void ReverseBlt(HDC dc1,int x1,int y1,int width,int height,HDC dc2,int x2,int y2,int uflag,COLORREF trans)
{
    POINT RotatePoints[3];
    Bar(tempdc,0,0,0,640,480,RGB(255,255,255));
    if (uflag==Left90) {
        RotatePoints[0].x= 0;      RotatePoints[0].y= height;
        RotatePoints[1].x= 0;      RotatePoints[1].y= 0;
        RotatePoints[2].x= width;  RotatePoints[2].y= height;
    } else if (uflag==Right90) {
        RotatePoints[0].x= width;  RotatePoints[0].y= 0;
        RotatePoints[1].x= width;  RotatePoints[1].y= height;
        RotatePoints[2].x= 0;      RotatePoints[2].y= 0;
    } else if (uflag==HengBlt) {
        RotatePoints[0].x= width;  RotatePoints[0].y= 0;
        RotatePoints[1].x= 0;      RotatePoints[1].y= 0;
        RotatePoints[2].x= width;  RotatePoints[2].y= height;
    } else if (uflag==ShuBlt) {
        RotatePoints[0].x= 0;      RotatePoints[0].y= height;
        RotatePoints[1].x= width;  RotatePoints[1].y= height;
        RotatePoints[2].x= 0;      RotatePoints[2].y= 0;
    } else if (uflag==HengShuBlt) {
        RotatePoints[0].x= width;  RotatePoints[0].y= height;
        RotatePoints[1].x= 0;      RotatePoints[1].y= height;
        RotatePoints[2].x= width;  RotatePoints[2].y= 0;
    } else {
        RotatePoints[0].x= 0;      RotatePoints[0].y= 0;
        RotatePoints[1].x= width;  RotatePoints[1].y= 0;
        RotatePoints[2].x= 0;      RotatePoints[2].y= height;
    }
    PlgBlt (tempdc, RotatePoints, dc2,x2,y2 , width,height, 0, 0,0);
    TransparentBlt(dc1,x1,y1,width,height,tempdc,0,0,width,height,trans);
}
int RouteBlt(HDC dc1,int x1,int y1,int x,int y,int width,int height,HDC dc2,int routex,int routey,double angle)
{
    int oldwidth,oldheight,newwidth,newheight,xpos,ypos;
    double sinangle,cosangle;
    POINT Points[3],RotatePoints[3];
    sinangle=sin(angle*PI);
    cosangle=cos(angle*PI);
    oldwidth=width;
    oldheight=height;

    newwidth=round(fabs(sinangle)*oldheight+fabs(cosangle)*oldwidth+0.5);
    newheight=round(fabs(sinangle)*oldwidth+fabs(cosangle)*oldheight+0.5);
    xpos=routex;
    ypos=routey;
    Points[0].x= -x1;//-OldWidth div 2;
    Points[0].y= -y1;//-OldHeight div 2;
    Points[1].x= Points[0].x + oldwidth;
    Points[1].y= Points[0].y           ;
    Points[2].x= Points[0].x           ;
    Points[2].y= Points[0].y + oldheight;
    RotatePoints[0].x= round(Points[0].x * cosangle - Points[0].y * sinangle) + xpos;
    RotatePoints[0].y= round(Points[0].x * sinangle + Points[0].y * cosangle) + ypos;
    RotatePoints[1].x= round(Points[1].x * cosangle - Points[1].y * sinangle) + xpos;
    RotatePoints[1].y= round(Points[1].x * sinangle + Points[1].y * cosangle) + ypos;
    RotatePoints[2].x= round(Points[2].x * cosangle - Points[2].y * sinangle) + xpos;
    RotatePoints[2].y= round(Points[2].x * sinangle + Points[2].y * cosangle) + ypos;
    return PlgBlt (dc1, RotatePoints, dc2, x,y, oldwidth,oldheight, 0, 0,0);
}
extern ImaButton button1,button2,button3,button4,button5,button6,button7;
char temps[256];
int alpha=0,alpha2=0,ypos[4]={-30,-30,480,480},xpos[4]={0,800};
LARGE_INTEGER nowtime2,lasttime2;
extern LARGE_INTEGER frequent;
void print()
{
    int i,x,y;
    double t;
    POINT RotatePoints[3];
    POINT p;
    HPEN pen,oldpen;
    HBRUSH brush,oldbrush;
    LARGE_INTEGER st,ed;
    BLENDFUNCTION a={0,0,0,0};
    a.AlphaFormat=AC_SRC_ALPHA;
    a.SourceConstantAlpha=255;
    a.BlendFlags=0;
    a.BlendOp=AC_SRC_OVER;
    int size;

    Bar(bitmapdc,0,0,0,640,480,RGB(255,255,255));
    BitBlt(bitmapdc,0,0,640,432,sky[1].color,0,0,SRCCOPY);
    TransparentBlt(bitmapdc,0,316,640,116,backcity[3].color,minx/5,0,640,116,RGB(255,255,255));
    TransparentBlt(bitmapdc,0,316,640,116,backcity[2].color,minx/4,0,640,116,RGB(255,255,255));
    TransparentBlt(bitmapdc,0,316,640,116,backcity[1].color,minx/2,0,640,116,RGB(255,255,255));
    TransparentBlt(bitmapdc,0,0,640,480,building[2].color,minx,0,640,480,RGB(255,255,255));
    if (girl.now==LieOnTheBox) {
        TransparentBlt(bitmapdc,girl.x-minx,girl.y,38,19,lie.color,0,0,38,19,RGB(255,255,255));
    } else if (girl.now==LieOnGround) {

    }
    for (i=1; i<=200; i++) {
        if (zom[i].flag) {
            if (zom[i].hp>0) {
                if (!zom[i].walktoward) {
                    RotatePoints[0].x= 18;      RotatePoints[0].y= 0;
                    RotatePoints[1].x= -1;      RotatePoints[1].y= 0;
                    RotatePoints[2].x= 18;      RotatePoints[2].y= 32;
                    PlgBlt (tempdc, RotatePoints, bitmapdc,zom[i].x-minx,zom[i].y , 19,32, 0, 0,0);
                } else {
                    RotatePoints[0].x= 0;       RotatePoints[0].y= 0;
                    RotatePoints[1].x= 18;      RotatePoints[1].y= 0;
                    RotatePoints[2].x= 0;       RotatePoints[2].y= 32;
                    PlgBlt (tempdc, RotatePoints, bitmapdc,zom[i].x-minx,zom[i].y , 18,32, 0, 0,0);
                }
                TransparentBlt(tempdc,0,0,18,32,zombies[zom[i].lei].color,17*zom[i].walknow,0,18,32,RGB(255,255,255));
                if (!zom[i].walktoward) {
                    RotatePoints[0].x= zom[i].x-minx+18; RotatePoints[0].y= zom[i].y;
                    RotatePoints[1].x= zom[i].x-minx;    RotatePoints[1].y= zom[i].y;
                    RotatePoints[2].x= zom[i].x-minx+18; RotatePoints[2].y= zom[i].y+32;
                } else {
                    RotatePoints[0].x= zom[i].x-minx;    RotatePoints[0].y= zom[i].y;
                    RotatePoints[1].x= zom[i].x-minx+18; RotatePoints[1].y= zom[i].y;
                    RotatePoints[2].x= zom[i].x-minx;    RotatePoints[2].y= zom[i].y+32;
                }
                PlgBlt (tempdc3, RotatePoints, tempdc,0,0 , 18,32, 0, 0,0);
                if (zom[i].walktoward) AlphaBlt(bitmapdc,zom[i].x-minx,zom[i].y,18,32,tempdc3,zom[i].x-minx,zom[i].y,18,32,zom[i].trans);
                else AlphaBlt(bitmapdc,zom[i].x-minx+1,zom[i].y,18,32,tempdc3,zom[i].x-minx+1,zom[i].y,18,32,zom[i].trans);
            } else {
                if (!zom[i].walktoward) {
                    RotatePoints[0].x= 18;      RotatePoints[0].y= 32;
                    RotatePoints[1].x= 18;      RotatePoints[1].y= 0;
                    RotatePoints[2].x= -1;      RotatePoints[2].y= 32;
                    PlgBlt (tempdc, RotatePoints, bitmapdc,zom[i].x-minx,zom[i].y+6 , 32,18, 0, 0,0);
                } else {
                    RotatePoints[0].x= 18;      RotatePoints[0].y= 0;
                    RotatePoints[1].x= 18;      RotatePoints[1].y= 32;
                    RotatePoints[2].x= -1;      RotatePoints[2].y= 0;
                    PlgBlt (tempdc, RotatePoints, bitmapdc,zom[i].x-minx,zom[i].y+6 , 32,18, 0, 0,0);
                }
                TransparentBlt(tempdc,0,0,17,32,zombies[zom[i].lei].color,477,0,17,32,RGB(255,255,255));
                if (!zom[i].walktoward) {
                    RotatePoints[0].x= zom[i].x-minx+32;    RotatePoints[0].y= zom[i].y+18+6;
                    RotatePoints[1].x= zom[i].x-minx+32;    RotatePoints[1].y= zom[i].y+6;
                    RotatePoints[2].x= zom[i].x-minx;       RotatePoints[2].y= zom[i].y+18+6;
                } else {
                    RotatePoints[0].x= zom[i].x-minx;       RotatePoints[0].y= zom[i].y+18+6;
                    RotatePoints[1].x= zom[i].x-minx;       RotatePoints[1].y= zom[i].y+6;
                    RotatePoints[2].x= zom[i].x-minx+32;    RotatePoints[2].y= zom[i].y+18+6;
                }
                PlgBlt (tempdc3, RotatePoints, tempdc,0,0 , 18,32, 0, 0,0);
                AlphaBlt(bitmapdc,zom[i].x-minx,zom[i].y+6,32,18,tempdc3,zom[i].x-minx,zom[i].y+6,32,18,zom[i].trans);
            }
        }
    }
    if (mann.hp>0) {
        if (!mann.toward) {
            RotatePoints[0].x= 18;      RotatePoints[0].y= 0;
            RotatePoints[1].x= -1;      RotatePoints[1].y= 0;
            RotatePoints[2].x= 18;      RotatePoints[2].y= 33;
            PlgBlt (tempdc, RotatePoints, bitmapdc,mann.x-minx,mann.y , 19,33, 0, 0,0);
        } else {
            RotatePoints[0].x= 0;       RotatePoints[0].y= 0;
            RotatePoints[1].x= 18;      RotatePoints[1].y= 0;
            RotatePoints[2].x= 0;       RotatePoints[2].y= 33;
            PlgBlt (tempdc, RotatePoints, bitmapdc,mann.x-minx,mann.y , 18,33, 0, 0,0);
        }
        TransparentBlt(tempdc,0,0,18,33,man.color,17*mann.walknow,0,18,33,RGB(255,255,255));
        if (!mann.toward) {
            RotatePoints[0].x= mann.x-minx+18; RotatePoints[0].y= mann.y;
            RotatePoints[1].x= mann.x-minx;    RotatePoints[1].y= mann.y;
            RotatePoints[2].x= mann.x-minx+18; RotatePoints[2].y= mann.y+33;
        } else {
            RotatePoints[0].x= mann.x-minx;    RotatePoints[0].y= mann.y;
            RotatePoints[1].x= mann.x-minx+18; RotatePoints[1].y= mann.y;
            RotatePoints[2].x= mann.x-minx;    RotatePoints[2].y= mann.y+33;
        }
        PlgBlt (bitmapdc, RotatePoints, tempdc,0,0 , 18,33, 0, 0,0);
    } else { //主角挂了
        BitBlt(tempdc,0,0,33,18,bitmapdc,mann.x-minx,mann.y+33-18+6,SRCCOPY);
        Bar(tempdc2,0,0,0,33,18,RGB(255,255,255));
        if (!mann.toward) {
            RotatePoints[0].x= 33;      RotatePoints[0].y= 18;
            RotatePoints[1].x= 33;      RotatePoints[1].y= 0;
            RotatePoints[2].x= -1;      RotatePoints[2].y= 18;
            PlgBlt (tempdc2, RotatePoints, man.color,511,0 , 18,33, 0, 0,0);
        } else {
            RotatePoints[0].x= 0;       RotatePoints[0].y= 18;
            RotatePoints[1].x= -1;      RotatePoints[1].y= 0;
            RotatePoints[2].x= 33;      RotatePoints[2].y= 18;
            PlgBlt (tempdc2, RotatePoints, man.color,511,0 , 18,33, 0, 0,0);
        }
        TransparentBlt(tempdc,0,0,33,18,tempdc2,0,0,33,18,RGB(255,255,255));
        BitBlt(bitmapdc,mann.x-minx,mann.y+33-18+6,33,18,tempdc,0,0,SRCCOPY);
    }
    //显示主角武器
    Bar(tempdc2,0,0,0,640,480,RGB(255,255,255));
    if (mann.hp>0) {
        if (!mann.toward) {
            RotatePoints[0].x= hweapon[weanow].image.width/2;        RotatePoints[0].y= 0;
            RotatePoints[1].x= 0;                                   RotatePoints[1].y= 0;
            RotatePoints[2].x= hweapon[weanow].image.width/2;        RotatePoints[2].y= hweapon[weanow].image.height;
            PlgBlt (tempdc2, RotatePoints, hweapon[weanow].image.color,hweapon[weanow].image.width/2*mann.fire,0 , hweapon[weanow].image.width/2,hweapon[weanow].image.height, 0, 0,0);
        } else {
            RotatePoints[0].x= 0;                                   RotatePoints[0].y= 0;
            RotatePoints[1].x= hweapon[weanow].image.width/2;        RotatePoints[1].y= 0;
            RotatePoints[2].x= 0;                                   RotatePoints[2].y= hweapon[weanow].image.height;
            PlgBlt (tempdc2, RotatePoints, hweapon[weanow].image.color,hweapon[weanow].image.width/2*mann.fire,0 , hweapon[weanow].image.width/2,hweapon[weanow].image.height, 0, 0,0);
        }
        Bar(tempdc,0,0,0,640,480,RGB(255,255,255));
        if (mann.toward) RouteBlt(tempdc,hweapon[weanow].routex,hweapon[weanow].routey,0,0,hweapon[weanow].image.width/2,hweapon[weanow].image.height,tempdc2,mann.x-minx+8,mann.y+13,-(mann.jiao+mann.up));
        else RouteBlt(tempdc,hweapon[weanow].image.width/2-hweapon[weanow].routex,hweapon[weanow].routey,1,0,hweapon[weanow].image.width/2,hweapon[weanow].image.height,tempdc2,mann.x-minx+10,mann.y+13,180-(mann.jiao+mann.up));
        TransparentBlt(bitmapdc,0,0,640,480,tempdc,0,0,640,480,RGB(255,255,255));
        if (fabs(mann.showangle)>1e-6) {
            if (mann.showangle<=360&&mann.showangle>=355) {
                DrawCircle(bitmapdc,mann.x-minx+10,mann.y-10,20,0,mann.showcolor);
            } else {
                p=getlinepixel(mann.x+10-minx,mann.y-10,10,90+mann.showangle);
                brush=CreateSolidBrush(mann.showcolor);
                oldbrush=(HBRUSH)SelectObject(bitmapdc,brush);
                Pie(bitmapdc,mann.x-minx,mann.y-20,mann.x+20-minx,mann.y,mann.x+11-minx,mann.y-20,p.x,p.y);
                DeleteObject(brush); SelectObject(bitmapdc,oldbrush);
            }
            if (mann.show==1) {
                TransparentBlt(bitmapdc,mann.x+20-minx,mann.y-20,7,11,bul.color,0,0,7,11,RGB(255,255,255));
                RECT r;
                r.top=mann.y-20; r.left=mann.x+20-minx+7;
                r.bottom=mann.y-20+12; r.right=mann.x+20-minx+7+50;
                SetTextColor(bitmapdc,RGB(255,255,255));
                sprintf(temps,"%d/%d",hweapon[weanow].now,hweapon[weanow].left);
                DrawText(bitmapdc,temps,-1,&r,DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            }
        }
    } else { //挂了之后的武器摆放

    }
    for (i=1; i<=200; i++) {
        if (bull[i].flag) {
            x=round(bull[i].sx+(bull[i].length-20)*cos(bull[i].jiao*PI));
            y=round(bull[i].sy-(bull[i].length-20)*sin(bull[i].jiao*PI));
            if (getdis(bull[i].x,bull[i].y,x,y)<100) {
                DrawLine(bull[i].x-minx,bull[i].y,x-minx,y,RGB(254,215,0),PS_SOLID,1);

            }
        }
    }
    for (i=1; i<=200; i++)
        if (gr[i].flag) {
            t=gr[i].t+0.01;
            x=gr[i].nowx+round(t*gr[i].vx*100/5);
            y=gr[i].nowy-round((gr[i].vy*t-4.9*t*t)*100/5);
            if (!(((gr[i].x-minx==0)&&(gr[i].y==0))||((x-minx==0)||(y==0))))
                DrawLine(gr[i].x-minx,gr[i].y,x-minx,y,RGB(255,255,0),PS_SOLID,1);
        }
    for (i=1; i<=15; i++)
        if (boom[i].now<16) {
            boom[i].now=boom[i].now+0.4;
            if (floor(boom[i].now)>0&&floor(boom[i].now)<16) {
                if (boom[i].boomflag==1) ReverseBlt(bitmapdc,boom[i].x-49-minx,boom[i].y-93,120,109,bopic[(int)(boom[i].now)].color,0,0,SameBlt,RGB(255,255,255));
                else if (boom[i].boomflag==2) ReverseBlt(bitmapdc,boom[i].x-49-minx,boom[i].y-109+93,120,109,bopic[(int)(boom[i].now)].color,0,0,ShuBlt,RGB(255,255,255));
                else if (boom[i].boomflag==3) ReverseBlt(bitmapdc,boom[i].x-93-minx,boom[i].y-120+49,120,109,bopic[(int)(boom[i].now)].color,0,0,Left90,RGB(255,255,255));
                else if (boom[i].boomflag==4) ReverseBlt(bitmapdc,boom[i].x-109+93-minx,boom[i].y-49,120,109,bopic[(int)(boom[i].now)].color,0,0,Right90,RGB(255,255,255));
            }
        }
    a.AlphaFormat=AC_SRC_ALPHA;
    a.SourceConstantAlpha=255;
    a.BlendFlags=0;
    a.BlendOp=AC_SRC_OVER;
    for (i=0; i<5000; i++)
        if (blood[i].flag){
            SetPixel(bitmapdc,blood[i].x-minx,blood[i].y,RGB(255,0,0));
        }
    for (i=0; i<100; i++)
        if (smog[i].flag){
            size=10+round(10*smog[i].t);
            if (size>50) size=50;
            AlphaBlend(bitmapdc,smog[i].x-minx-size/2,smog[i].y-size/2,size,size,smogpic.color,0,0,50,50,a);
        }
    AlphaBlt(bitmapdc,0,0,640,480,red,0,0,640,480,turnred);
    RECT r;
    r.top=0; r.left=60;
    r.bottom=30; r.right=480;
    SetTextColor(bitmapdc,RGB(255,255,255));
    SelectObject(bitmapdc,thefont);
    sprintf(temps,"HP: %.0lf Money: %d          Wave: %d",round(mann.hp),mann.money,wave);
    DrawText(bitmapdc,temps,-1,&r,DT_SINGLELINE| DT_VCENTER);
    QueryPerformanceCounter(&nowtime2);
    if (pause) {
        alpha=alpha+round(512*((double)(nowtime2.QuadPart-lasttime2.QuadPart)/frequent.QuadPart));
        if (alpha>255) alpha=255;
        AlphaBlt(bitmapdc,320-100,240-125,200,250,menu.color,0,0,200,250,alpha);
        ypos[0]+=round((MenuKeepY+15-ypos[0])*5*((double)(nowtime2.QuadPart-lasttime2.QuadPart)/frequent.QuadPart));
        if (ypos[0]>MenuKeepY) ypos[0]=MenuKeepY;
        SetWindowPos(button2.hwindow,0,320-80,ypos[0],0,0,SWP_NOZORDER|SWP_NOSIZE);
        ypos[1]+=round((MenuBackY+15-ypos[1])*5*((double)(nowtime2.QuadPart-lasttime2.QuadPart)/frequent.QuadPart));
        if (ypos[1]>MenuBackY) ypos[1]=MenuBackY;
        SetWindowPos(button5.hwindow,0,320-80,ypos[1],0,0,SWP_NOZORDER|SWP_NOSIZE);
        ypos[2]-=round((ypos[2]-MenuSaveY+15)*5*((double)(nowtime2.QuadPart-lasttime2.QuadPart)/frequent.QuadPart));
        if (ypos[2]<MenuSaveY) ypos[2]=MenuSaveY;
        SetWindowPos(button6.hwindow,0,320-80,ypos[2],0,0,SWP_NOZORDER|SWP_NOSIZE);
        ypos[3]-=round((ypos[3]-MenuExitY+15)*5*((double)(nowtime2.QuadPart-lasttime2.QuadPart)/frequent.QuadPart));
        if (ypos[3]<MenuExitY) ypos[3]=MenuExitY;
        SetWindowPos(button7.hwindow,0,320-80,ypos[3],0,0,SWP_NOZORDER|SWP_NOSIZE);
    } else {
        alpha=(alpha>0)?alpha-round(512*((double)(nowtime2.QuadPart-lasttime2.QuadPart)/frequent.QuadPart)):0;
        if (alpha>0) AlphaBlt(bitmapdc,320-100,240-125,200,250,menu.color,0,0,200,250,alpha);
        ypos[0]-=round((ypos[0]+45)*5*((double)(nowtime2.QuadPart-lasttime2.QuadPart)/frequent.QuadPart));
        if (ypos[0]<-30) ypos[0]=-30;
        SetWindowPos(button2.hwindow,0,320-80,ypos[0],0,0,SWP_NOZORDER|SWP_NOSIZE);
        ypos[1]-=round((ypos[1]+45)*5*((double)(nowtime2.QuadPart-lasttime2.QuadPart)/frequent.QuadPart));
        if (ypos[1]<-30) ypos[1]=-30;
        SetWindowPos(button5.hwindow,0,320-80,ypos[1],0,0,SWP_NOZORDER|SWP_NOSIZE);
        ypos[2]+=round((495-ypos[2])*5*((double)(nowtime2.QuadPart-lasttime2.QuadPart)/frequent.QuadPart));
        if (ypos[2]>480) ypos[2]=480;
        SetWindowPos(button6.hwindow,0,320-80,ypos[2],0,0,SWP_NOZORDER|SWP_NOSIZE);
        ypos[3]+=round((495-ypos[3])*5*((double)(nowtime2.QuadPart-lasttime2.QuadPart)/frequent.QuadPart));
        if (ypos[3]>480) ypos[3]=480;
        SetWindowPos(button7.hwindow,0,320-80,ypos[3],0,0,SWP_NOZORDER|SWP_NOSIZE);
    }
    if (IsWindowVisible(button1.hwindow)) SendMessage(button1.hwindow,WM_PAINT,(WPARAM)bitmapdc,0);
    if (IsWindowVisible(button2.hwindow)) SendMessage(button2.hwindow,WM_PAINT,(WPARAM)bitmapdc,0);
    if (IsWindowVisible(button5.hwindow)) SendMessage(button5.hwindow,WM_PAINT,(WPARAM)bitmapdc,0);
    if (IsWindowVisible(button6.hwindow)) SendMessage(button6.hwindow,WM_PAINT,(WPARAM)bitmapdc,0);
    if (IsWindowVisible(button7.hwindow)) SendMessage(button7.hwindow,WM_PAINT,(WPARAM)bitmapdc,0);
    lasttime2=nowtime2;
    AlphaBlt(bitmapdc,0,0,640,480,black,0,0,640,480,turnblack);
    #ifndef _SHOW_WAY_

    #endif // _SHOW_WAY_
    //RECT r;
    //GetClientRect(hWindow,&r);
    //DrawText(windc,IntToStr(zhen[zhennow][2]*zhenhan),-1,&r,DT_SINGLELINE | DT_CENTER | DT_VCENTER);
}
LARGE_INTEGER nowtime3,lasttime3;
void PrintBox()
{
    //BitBlt(bitmapdc,0,0,640,480,save,0,0,SRCCOPY);
    QueryPerformanceCounter(&nowtime3);
    if (Box) {
        alpha2=alpha2+round(512*((double)(nowtime3.QuadPart-lasttime3.QuadPart)/frequent.QuadPart));
        if (alpha2>255) alpha2=255;
        AlphaBlt(bitmapdc,320-171/2,240-65/2,171,65,box.color,0,0,171,65,alpha2);
        xpos[0]+=round((BoxCancelX+15-xpos[0])*5*((double)(nowtime3.QuadPart-lasttime3.QuadPart)/frequent.QuadPart));
        if (xpos[0]>BoxCancelX) xpos[0]=BoxCancelX;
        SetWindowPos(button3.hwindow,0,xpos[0],BoxOKY,0,0,SWP_NOZORDER|SWP_NOSIZE);
        xpos[1]-=round((xpos[1]-BoxOKX+15)*5*((double)(nowtime3.QuadPart-lasttime3.QuadPart)/frequent.QuadPart));
        if (xpos[1]<BoxOKX) xpos[1]=BoxOKX;
        SetWindowPos(button4.hwindow,0,xpos[1],BoxOKY,0,0,SWP_NOZORDER|SWP_NOSIZE);
    } else {
        alpha2=(alpha2>0)?alpha2-round(512*((double)(nowtime3.QuadPart-lasttime3.QuadPart)/frequent.QuadPart)):0;
        if (alpha2>0) AlphaBlt(bitmapdc,320-171/2,240-65/2,171,65,box.color,0,0,171,65,alpha2);
        xpos[0]-=round((xpos[0]+95)*5*((double)(nowtime3.QuadPart-lasttime3.QuadPart)/frequent.QuadPart));
        if (xpos[0]<-82) xpos[0]=-82;
        SetWindowPos(button3.hwindow,0,xpos[0],BoxOKY,0,0,SWP_NOZORDER|SWP_NOSIZE);
        xpos[1]+=round((640+15-xpos[1])*5*((double)(nowtime3.QuadPart-lasttime3.QuadPart)/frequent.QuadPart));
        if (xpos[1]>640) xpos[1]=640;
        SetWindowPos(button4.hwindow,0,xpos[1],BoxOKY,0,0,SWP_NOZORDER|SWP_NOSIZE);
    }
    if (IsWindowVisible(button3.hwindow)) SendMessage(button3.hwindow,WM_PAINT,(WPARAM)bitmapdc,0);
    if (IsWindowVisible(button4.hwindow)) SendMessage(button4.hwindow,WM_PAINT,(WPARAM)bitmapdc,0);
    lasttime3=nowtime3;
}
HFONT Font1;
void PrintShop()
{
    const int left=ShopWeaWindowLeft;
    const int top=ShopWeaWindowTop;
    const int bottom=ShopWeaWindowBottom;
    const int right=ShopWeaWindowRight;
    POINT RotatePoints[3];
    POINT p;
    int i,width,height;
    RECT r;
    HFONT oldfont;
    Bar(bitmapdc,0,0,0,640,480,RGB(122,114,105));
    Bar(bitmapdc,0,0,0,640,480,RGB(255,255,255));

    i=choosenow;
    //画主角
    mann.x=30; mann.y=50;
    TransparentBlt(tempdc,0,0,18,33,man.color,0,0,18,33,RGB(255,255,255));
    RotatePoints[0].x= mann.x;    RotatePoints[0].y= mann.y;
    RotatePoints[1].x= mann.x+18; RotatePoints[1].y= mann.y;
    RotatePoints[2].x= mann.x;    RotatePoints[2].y= mann.y+33;
    PlgBlt (bitmapdc, RotatePoints, tempdc,0,0 , 18,33, 0, 0,0);
    //武器
    RotatePoints[0].x= 0;                                   RotatePoints[0].y= 0;
    RotatePoints[1].x= weapon[i].image.width/2;        RotatePoints[1].y= 0;
    RotatePoints[2].x= 0;                                   RotatePoints[2].y= weapon[i].image.height;
    PlgBlt (tempdc2, RotatePoints, weapon[i].image.color,weapon[i].image.width/2*mann.fire,0 , weapon[i].image.width/2,weapon[i].image.height, 0, 0,0);
    Bar(tempdc,0,0,0,640,480,RGB(255,255,255));
    RouteBlt(tempdc,weapon[i].routex,weapon[i].routey,0,0,weapon[i].image.width/2,weapon[i].image.height,tempdc2,mann.x+8,mann.y+13,0);
    TransparentBlt(bitmapdc,0,0,640,480,tempdc,0,0,640,480,RGB(255,255,255));

    width=weapon[i].simage.width;
    height=weapon[i].simage.height;
    if (width>(right-left)||height>(bottom-top)) {
        width=right-left;
        height=round((double)(right-left)/weapon[i].simage.width*weapon[i].simage.height);
        if (height>(bottom-top)) {
            height=bottom-top;
            width=round((double)(bottom-top)/weapon[i].simage.height*weapon[i].simage.width);
        }
    }
    TransparentBlt(bitmapdc,left+(right-left-width)/2,top+(bottom-top-height)/2,width,height,weapon[i].simage.color,0,0,weapon[i].simage.width,weapon[i].simage.height,RGB(255,255,255));

    r.top=100; r.left=30;
    r.bottom=480; r.right=left;
    SetTextColor(bitmapdc,RGB(0,0,0));
    oldfont=(HFONT)SelectObject(bitmapdc,Font1);
    sprintf(temps,"");
    DrawText(bitmapdc,temps,-1,&r,0);
    r.top=bottom; r.left=left;
    r.bottom=480; r.right=right;
    SetTextColor(bitmapdc,RGB(0,0,0));
    sprintf(temps,"名称：%s\n射速：%d\n杀伤：%d\n弹夹容量：%d\n口径：%.2lf\n\n价格：%d",weapon[choosenow].name,weapon[choosenow].speed,weapon[choosenow].damage,weapon[choosenow].danjia,weapon[choosenow].diameter,weapon[choosenow].price);
    DrawText(bitmapdc,temps,-1,&r,0);
    r.top=0; r.left=550;
    r.bottom=40; r.right=480;
    sprintf(temps,"金钱：%d",mann.money);
    DrawText(bitmapdc,temps,-1,&r,0);
    SelectObject(bitmapdc,oldfont);
    if (IsWindowVisible(bprevious.hwindow)) SendMessage(bprevious.hwindow,WM_PAINT,(WPARAM)bitmapdc,0);
    if (IsWindowVisible(bnext.hwindow)) SendMessage(bnext.hwindow,WM_PAINT,(WPARAM)bitmapdc,0);
    if (IsWindowVisible(bbuy.hwindow)) SendMessage(bbuy.hwindow,WM_PAINT,(WPARAM)bitmapdc,0);
    if (IsWindowVisible(bcontinue.hwindow)) SendMessage(bcontinue.hwindow,WM_PAINT,(WPARAM)bitmapdc,0);
    if (IsWindowVisible(bexit.hwindow)) SendMessage(bexit.hwindow,WM_PAINT,(WPARAM)bitmapdc,0);
}
DWORD ThreadID4;
HANDLE Handle4;
extern bool printnow;
DWORD WINAPI Thread4_print(LPVOID pParam)
{
    QueryPerformanceCounter(&lasttime2);
    QueryPerformanceCounter(&lasttime3);
    MakeFont("华文中宋",16,&Font1);
    while (1) {
        if ((printnow||pause||quit)&&!shop) {
            print();
            printnow=false;
        }
        if (shop) {
            PrintShop();
        }
        PrintBox();
        BitBlt(windc,zhen[zhennow][0]*zhenhan,zhen[zhennow][1]*zhenhan,640,480,bitmapdc,0,0,SRCCOPY);
        Sleep(1);
    }
}
HWND WinCreate(HINSTANCE hInstance)
{
    const int mode=HALFTONE;
    HWND hWindow;
    maxx=640+GetSystemMetrics(SM_CYFIXEDFRAME);
    maxy=480+GetSystemMetrics(SM_CYFIXEDFRAME)+GetSystemMetrics(SM_CYCAPTION);
    hWindow=CreateWindow("丧尸之城","丧尸之城",WS_CAPTION|WS_SYSMENU,CW_USEDEFAULT,CW_USEDEFAULT,
                         maxx,maxy,0,0,hInstance,NULL);
    if (hWindow!=0) {
        ShowWindow(hWindow,SW_SHOW);
        UpdateWindow(hWindow);
    }
    windc=GetDC(hWindow);
    bitmapdc=CreateCompatibleDC(windc);
    savescreen=CreateCompatibleBitmap(windc,maxx,maxy);
    SelectObject(bitmapdc,savescreen);
    Bar(bitmapdc,0,0,0,640,480,RGB(255,255,255));
    makefont(12,&thefont);
    SetTextColor(bitmapdc,RGB(0,0,255));
    SelectObject(bitmapdc,thefont);
    SetBkMode(bitmapdc,1);
    SetStretchBltMode(bitmapdc,mode);

    save=CreateCompatibleDC(windc);
    savescreen=CreateCompatibleBitmap(windc,maxx,maxy);
    SelectObject(save,savescreen);
    Bar(save,0,0,0,640,480,RGB(255,255,255));
    makefont(12,&thefont);
    SetTextColor(save,RGB(0,0,255));
    SelectObject(save,thefont);
    SetBkMode(save,1);
    SetStretchBltMode(save,mode);

    tempdc=CreateCompatibleDC(windc);
    savedscreen=CreateCompatibleBitmap(windc,maxx,maxy);
    SelectObject(tempdc,savedscreen);
    Bar(tempdc,0,0,0,640,480,RGB(255,255,255));
    SetTextColor(tempdc,RGB(0,0,255));
    SelectObject(tempdc,thefont);
    SetBkMode(tempdc,1);
    SetStretchBltMode(tempdc,mode);

    tempdc2=CreateCompatibleDC(windc);
    savedscreen=CreateCompatibleBitmap(windc,maxx,maxy);
    SelectObject(tempdc2,savedscreen);
    Bar(tempdc2,0,0,0,640,480,RGB(255,255,255));
    SetTextColor(tempdc2,RGB(0,0,255));
    SelectObject(tempdc2,thefont);
    SetBkMode(tempdc2,1);
    SetStretchBltMode(tempdc2,mode);

    tempdc3=CreateCompatibleDC(windc);
    savedscreen=CreateCompatibleBitmap(windc,maxx,maxy);
    SelectObject(tempdc3,savedscreen);
    Bar(tempdc3,0,0,0,640,480,RGB(255,255,255));
    SetTextColor(tempdc3,RGB(0,0,255));
    SelectObject(tempdc3,thefont);
    SetBkMode(tempdc3,1);
    SetStretchBltMode(tempdc3,mode);

    red=CreateCompatibleDC(windc);
    savedscreen=CreateCompatibleBitmap(windc,maxx,maxy);
    SelectObject(red,savedscreen);
    Bar(red,0,0,0,640,480,RGB(255,0,0));
    SetTextColor(red,RGB(0,0,255));
    SelectObject(red,thefont);
    SetBkMode(red,1);
    SetStretchBltMode(red,mode);

    black=CreateCompatibleDC(windc);
    savedscreen=CreateCompatibleBitmap(windc,maxx,maxy);
    SelectObject(black,savedscreen);
    Bar(black,0,0,0,640,480,RGB(0,0,0));
    SetTextColor(black,RGB(0,0,0));
    SelectObject(black,thefont);
    SetBkMode(black,1);
    SetStretchBltMode(black,mode);

    Handle4=CreateThread(NULL,0,Thread4_print,NULL,0,&ThreadID4);
    return hWindow;
}
int GetIt(int x,int y)
{
    const int mmax=10;
    COLORREF color;
    int left,right,top,bottom;
    color=GetPixelAtBmp(&formmark,x,y);
    if (x<0||y<0||x>1071||y>479) return GI_OUTOFRANGE;
    else if (color==RGB(255,0,0)) return GI_DOOR1;
    else if (color==RGB(0,255,0)) return GI_DOOR2;
    else if (color==RGB(0,0,255)) return GI_DOOR3;
    else if (color==RGB(255,255,0)) {
        left=x-1;
        while (left>=0&&GetPixelAtBmp(&formmark,left,y)==RGB(255,255,0)) left--;
        left=abs(x-left);
        right=x+1;
        while ((right<=1071)&&(GetPixelAtBmp(&formmark,right,y)==RGB(255,255,0))) right++;
        right=abs(x-right);
        top=y-1;
        while ((top>=0)&&(GetPixelAtBmp(&formmark,x,top)==RGB(255,255,0))) top--;
        top=abs(y-top);
        bottom=y+1;
        while ((bottom<=479)&&(GetPixelAtBmp(&formmark,x,bottom)==RGB(255,255,0)))bottom++;
        bottom=abs(y-bottom);
        if (left<right&&left<mmax) {
            if (left<top&&left<bottom) return GI_LEFT;
            else if (top<bottom) return GI_TOP;
            else return GI_BOTTOM;
        } else if (left>right&&right<mmax) {
            if (right<top&&right<bottom) return GI_RIGHT;
            else if (top<bottom) return GI_TOP;
            else return GI_BOTTOM;
        } else if (top<bottom&&top<mmax) {
            return GI_TOP;
        } else if (top>bottom&&bottom<mmax) {
            return GI_BOTTOM;
        }
    }
}
int GetItAtZombie(int x,int y,int which,int walknow)
{
    const int mmax=2;
    COLORREF color;
    int left,right,top,bottom;
    color=GetPixelAtBmp(&formzombie[which],x+17*walknow,y);
    if (x<0||y<0||x>18||y>32) return GI_OUTOFRANGE;
    else if (color==RGB(255,255,255)) return GI_NOTHING;
    else if (color!=RGB(255,255,255)) {
        left=x-1;
        while (left>=0&&GetPixelAtBmp(&formzombie[which],left+17*walknow,y)!=RGB(255,255,255)) left--;
        left=abs(x-left);
        right=x+1;
        while ((right<=32)&&(GetPixelAtBmp(&formzombie[which],right+17*walknow,y)!=RGB(255,255,255))) right++;
        right=abs(x-right);
        top=y-1;
        while ((top>=0)&&(GetPixelAtBmp(&formzombie[which],x+17*walknow,top)!=RGB(255,255,255))) top--;
        top=abs(y-top);
        bottom=y+1;
        while ((bottom<=18)&&(GetPixelAtBmp(&formzombie[which],x+17*walknow,bottom)!=RGB(255,255,255)))bottom++;
        bottom=abs(y-bottom);
        if (left<right&&left<mmax) {
            if (left<top&&left<bottom) return GI_LEFT;
            else if (top<bottom) return GI_TOP;
            else return GI_BOTTOM;
        } else if (left>right&&right<mmax) {
            if (right<top&&right<bottom) return GI_RIGHT;
            else if (top<bottom) return GI_TOP;
            else return GI_BOTTOM;
        } else if (top<bottom&&top<mmax) {
            return GI_TOP;
        } else if (top>bottom&&bottom<mmax) {
            return GI_BOTTOM;
        }
    }
}
