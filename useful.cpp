#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <GdiPlus.h>
#include "useful.h"
#include <stdlib.h>
#include <stdio.h>
#pragma comment (lib,"Gdiplus.lib")
#define   COMPILE_MULTIMON_STUBS
#include   <winuser.h>
extern HINSTANCE Hinstance;
LPImaButton now[10000];
int ibsum;
UINT DPI_x,DPI_y;
Gdiplus::GdiplusStartupInput vStartupInput;
ULONG vToken;
typedef enum _MONITOR_DPI_TYPE {
  MDT_EFFECTIVE_DPI  = 0,
  MDT_ANGULAR_DPI    = 1,
  MDT_RAW_DPI        = 2,
  MDT_DEFAULT        = MDT_EFFECTIVE_DPI
} Monitor_DPI_Type;
typedef enum _PROCESS_DPI_AWARENESS {
  PROCESS_DPI_UNAWARE            = 0,
  PROCESS_SYSTEM_DPI_AWARE       = 1,
  PROCESS_PER_MONITOR_DPI_AWARE  = 2
} PROCESS_DPI_AWARENESS;

typedef HRESULT (WINAPI *pGetDpiForMonitor)(HMONITOR,Monitor_DPI_Type,UINT*,UINT*);
pGetDpiForMonitor GetDpiForMonitor;

typedef HRESULT (WINAPI *pSetProcessDpiAwareness)(PROCESS_DPI_AWARENESS value);
pSetProcessDpiAwareness SetProcessDpiAwareness;

typedef int (WINAPI *pGdipCreateHBITMAPFromBitmap)(HANDLE,HBITMAP*,ULONG);
pGdipCreateHBITMAPFromBitmap GdipCreateHBITMAPFromBitmap;

typedef int (WINAPI *pGdipCreateBitmapFromFile)(WCHAR*, HANDLE **);
pGdipCreateBitmapFromFile GdipCreateBitmapFromFile;

typedef int (WINAPI *pGdipDisposeImage)(HANDLE*);
pGdipDisposeImage GdipDisposeImage;
inline int mmin(int x1,int x2)
{
    return x1<x2?x1:x2;
}
inline int mmax(int x1,int x2)
{
    return x1>x2?x1:x2;
}
char *IntToStr(int x)
{
    static char s[20];
    itoa(x,s,10);
    return s;
}
int StrToInt(char *s)
{
    return atoi(s);
}
char *RealToStr(double x)
{
    static char s[25];
    sprintf(s,"%lf",x);
    return s;
}
double StrToReal(char *s)
{
    return atof(s);
}
int Gcd(int n,int m)
{
    if (n%m!=0)
        return Gcd(m,n%m);
    else
        return m;
}
double DoubleMod(double x1,double x2)
{
    x1=x1-x2*floor(x1/x2);
    return x1;
}
int pos(char c,char *s)
{
    if (strchr(s,c)!=NULL)
        return strchr(s,c)-s+1;
    else
        return 0;
}
int pos(char *c,char *s)
{
    if (strstr(s,c)!=NULL)
        return strstr(s,c)-s+1;
    else
        return 0;
}

void Bar(HDC dc,HDC dc2,int x1,int y1,int x2,int y2,COLORREF color)
{
    int y;
    HPEN pen,oldpen;
    HBRUSH brush,oldbrush;
    pen=CreatePen(PS_SOLID,1,color);
    oldpen=(HPEN)SelectObject(dc,pen);
    brush=(HBRUSH)CreateSolidBrush(color);
    oldbrush=(HBRUSH)SelectObject(dc,brush);
    Rectangle(dc,x1,y1,x2+1,y2+1);
    DeleteObject(pen); SelectObject(dc,oldpen);
    DeleteObject(brush); SelectObject(dc,oldbrush);
    if (dc2!=0) BitBlt(dc2,x1,y1,x2-x1+1,y2-y1+1,dc,x1,y1,SRCCOPY);
}
void makefont(int height,HFONT *thefont)
{
    LOGFONT font;

    if (*thefont!=0) {
        DeleteObject(*thefont);
        *thefont=0;
    }
    font.lfHeight=height;
    font.lfWidth=0;
    font.lfEscapement=0;
    font.lfWeight=400;
    font.lfItalic=0;
    font.lfUnderline=0;
    font.lfStrikeOut=0;
    font.lfCharSet=134;
    font.lfOutPrecision=3;
    font.lfClipPrecision=2;
    font.lfQuality=1;
    font.lfPitchAndFamily=49;
    strcpy(font.lfFaceName,"ËÎÌו");
    *thefont=CreateFontIndirect(&font);
}
void MakeFont(char *name,int height,HFONT *thefont)
{
    LOGFONT font;
    if ((*thefont)!=0) {
        DeleteObject(*thefont); *thefont=0;
    }
    font.lfHeight=height;
    font.lfWidth=0;
    font.lfEscapement=0;
    font.lfWeight=400;
    font.lfItalic=0;
    font.lfUnderline=0;
    font.lfStrikeOut=0;
    font.lfCharSet=134;
    font.lfOutPrecision=3;
    font.lfClipPrecision=2;
    font.lfQuality=1;
    font.lfPitchAndFamily=49;
    strcpy(font.lfFaceName,name);
    *thefont=CreateFontIndirect(&font);
}
void loadpic(char *path,tbmp *m)
{
    HANDLE* bmp;
    BITMAPINFOHEADER vbitmapinfo;
    HBITMAP vbitmap;
    DIBSECTION vdibsection;
    HDC nowdc;
    wchar_t* s;
    int len,i;
    if (m->colorbmp!=NULL) DeleteObject(m->colorbmp);
    if (m->color!=NULL) DeleteObject(m->color);
    len=strlen(path);
    bmp=NULL;
    s=(wchar_t*)malloc((len+1)*sizeof(wchar_t));
    memset(s,0,sizeof(s));
    MultiByteToWideChar(CP_ACP,0,path,len+1,s,len+1);
    GdipCreateBitmapFromFile(s,&bmp);
    GdipCreateHBITMAPFromBitmap(bmp,&m->colorbmp,RGB(0,0,0));
    vbitmapinfo.biSize=sizeof(vbitmapinfo);
    GetObject(m->colorbmp,sizeof(vdibsection),&vdibsection);
    m->width=vdibsection.dsBm.bmWidth;
    m->height=vdibsection.dsBm.bmHeight;
    nowdc=GetDC(0);
    m->color=CreateCompatibleDC(nowdc);
    ReleaseDC(0,nowdc);
    SelectObject(m->color,m->colorbmp);
    GdipDisposeImage(bmp);
    free(s);
}
LRESULT WINAPI MouseProc(int nCode,WPARAM WParam,LPARAM LParam)
{
    MSLLHOOKSTRUCT *a;
    int i;
    for (i=1; i<=ibsum; i++) {
        a=(MSLLHOOKSTRUCT*)LParam;
        //a->pt.x=round(a->pt.x*96.0/120.0);
        //a->pt.y=round(a->pt.y*96.0/120.0);
        PostMessage(now[i]->hwindow,WParam,100000000,MAKELONG(a->pt.x,a->pt.y));
    }
}
void ReadConfig2(char *path,ImaButton *item)
{
    FILE *f;
    int i,readima;
    char com[256],param[256],st[256];
    f=fopen(path,"r");
    while (!feof(f)) {
        fscanf(f,"%s",st);
        for (i=0; i<strlen(st); i++)
            if (st[i]>='A'&&st[i]<='Z')
                st[i]=st[i]+32;
        if (strchr(st,'=')==NULL) {
            strcpy(com,""); strcpy(param,"");
        } else {
            strcpy(com,st); com[pos('=',com)-1]='\0';
            strcpy(param,strchr(st,'=')+1);
        }
        if (strcmp(com,"window")==0) strcpy(item->imagepath,param);
        else if (strcmp(com,"width")==0) item->width=StrToInt(param);
        else if (strcmp(com,"height")==0) item->height=StrToInt(param);
        else if (strcmp(com,"edgewidth")==0) item->edgewidth=StrToInt(param);
        else if (strcmp(com,"edgeheight")==0) item->edgeheight=StrToInt(param);
    }
    fclose(f);
}
void CreateImaButton(ImaButton *item,char *name,HWND father,DWORD Style,int x,int y,int width,int height,int num,char *imapath)
{
    char temp[256];
    int i,j;
    HRGN rgn;
    HBITMAP thebmp;
    sprintf(temp,"%sconfig.ini",imapath);
    ReadConfig2(temp,item);
    item->x=x; item->y=y;
    item->father=father; item->num=num;
    strcpy(item->text,name);
    sprintf(temp,"%s%s",imapath,item->imagepath);
    loadpic(temp,&item->image);
    strcpy(item->imagepath,imapath);
    item->autosize=false; item->noww=width; item->nowh=height;
    if ((Style&IBS_AUTOSIZE)==IBS_AUTOSIZE) {
        item->noww=item->image.width/3; item->nowh=item->image.height;
        item->rgn1=CreateRectRgn(0,0,0,0);
        for (i=0; i<=item->noww; i++)
            for (j=0; j<=item->nowh; j++)
                if (GetPixel(item->image.color,i,j)!=RGB(255,0,255)) {
                    rgn=CreateRectRgn(i,j,i+1,j+1);
                    CombineRgn(item->rgn1,item->rgn1,rgn,RGN_OR);
                    DeleteObject(rgn);
                }
        item->rgn2=CreateRectRgn(0,0,0,0);
        for (i=0; i<=item->noww; i++)
            for (j=0; j<=item->nowh; j++)
                if (GetPixel(item->image.color,i+item->noww,j)!=RGB(255,0,255)) {
                    rgn=CreateRectRgn(i,j,i+1,j+1);
                    CombineRgn(item->rgn2,item->rgn2,rgn,RGN_OR);
                    DeleteObject(rgn);
                }
        item->rgn3=CreateRectRgn(0,0,0,0);
        for (i=0; i<=item->noww; i++)
            for (j=0; j<=item->nowh; j++)
                if (GetPixel(item->image.color,i+2*item->noww,j)!=RGB(255,0,255)) {
                    rgn=CreateRectRgn(i,j,i+1,j+1);
                    CombineRgn(item->rgn3,item->rgn3,rgn,RGN_OR);
                    DeleteObject(rgn);
                }
        item->autosize=true;
    } else {
        item->rgn1=CreateRectRgn(0,0,item->noww,item->nowh);
        item->rgn2=CreateRectRgn(0,0,item->noww,item->nowh);
        item->rgn3=CreateRectRgn(0,0,item->noww,item->nowh);
    }
    item->maxx=item->noww; item->maxy=item->nowh;
    item->hwindow=CreateWindowEx(0,ImaButtonName,"",WS_CHILD|WS_VISIBLE,x,y,item->maxx,item->maxy,father,0,Hinstance,NULL);
    item->dc=GetDC(item->hwindow);
    MakeFont("ËÎÌו",12,&item->font);
    item->save=CreateCompatibleDC(item->dc);
    thebmp=CreateCompatibleBitmap(item->dc,2000,2000);
    SelectObject(item->save,thebmp);
    SelectObject(item->save,item->font);
    SetBkMode(item->save,1);
    SetTextColor(item->save,RGB(255,255,255));
    Bar(item->save,0,0,0,item->maxx,item->maxy,RGB(255,255,255));
    SetTimer(item->hwindow,num,10,NULL);
    item->laststate=-1;
    ibsum++; now[ibsum]=item;
}
LPImaButton FindImaButton(HWND window)
{
    int i;
    for (i=1; i<=ibsum; i++)
        if (now[i]->hwindow==window) return now[i];
    return NULL;
}
void display(LPImaButton ib,int state)
{
    BLENDFUNCTION a={0,0,0,0};
    int width,height,temp,i,j;
    HRGN rgn;
    RECT r,rr;
    bool redraw=false;
    a.AlphaFormat=0;
    a.SourceConstantAlpha=32;
    width=ib->image.width/3;
    if (!ib->autosize) height=ib->image.height/2;
    else height=ib->image.height;
    //BitBlt(ib->save,0,0,ib->maxx,ib->maxy,ib->dc,0,0,SRCCOPY);
    if (state==0) {
        if (ib->laststate!=state) {
            //rgn=CreateRectRgn(0,0,0,0);
            //CombineRgn(rgn,rgn,ib->rgn1,RGN_OR);
            //SetWindowRgn(ib->hwindow,ib->rgn1,true);
            redraw=true;
            //DeleteObject(rgn);
        }
        AlphaBlend(ib->save,ib->edgewidth,ib->edgeheight,ib->maxx-2*ib->edgewidth,ib->maxy-2*ib->edgeheight,ib->image.color,0,0,width,height,a);
    } else if (state==1) {
        if (ib->laststate!=state) {
            //rgn=CreateRectRgn(0,0,0,0);
            //CombineRgn(rgn,rgn,ib->rgn2,RGN_OR);
            //SetWindowRgn(ib->hwindow,rgn,false);
            redraw=true;
            //DeleteObject(rgn);
        }
        AlphaBlend(ib->save,ib->edgewidth,ib->edgeheight,ib->maxx-2*ib->edgewidth,ib->maxy-2*ib->edgeheight,ib->image.color,width,0,width,height,a);
    } else if (state==2) {
        if (ib->laststate!=state) {
            //rgn=CreateRectRgn(0,0,0,0);
            //CombineRgn(rgn,rgn,ib->rgn3,RGN_OR);
            //SetWindowRgn(ib->hwindow,rgn,false);
            //PostMessage(ib->father,WM_PAINT,0,0);
            redraw=true;
            //DeleteObject(rgn);
        }
        AlphaBlend(ib->save,ib->edgewidth,ib->edgeheight,ib->maxx-2*ib->edgewidth,ib->maxy-2*ib->edgeheight,ib->image.color,2*width,0,width,height,a);
    }
    if (!ib->autosize) {
        AlphaBlend(ib->save,0,0,ib->width,ib->height,ib->image.color,width*state,height,ib->width,ib->height,a);
        AlphaBlend(ib->save,ib->maxx-ib->width,0,ib->width,ib->height,ib->image.color,width*(state+1)-ib->width,height,ib->width,ib->height,a);
        AlphaBlend(ib->save,0,ib->maxy-ib->height,ib->width,ib->height,ib->image.color,width*state,2*height-ib->height,ib->width,ib->height,a);
        AlphaBlend(ib->save,ib->maxx-ib->width,ib->maxy-ib->height,ib->width,ib->height,ib->image.color,width*(state+1)-ib->width,2*height-ib->height,ib->width,ib->height,a);
        AlphaBlend(ib->save,ib->width,0,ib->maxx-2*ib->width,ib->edgeheight,ib->image.color,width*state+ib->width,height,width-2*ib->width,ib->edgeheight,a);
        AlphaBlend(ib->save,ib->width,ib->maxy-ib->edgeheight,ib->maxx-2*ib->width,ib->edgeheight,ib->image.color,width*state+ib->width,2*height-ib->edgeheight,width-2*ib->width,ib->edgeheight,a);
        AlphaBlend(ib->save,0,ib->height,ib->edgewidth,ib->maxy-2*ib->height,ib->image.color,width*state,height+ib->height,ib->edgewidth,height-2*ib->height,a);
        AlphaBlend(ib->save,ib->maxx-ib->edgewidth,ib->height,ib->edgewidth,ib->maxy-2*ib->height,ib->image.color,width*(state+1)-ib->edgewidth,height+ib->height,ib->edgewidth,height-2*ib->height,a);
    }
    GetClientRect(ib->hwindow,&r);
    DrawText(ib->save,ib->text,-1,&r,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
    //BitBlt(ib->dc,0,0,ib->maxx,ib->maxy,ib->save,0,0,SRCCOPY);
    if (redraw) {
        GetWindowRect(ib->hwindow,&r);
        GetWindowRect(ib->father,&rr);
        r.left-=rr.left; r.top-=rr.top;
        r.right-=rr.left; r.bottom-=rr.top;
        //InvalidateRect(ib->hwindow,&r,false);
    }
    ib->laststate=state;
}
LRESULT CALLBACK ImabuttonProc(HWND Window,UINT AMessage,WPARAM WParam,LPARAM LParam)
{
    LPImaButton ib=NULL;
    HDC dc;
    RECT r;
    int x,y;
    POINT p;
    WINDOWPOS wp;
    if (ibsum!=0) ib=FindImaButton(Window);
    switch (AMessage) {
        case IMB_GETSTATE:
            if (ib!=NULL) return ib->state;
            break;
        case WM_TIMER:
            if (ib!=NULL) display(ib,ib->state);
            break;
        case WM_PAINT:
            if (ib!=NULL) {
                display(ib,ib->state);
                BitBlt((HDC)WParam,ib->x,ib->y,ib->maxx,ib->maxy,ib->save,0,0,SRCCOPY);
            }
            break;
        case WM_WINDOWPOSCHANGED:
            wp=*(WINDOWPOS*)LParam;
            if (ib!=NULL) {
                ib->x=wp.x; ib->y=wp.y;
                ib->maxx=wp.cx; ib->maxy=wp.cy;
                /*DeleteObject(ib->rgn1);
                DeleteObject(ib->rgn2);
                DeleteObject(ib->rgn3);
                ib->rgn1=CreateRectRgn(0,0,ib->maxx,ib->maxy);
                ib->rgn2=CreateRectRgn(0,0,ib->maxx,ib->maxy);
                ib->rgn3=CreateRectRgn(0,0,ib->maxx,ib->maxy);*/
                //display(ib,ib->state);
            }
            break;
        case WM_MOUSEMOVE:
            if (ib!=NULL)
                if (WParam==100000000) {
                    x=GET_X_LPARAM(LParam); y=GET_Y_LPARAM(LParam);
                    GetWindowRect(ib->hwindow,&r);
                    x-=r.left; y-=r.top;
                    if (x>=0&&y>=0&&x<=ib->maxx&&y<=ib->maxy) {
                        if (!ib->havepressed) {
                            if (ib->state!=1) ib->state=1;
                        } else {
                            if (ib->state!=2) {
                                ib->state=2; ib->havepressed=true;
                            }
                        }
                    } else {
                        if (ib->state!=0) {
                            ib->state=0;
                        }
                    }
                    return 0;
                }
            break;
        case WM_LBUTTONDOWN:
            if (ib!=NULL)
                if (WParam==100000000) {
                    x=LOWORD(LParam); y=HIWORD(LParam);
                    GetWindowRect(ib->hwindow,&r);
                    x-=r.left; y-=r.top;
                    if (x>=0&&y>=0&&x<=ib->maxx&&y<=ib->maxy) {
                        ib->state=2; ib->havepressed=true;
                        GetClientRect(Window,&r);
                    } else {
                        ib->state=0;
                    }
                    return 0;
                }
            break;
        case WM_LBUTTONUP:
            if (ib!=NULL)
                if (WParam==100000000) {
                    x=LOWORD(LParam); y=HIWORD(LParam);
                    p.x=x; p.y=y;
                    GetWindowRect(ib->hwindow,&r);
                    x-=r.left; y-=r.top;
                    if (x>=0&&y>=0&&x<=ib->maxx&&y<=ib->maxy) {
                        ib->state=1;
                        if (ib->havepressed&&WindowFromPoint(p)==ib->hwindow) {
                            PostMessage(ib->father,WM_COMMAND,ib->num,0);
                        }
                        ib->havepressed=false;
                    } else {
                        ib->state=0; ib->havepressed=false;
                    }
                    return 0;
                }
            break;
    }
    return DefWindowProc(Window, AMessage, WParam, LParam);
}
bool ImaButtonRegister()
{
    WNDCLASS WindowClass;
    WindowClass.style=0;
    WindowClass.lpfnWndProc=&ImabuttonProc;
    WindowClass.cbClsExtra=0;
    WindowClass.cbWndExtra=0;
    WindowClass.hInstance=Hinstance;
    WindowClass.hIcon=(HICON)LoadIcon(0, IDI_APPLICATION);
    WindowClass.hCursor=LoadCursor(0,IDC_ARROW);
    WindowClass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
    WindowClass.lpszMenuName=NULL;
    WindowClass.lpszClassName=ImaButtonName;
    return RegisterClass(&WindowClass)!=0;
}
int Useful_Init()
{
    RECT rc;
    rc.top=0; rc.bottom=800; rc.left=0; rc.right=800;
    LoadLibrary("gdiplus.dll");
    LoadLibrary("SHCore.dll");
    HMODULE hModule=GetModuleHandle("gdiplus.dll");
    HMODULE hModule2=GetModuleHandle("SHCore.dll");
    ImaButtonRegister();
    ibsum=0;
    SetWindowsHookEx(WH_MOUSE_LL,&MouseProc,Hinstance,0);
    if (hModule2!=NULL) {
        SetProcessDpiAwareness=(pSetProcessDpiAwareness)GetProcAddress(hModule2, "SetProcessDpiAwareness");
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
        GetDpiForMonitor =(pGetDpiForMonitor)GetProcAddress(hModule2, "GetDpiForMonitor");
        GetDpiForMonitor(MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST),MDT_DEFAULT,&DPI_x,&DPI_y);
    }
    vStartupInput.DebugEventCallback = NULL;
    vStartupInput.SuppressBackgroundThread = false;
    vStartupInput.SuppressExternalCodecs   = false;
    vStartupInput.GdiplusVersion = 1;
    GdiplusStartup(&vToken, &vStartupInput, NULL);
    GdipCreateHBITMAPFromBitmap = (pGdipCreateHBITMAPFromBitmap)GetProcAddress(hModule, "GdipCreateHBITMAPFromBitmap");
    if(GdipCreateHBITMAPFromBitmap == NULL)
    {
        return 1;
    }
    GdipCreateBitmapFromFile = (pGdipCreateBitmapFromFile)GetProcAddress(hModule, "GdipCreateBitmapFromFile");
    if(GdipCreateBitmapFromFile == NULL)
    {
        return 2;
    }
    GdipDisposeImage = (pGdipDisposeImage)GetProcAddress(hModule,"GdipDisposeImage");
    if (GdipDisposeImage == NULL)
    {
        return 3;
    }
    return 0;
}
void Useful_Final()
{
    Gdiplus::GdiplusShutdown(vToken);
}
