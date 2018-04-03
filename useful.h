#ifndef _USEFUL_H_
#define _USEFUL_H_

#include <windows.h>
#include <stdbool.h>
#include <math.h>
typedef struct {
    int height,width;
    HDC color;
    HBITMAP colorbmp;
} tbmp;

typedef struct {
    char imagepath[256];
    tbmp image;
    HWND hwindow,father;
    HDC dc,save;
    HFONT font;
    char text[256];
    int minx,miny,maxx,maxy,state,num,x,y;
    int edgewidth,edgeheight,width,height,laststate,noww,nowh;
    bool havepressed,autosize;
    HRGN rgn1,rgn2,rgn3;
} ImaButton;
typedef ImaButton *LPImaButton;

#define ImaButtonName "Imabutton"
const int IBS_AUTOSIZE=1;
const int IMB_GETSTATE=1;
const int ITB_GETPOS=100000000;
const int ITB_POSCHANGING=10000001;
const int ITB_POSCHANGED=10000002;
const int ITM_ADDITEM=10000;
const int ITM_INSERTITEM=999;


inline int mmin(int x1,int x2);
inline int mmax(int x1,int x2);
char *IntToStr(int x);
int StrToInt(char *s);
char *RealToStr(double x);
double StrToReal(char *s);
int Gcd(int n,int m);
double DoubleMod(double x1,double x2);
int pos(char c,char *s);
int pos(char *c,char *s);
void Bar(HDC dc,HDC dc2,int x1,int y1,int x2,int y2,COLORREF color);
void makefont(int height,HFONT *thefont);
void MakeFont(char *name,int height,HFONT *thefont);
void loadpic(char *path,tbmp *m);
void CreateImaButton(ImaButton *item,char *name,HWND father,DWORD Style,int x,int y,int width,int height,int num,char *imapath);
int Useful_Init();
void Useful_Final();

#endif
