#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <stdbool.h>
#include <tchar.h>
#include "window.h"
#include "useful.h"
#include "sound.h"
#include "AI.h"
#include "Blood.h"
#include "smog.h"
#include "save.h"
#include <math.h>
#include <time.h>
const char* AppName="丧尸之城";
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
HDC windc,bitmapdc,tempdc,tempdc2,tempdc3,red,black,save;
HBITMAP savescreen,savedscreen;
HFONT thefont;
BMP formmark;
BMP formzombie[19];
int bullsum,grasum,weasum,hweasum,zombieleft,zombiesum,killzombie,wave;
int minx,minaim,mousex,mousey,maxx,maxy,weanow,boomsum,zhennow,zhenhan,turnred,turnblack;
tbmp sky[6];
tbmp backcity[4];
tbmp building[4];
tbmp bopic[17];
tbmp smogpic;
tbmp weatest;
boomnode boom[100];
weanode weapon[30],hweapon[30];
tbmp form[4];
tbmp zombies[19];
tbmp menu,box;
bullnode bull[211];
granode gr[211];
zombie zom[211];
bloodnode blood[5000];
smognode smog[100];
tbmp man,bul,lie;
people mann;
girlnode girl;
LARGE_INTEGER frequent;
HWND hWindow;
people tman;
bool printnow,loadfromfile=true;
int head,tail,nowx,nowy,i,minwho,money,BoxCommand;
bool bo[5];
bool bfloor,found,nextfire;
double s,t,mintime;
COLORREF lastcolor;
listnode list[10000];
bool boo[1073][5];
double angle,x,y,v,v0,v1,a,v11,tt;
//int playnow;
//bool play[4];
//char path[4][256],oldpath[4][256];
HANDLE Handle1,Handle2,Handle3,Handle7,Handle8;
char temp[256];
HINSTANCE Hinstance;
ImaButton button1,button2,button3,button4,button5,button6,button7;
bool pause,quit,mainmenu,shop,Box,dead;
LARGE_INTEGER lasttime,nowtime;

//shop
int choosenow;
ImaButton bprevious,bnext,bbuy,bcontinue,bexit;
void fire()
{
    POINT p;
    if (hweapon[weanow].now>0) {
        mann.fire=true;
        hweapon[weanow].now--;
        if (hweapon[weanow].lei==Gun) {
            if (bullsum<200) bullsum++; else bullsum=1;
            bull[bullsum].jiao=mann.jiao+mann.up;
            bull[bullsum].sx=mann.x+8; bull[bullsum].sy=mann.y+13;
            bull[bullsum].length=30; bull[bullsum].flag=true;
            bull[bullsum].damage=hweapon[weanow].damage;
            PlaySound(hweapon[weanow].shoot);
            p=getlinepixel(bull[bullsum].sx,bull[bullsum].sy,30,bull[bullsum].jiao);
            PutSmog(p.x,p.y,bull[bullsum].jiao,2,1);
        } else if (hweapon[weanow].lei==Gre) {
            if (grasum<200)  grasum++; else grasum=1;
            if (mann.toward) {
                if ((mann.jiao+mann.up)<0)  gr[grasum].jiao=(mann.jiao+mann.up)+90; else gr[grasum].jiao=180-(mann.jiao+mann.up)+90;
            } else {
                if (abs((mann.jiao+mann.up))<180)  gr[grasum].jiao=90+(mann.jiao+mann.up); else gr[grasum].jiao=270-(mann.jiao+mann.up);
            };
            gr[grasum].jiao=(mann.jiao+mann.up);
            gr[grasum].nowx=mann.x+8; gr[grasum].nowy=mann.y+13;
            gr[grasum].vy=sin(gr[grasum].jiao*PI)*15;
            //writeln(gr[grasum].v1:0:2);
            gr[grasum].vx=cos(gr[grasum].jiao*PI)*15;
            gr[grasum].t=0.1;
            gr[grasum].flag=true; gr[grasum].boom=true;
            gr[grasum].damage=hweapon[weanow].damage;
            PlaySound("music\\gre1.wav");
            p=getlinepixel(gr[grasum].nowx,gr[grasum].nowy,30,gr[grasum].jiao);
            PutSmog(p.x,p.y,gr[grasum].jiao,4,rand()%5+4);
        };
        zhennow=1; zhenhan=2;
    };
    mann.showangle=360*(double)hweapon[weanow].now/(double)hweapon[weanow].danjia;
    if (mann.toward) {
        mann.up=mann.up+mann.canup/10;
        mann.canup=hweapon[weanow].up-mann.up;
    } else {
        mann.up=mann.up+mann.canup/10;
        mann.canup=-hweapon[weanow].up-mann.up;
    };
    if (hweapon[weanow].now<=0) {
        mann.fire=false;
        if (!mann.reload) {
            SetTimer(hWindow,3,50,NULL); mann.reload=true;
            hweapon[weanow].reltime=hweapon[weanow].reloadtime;
            mann.show=0;
        };
    };
}
void setmin(int now)
{
   minx=now;
   if (minx<0) minx=0;
   if (minx>1070-640) minx=1070-640;
}
void DisplayShop()
{
    shop=true; pause=true;
    ShowWindow(bprevious.hwindow,SW_SHOW);
    ShowWindow(bnext.hwindow,SW_SHOW);
    ShowWindow(bbuy.hwindow,SW_SHOW);
    ShowWindow(bcontinue.hwindow,SW_SHOW);
    ShowWindow(bexit.hwindow,SW_SHOW);
    choosenow=1;
}
void UnDisplayShop()
{
    shop=false; pause=false;
    ShowWindow(bprevious.hwindow,SW_HIDE);
    ShowWindow(bnext.hwindow,SW_HIDE);
    ShowWindow(bbuy.hwindow,SW_HIDE);
    ShowWindow(bcontinue.hwindow,SW_HIDE);
    ShowWindow(bexit.hwindow,SW_HIDE);
}
void StartNewGame()
{
    const int zombieset[30]={0,10,20,30,50,55,60,70,80,90,100};
    mann.y=480-48-32; mann.x=320-17+200;
    mann.toward=true;
    mann.now=Nothing;
    mann.show=1;
    mann.hp=100;
    mann.jiao=0;
    girl.now=LieOnTheBox;
    girl.x=422;
    girl.y=253;
    if (wave==0) {
        hweasum=1;
        hweapon[1]=weapon[1];
        hweapon[1].left=1500; hweapon[1].now=15;
    }
    /*hweapon[2].left=1500; hweapon[2].now=8;
    hweapon[3].left=1500; hweapon[3].now=30;
    hweapon[4].left=200;hweapon[4].now=1; hweapon[4].danjia=10;
    hweapon[5].left=200;hweapon[5].now=1;
    hweapon[6].left=1500;hweapon[6].now=30;
    hweapon[7].left=1500; hweapon[7].now=100;*/
    for (i=1; i<=100; i++) boom[i].now=16;
    weanow=1; boomsum=0; zhennow=0;
    minx=200; minaim=minx;
    zombiesum=zombieset[++wave]; zombieleft=zombiesum; killzombie=0;
    pause=false; mainmenu=false; loadfromfile=false; shop=false; Box=false;
    quit=false;
    QueryPerformanceCounter(&mann.lasttime);
    QueryPerformanceCounter(&lasttime);
    for (i=1; i<=200; i++)
        if (zom[i].flag) zom[i].flag=false;
    for (i=0; i<5000; i++)
        if (blood[i].flag) blood[i].flag=false;
    for (i=0; i<100; i++)
        if (smog[i].flag) smog[i].flag=false;
    for (i=1; i<=hweasum; i++)
        if (hweapon[i].left>=hweapon[i].danjia-hweapon[i].now) {
            hweapon[i].left-=hweapon[i].danjia-hweapon[i].now;
            hweapon[i].now=hweapon[i].danjia;
        } else {
            hweapon[i].now+=hweapon[i].left;
            hweapon[i].left=0;
        }
    strcpy(button2.text,"继续");
    strcpy(button5.text,"返回主菜单");
    strcpy(button6.text,"快速存档");
    strcpy(button7.text,"退出游戏");
    SetTimer(hWindow,6,10,NULL);
    SetTimer(hWindow,4,700,NULL);
}
void ContinueGame()
{
    if (loadfromfile) {
        ReadTheGame();
        loadfromfile=false;
    }
    QueryPerformanceCounter(&lasttime);
    pause=false; mainmenu=false;
    QueryPerformanceCounter(&mann.lasttime);
    QueryPerformanceCounter(&lasttime);
    for (i=1; i<=200; i++)
        if (zom[i].flag) QueryPerformanceCounter(&zom[i].lasttime);
    for (i=0; i<5000; i++)
        if (blood[i].flag) QueryPerformanceCounter(&blood[i].lasttime);
    for (i=0; i<100; i++)
        if (smog[i].flag) QueryPerformanceCounter(&smog[i].lasttime);
    strcpy(button2.text,"继续");
    strcpy(button5.text,"返回主菜单");
    strcpy(button6.text,"快速存档");
    strcpy(button7.text,"退出游戏");
    if (shop) DisplayShop();
    SetTimer(hWindow,4,700,NULL);
}
void Dead()
{
    dead=true; pause=true;
    strcpy(button2.text,"返回存档点");
    strcpy(button5.text,"返回主菜单");
    strcpy(button6.text,"新游戏");
    strcpy(button7.text,"退出游戏");
}
void BackMenu()
{
    pause=true; mainmenu=true;
    strcpy(button2.text,"新游戏");
    strcpy(button5.text,"继续游戏");
    strcpy(button6.text,"设置");
    strcpy(button7.text,"退出游戏");
}
void ShowBox(char *title,char *content,int command)
{
    RECT r;
    HFONT font;
    BitBlt(save,0,0,640,480,bitmapdc,0,0,SRCCOPY);
    loadpic("image\\box.png",&box);
    r.top=3; r.left=3;
    r.bottom=16; r.right=168;
    SetTextColor(box.color,RGB(255,255,255));
    SetBkMode(box.color,1);
    MakeFont("幼圆",10,&font);
    SelectObject(box.color,font);
    DrawText(box.color,title,-1,&r,DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    DeleteObject(font);
    r.top=20; r.left=3;
    r.bottom=41; r.right=167;
    MakeFont("幼圆",14,&font);
    SelectObject(box.color,font);
    DrawText(box.color,content,-1,&r,DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    Box=true; BoxCommand=command;
}
void HideBox()
{
    Box=false;
}
void BuyWeapon()
{
    bool flag;
    int i;
    if (mann.money>=weapon[choosenow].price) {
        bool flag=false;
        for (i=1; i<=hweasum; i++)
            if (!strcmp(hweapon[i].name,weapon[choosenow].name)) {
                flag=true; break;
            }
        if (flag) {
            ShowBox("提示","你已经拥有了这个武器",103);
        } else {
            hweapon[++hweasum]=weapon[choosenow];
            hweapon[hweasum].left=5*hweapon[hweasum].danjia;
            hweapon[hweasum].now=hweapon[hweasum].danjia;
            mann.money-=weapon[choosenow].price;
            ShowBox("提示","成功购买",103);
        }
    } else {
        ShowBox("提示","你没有足够金钱购买！",104);
    }
}
void QueryQuit()
{
    quit=true;
    ShowBox("询问","您确定要离开吗？",101);
    ShowWindow(button3.hwindow,SW_SHOW);
    ShowWindow(button4.hwindow,SW_SHOW);
}
void NotQuit()
{
    quit=false;
    HideBox();
    QueryPerformanceCounter(&mann.lasttime);
    QueryPerformanceCounter(&lasttime);
    for (i=1; i<=200; i++)
        if (zom[i].flag) QueryPerformanceCounter(&zom[i].lasttime);
    for (i=0; i<5000; i++)
        if (blood[i].flag) QueryPerformanceCounter(&blood[i].lasttime);
    for (i=0; i<100; i++)
        if (smog[i].flag) QueryPerformanceCounter(&smog[i].lasttime);
}
void Pause()
{
    int i;
    if (!pause&&!quit) {
        pause=true;
        //SetWindowPos(button1.hwindow,0,320-50,100,100,30,SWP_NOZORDER);
        ShowWindow(button1.hwindow,SW_HIDE);
    } else {
        pause=false;
        ShowWindow(button1.hwindow,SW_SHOW);
        //strcpy(button1.text,"暂停");
        //SetWindowPos(button1.hwindow,0,10,10,50,20,SWP_NOZORDER);
        QueryPerformanceCounter(&mann.lasttime);
        QueryPerformanceCounter(&lasttime);
        for (i=1; i<=200; i++)
            if (zom[i].flag) QueryPerformanceCounter(&zom[i].lasttime);
        for (i=0; i<5000; i++)
            if (blood[i].flag) QueryPerformanceCounter(&blood[i].lasttime);
        for (i=0; i<100; i++)
            if (smog[i].flag) QueryPerformanceCounter(&smog[i].lasttime);
    }
}
WPARAM press;
LRESULT CALLBACK WindowProc(HWND Window,UINT AMessage,WPARAM WParam,LPARAM LParam)
{
    HDC dc;
    PAINTSTRUCT ps;
    RECT r;
    bool bo,bk;
    double jiao,deltat;
    int now,nowx,nowy,i,j,k,x;
    listnode nextact;
    char temps[50];
    switch (AMessage) {
        case WM_PAINT:
            //BitBlt(windc,0,0,640,480,bitmapdc,0,0,SRCCOPY);
            printnow=true;
            break;
        case WM_KEYDOWN:
            if (WParam!=press)  {
                switch (WParam) {
                    case 'a':
                    case 'A':
                        if (!pause&&!quit) {
                        mann.now=Walk;
                        mann.walktoward=false;
                        }
                        break;
                    case 'd':
                    case 'D':
                        if (!pause&&!quit) {
                        mann.now=Walk;
                        mann.walktoward=true;
                        }
                        break;
                    case 's':
                    case 'S':
                        if (!pause&&!quit) {
                        if (GetPixelAtBmp(&formmark,mann.x+8,mann.y+16)==RGB(255,0,0))  {
                            if (mann.y>373)  mann.y=320; else mann.y=400;
                        } else if (GetPixelAtBmp(&formmark,mann.x+8,mann.y+16)==RGB(0,255,0))  {
                            if (mann.y>293)  mann.y=240; else mann.y=320;
                        } else if (GetPixelAtBmp(&formmark,mann.x+8,mann.y+16)==RGB(0,0,255))  {
                            if (mann.y>213)  mann.y=160; else mann.y=240;
                        };
                        }
                        break;
                    case ' ':
                        if (!pause&&!quit) {
                        if (!mann.drop)  {
                            mann.drop=true;
                            mann.v1=6; mann.t=0;
                            mann.nowx=mann.x; mann.nowy=mann.y-3;
                            QueryPerformanceCounter(&mann.lasttime);
                        };
                        }
                        break;
                    case 'q':
                    case 'Q':
                        if (!pause&&!quit) {
                        nextfire=true;
                        mann.fire=false;
                        KillTimer(hWindow,2);
                        if (weanow>1)  weanow--; else weanow=hweasum;
                        mann.show=1; mann.reload=false; KillTimer(hWindow,3);
                        }
                        break;
                    case 'e':
                    case 'E':
                        if (!pause&&!quit) {
                        nextfire=true;
                        mann.fire=false;
                        KillTimer(hWindow,2);
                        if (weanow<hweasum)  weanow++; else weanow=1;
                        mann.show=1; mann.reload=false; KillTimer(hWindow,3);
                        }
                        break;
                    case 'r':
                    case 'R':
                        mann.fire=false;
                        if (!mann.reload) {
                            SetTimer(hWindow,3,50,NULL); mann.reload=true;
                            hweapon[weanow].reltime=hweapon[weanow].reloadtime;
                            mann.show=0;
                        };
                        break;
                    case VK_RIGHT:
                        if (!pause&&!quit) {
                        if (minx<1070-640) minx+=5;
                        }
                        break;
                    case VK_LEFT:
                        if (!pause&&!quit) {
                        if (minx>0) minx-=5;
                        }
                        break;
                    case VK_ESCAPE:
                        if (!mainmenu) Pause();
                        break;
                };
                press=WParam;
                printnow=true;
            };
            break;
        case WM_KEYUP:
            switch (WParam) {
                case 'a':
                case 'A':
                    if (!pause&&!quit) {
                    mann.now=Nothing;
                    mann.walktoward=false;
                    mann.walknow=0;
                    }
                    break;
                case 'd':
                case 'D':
                    if (!pause&&!quit) {
                    mann.now=Nothing;
                    mann.walktoward=true;
                    mann.walknow=0;
                    }
                    break;
                case VK_ESCAPE:

                    break;
            }
            press=0;
            printnow=true;
            //print();
            break;
        case WM_COMMAND:
            if (BoxCommand==101) {
                if (WParam==201) {
                    NotQuit();
                } else if (WParam==202) {
                    SaveTheGame();
                    SendMessage(hWindow,WM_DESTROY,0,0);
                }
            } else if (BoxCommand==103) {
                if (WParam==201) {
                    HideBox();
                } else if (WParam==202) {
                    HideBox();
                }
            } else if (BoxCommand==104) {
                if (WParam==201) {
                    HideBox();
                } else if (WParam==202) {
                    HideBox();
                }
            }
            if (mainmenu) {
                if (WParam==101) {
                    StartNewGame();
                    SaveTheGame();
                } else if (WParam==102) {
                    ContinueGame();
                } else if (WParam==103) {

                } else if (WParam==104) {
                    QueryQuit();
                }
            } else {
                if (dead) {
                    if (WParam==101) {
                        ReadTheGame();
                        ContinueGame();
                        dead=false;
                    } else if (WParam==102) {
                        dead=false;
                        BackMenu();
                    } else if (WParam==103) {
                        wave=0; dead=false;
                        StartNewGame();
                    } else if (WParam==104) {
                        QueryQuit();
                    }
                } else {
                    if (WParam==101) Pause();
                    else if (WParam==102) {
                        BackMenu();
                    } else if (WParam==103) {
                        SaveTheGame();
                        Pause();
                    } else if (WParam==104) {
                        QueryQuit();
                    }
                }
            }
            if (WParam==301) {
                if (choosenow>1) choosenow--; else choosenow=weasum;
            } else if (WParam==302) {
                if (choosenow<weasum) choosenow++; else choosenow=1;
            } else if (WParam==303) {
                BuyWeapon();
            } else if (WParam==304) {
                UnDisplayShop();
                StartNewGame();
            } else if (WParam==305) {
                if (killzombie>=zombiesum&&zombiesum>0) StartNewGame();
                shop=true;
                SaveTheGame();
                shop=false;
                UnDisplayShop();
                BackMenu();
                SetTimer(hWindow,6,10,NULL);
                loadfromfile=true;
            }
            break;
        case WM_TIMER:
            if (WParam==1) {
                //僵尸AI 动作处理
                if (killzombie>=zombiesum&&zombiesum>0) {
                    SetTimer(hWindow,8,500,NULL);
                    killzombie=0;
                }
                if (turnred>0) turnred--;
                if (!pause&&!quit)
                for (x=1; x<=200; x++)
                    if ((zom[x].flag)&&(zom[x].hp>0))  {
                        if (GetTarget(zom[x].x,zom[x].y)!=Nothing) {
                            if (GetTarget(zom[x].x,zom[x].y)==TheMan) {
                                if (mann.hp>0) mann.hp-=0.04;
                                if (turnred<128) turnred+=1;
                                if (mann.hp<=0) Dead();
                                if (zom[x].x<mann.x) zom[x].walktoward=true;
                                else zom[x].walktoward=false;
                            } else if (GetTarget(zom[x].x,zom[x].y)==TheGirl) {
                                if (zom[x].x<girl.x) zom[x].walktoward=true;
                                else zom[x].walktoward=false;
                            }
                            if (zom[x].walknow<24) zom[x].walknow=24;
                            else zom[x].walknow=(zom[x].walknow+1)>27?24:zom[x].walknow+1;
                        }
                        if (((zom[x].next.act==ACT_DROP)&&(zom[x].acttime<=0))||(zom[x].next.act!=ACT_DROP))  {
                            nextact=BFS(zom[x].x,GetFloorPixel(GetFloor(zom[x].y)));
                        };
                        if (zom[x].next.act==ACT_NOTHING) zom[x].now=Nothing;
                        if (((zom[x].next.act==ACT_DROP)&&(zom[x].acttime>0))||((nextact.act==ACT_DROP)&&(zom[x].next.act!=ACT_DROP)))  {
                            if ((nextact.act==ACT_DROP&&zom[x].next.act!=ACT_DROP))  {
                                zom[x].next=nextact;
                                zom[x].sumacttime=nextact.t; zom[x].acttime=zom[x].sumacttime;
                            };
                            if (zom[x].acttime<=zom[x].sumacttime*0.55&&zom[x].acttime>=0)  {
                                //zom[x].sumacttime=0;
                                if (zom[x].nowx>mann.x)  {
                                    nowx=zom[x].nowx;
                                    nowx-=1;  if (nowx<0) nowx=0;
                                    bo=true;
                                    for (j=1; j<=31; j++)
                                        if (GetPixelAtBmp(&formmark,nowx-1,zom[x].y+j)==RGB(255,255,0))  {
                                            bo=false; break;
                                        };
                                    if (bo)  {zom[x].x=nowx; zom[x].nowx=nowx;};
                                    zom[x].walktoward=false;
                                } else if (zom[x].nowx<mann.x)  {
                                    nowx=zom[x].nowx;
                                    nowx+=1; if (nowx>1072-17) nowx=1072-17;
                                    bo=true;
                                    for (i=1; i<=31; i++)
                                        if (GetPixelAtBmp(&formmark,nowx+18,zom[x].y+i)==RGB(255,255,0))  {
                                            bo=false; break;
                                        };
                                    if (bo)  { zom[x].x=nowx; zom[x].nowx=nowx; };
                                    zom[x].walktoward=true;
                                };
                            } else {
                                if (zom[x].walktoward)  {
                                    nowx=zom[x].nowx;
                                    nowx+=1; if (nowx>1072-17) nowx=1072-17;
                                    bo=true;
                                    for (i=1; i<=31; i++)
                                        if (GetPixelAtBmp(&formmark,nowx+18,zom[x].y+i)==RGB(255,255,0))  {
                                            bo=false; break;
                                        };
                                    if (bo)  { zom[x].x=nowx; zom[x].nowx=nowx; };
                                } else {
                                    nowx=zom[x].nowx;
                                    nowx-=1;  if (nowx<0) nowx=0;
                                    bo=true;
                                    for (j=1; j<=31; j++)
                                        if (GetPixelAtBmp(&formmark,nowx-1,zom[x].y+j)==RGB(255,255,0))  {
                                            bo=false; break;
                                        };
                                    if (bo)  { zom[x].x=nowx; zom[x].nowx=nowx; };
                                    zom[x].walktoward=false;
                                };
                            };
                            //zom[x].acttime=zom[x].acttime-(double)(nowtime.QuadPart-lasttime.QuadPart)/frequent.QuadPart;
                        } else {
                            zom[x].next=nextact;
                            if (zom[x].next.act==ACT_LEFT)  {
                                if (zom[x].walktoward)  zom[x].walktoward=false;
                                zom[x].now=Walk; zom[x].x=zom[x].x-1;
                            } else if (zom[x].next.act==ACT_RIGHT)  {
                                if (not zom[x].walktoward)  zom[x].walktoward=true;
                                zom[x].now=Walk; zom[x].x=zom[x].x+1;
                            } else if (zom[x].next.act==ACT_STAIR)  {
                                zom[x].y=zom[x].next.y;
                            };
                        };
                        if (zom[x].now==Walk)  {
                            if (!zom[x].drop)  {
                                bo=true;
                                if (zom[x].hp>0)  {
                                    for (i=5; i<=12; i++)
                                        if (GetPixelAtBmp(&formmark,zom[x].x+i,zom[x].y+34)==RGB(255,255,0))  {
                                      bo=false; break;
                                   };
                                } else {
                                    for (i=1; i<=32; i++)
                                        if (GetPixelAtBmp(&formmark,zom[x].x+i,zom[x].y+18)==RGB(255,255,0))  {
                                            bo=false; break;
                                        };
                                };
                                if (bo)  {
                                    zom[x].drop=true;
                                    zom[x].vx=0; zom[x].t=0;
                                    zom[x].nowx=zom[x].x; zom[x].nowy=zom[x].y; QueryPerformanceCounter(&zom[x].lasttime);
                                };
                            };
                            zom[x].walknow=(zom[x].walknow+1)%24;
                        };
                        QueryPerformanceCounter(&zom[x].nowtime);
                        if (zom[x].drop) zom[x].t=zom[x].t+(double)(zom[x].nowtime.QuadPart-zom[x].lasttime.QuadPart)/frequent.QuadPart;
                        zom[x].acttime=zom[x].acttime-(double)(zom[x].nowtime.QuadPart-zom[x].lasttime.QuadPart)/frequent.QuadPart;
                        zom[x].lasttime=zom[x].nowtime;
                        if (zom[x].drop)  {
                            //zom[x].x=zom[x].nowx+round(zom[x].v0*t);
                            nowy=zom[x].nowy-round((zom[x].vy*zom[x].t-4.9*zom[x].t*zom[x].t)*100/5);
                            nowx=zom[x].nowx+round((zom[x].vx*zom[x].t)*100/5);
                            if (nowx<0) nowx=0;
                            if (nowx>1072-28) nowx=1072-28;
                            bo=true;
                            for (i=5; i<=12; i++)
                                if (GetPixelAtBmp(&formmark,nowx+i,nowy)==RGB(255,255,0)||GetPixelAtBmp(&formmark,nowx+i,nowy+34)==RGB(255,255,0))  {
                                bo=false; break;
                             };
                            if (!bo)  {
                                //writeln(nowy,' ',zom[x].zhuang,' ',GetPixelAtBmp(&formmark,zom[x].x+i,nowy)=RGB(255,255,0),' ',GetPixelAtBmp(&formmark,zom[x].x+i,nowy+34)=RGB(255,255,0));
                                if ((GetPixelAtBmp(&formmark,nowx+i,nowy)==RGB(255,255,0))&&(!zom[x].zhuang))  {
                                    zom[x].drop=true; zom[x].zhuang=true;//撞头顶
                                    zom[x].vy=0; zom[x].t=0; QueryPerformanceCounter(&zom[x].lasttime);
                                    zom[x].nowx=zom[x].x; zom[x].nowy=zom[x].y;
                                } else if (GetPixelAtBmp(&formmark,nowx+i,nowy+34)==RGB(255,255,0))  {
                                    zom[x].drop=false; zom[x].zhuang=false; zom[x].t=0; zom[x].acttime=-1;//落地
                                    zom[x].y=GetFloorPixel(GetFloor(zom[x].y)); zom[x].vy=0;
                                } else bo=true;
                            };
                            bk=true;
                            for (i=0; i<=31; i++)
                                if (GetPixelAtBmp(&formmark,nowx+12,nowy+i)==RGB(255,255,0)||GetPixelAtBmp(&formmark,nowx+5,nowy+i)==RGB(255,255,0))  {
                                    bk=false; break;
                                };
                            if (!bk)  {
                                zom[x].drop=true; zom[x].zhuang=true;//撞墙
                                //zom[x].vy=0;
                                zom[x].t=0; zom[x].vx=0;
                                zom[x].nowx=zom[x].x; zom[x].nowy=zom[x].y;
                            };
                            if (bo)  { zom[x].y=nowy; };
                          if (bk)  { zom[x].x=nowx; };
                            //print;
                        };
                        QueryPerformanceCounter(&zom[x].lasttime);
                    } else if (zom[x].flag&&zom[x].hp<=0)  {
                        zom[x].showtime--;
                        if (zom[x].showtime<0) {
                            zom[x].trans--; if (zom[x].trans<=0) zom[x].flag=false;
                        };
                        if (!zom[x].drop)  {
                            bo=true;
                            for (i=1; i<=32; i++)
                                if (GetPixelAtBmp(&formmark,zom[x].x+i,zom[x].y+18)==RGB(255,255,0))  {
                            bo=false; break;
                         };
                         if (bo)  {
                            zom[x].drop=true;
                            zom[x].vy=0; zom[x].t=0;
                            zom[x].nowx=zom[x].x; zom[x].nowy=zom[x].y;
                         };
                      };
                      if (zom[x].drop)  {
                            QueryPerformanceCounter(&zom[x].nowtime);
                            zom[x].t=zom[x].t+(double)(zom[x].nowtime.QuadPart-zom[x].lasttime.QuadPart)/frequent.QuadPart;
                            zom[x].lasttime=zom[x].nowtime;
                          //zom[x].x=zom[x].nowx+round(zom[x].v0*t);
                          nowy=zom[x].nowy-round((zom[x].vy*zom[x].t-4.9*zom[x].t*zom[x].t)*100/5);
                          nowx=zom[x].nowx+round(zom[x].vx*zom[x].t);
                          bo=true;
                          for (i=0; i<=31; i++)
                             if (GetPixelAtBmp(&formmark,nowx+i,nowy)==RGB(255,255,0)||GetPixelAtBmp(&formmark,nowx+i,nowy+18)==RGB(255,255,0))  {
                                bo=false; break;
                             };
                          if (!bo)  {
                             //writeln(nowy,' ',zom[x].zhuang,' ',GetPixelAtBmp(&formmark,zom[x].x+i,nowy)=RGB(255,255,0),' ',GetPixelAtBmp(&formmark,zom[x].x+i,nowy+34)=RGB(255,255,0));
                             if ((GetPixelAtBmp(&formmark,nowx+i,nowy)==RGB(255,255,0))&&(!zom[x].zhuang))  {
                                zom[x].drop=true; zom[x].zhuang=true;
                                zom[x].vy=0; zom[x].t=0;
                                zom[x].nowx=zom[x].x; zom[x].nowy=zom[x].y;
                             } else if (GetPixelAtBmp(&formmark,nowx+i,nowy+18)==RGB(255,255,0))  {
                                zom[x].drop=false; zom[x].zhuang=false; zom[x].t=0;
                                if (zom[x].y<=160+32-18)  zom[x].y=160+32-18;
                                if (zom[x].y<=240+32-18&&zom[x].y>160+32-18)  zom[x].y=240+32-18;
                                if (zom[x].y<=320+32-18&&zom[x].y>240+32-18)  zom[x].y=320+32-18;
                                if (zom[x].y<=400+32-18&&zom[x].y>320+32-18)  zom[x].y=400+32-18;
                                if (zom[x].y>400+32-18)  zom[x].y=400+32-18;
                             } else bo=true;
                          };
                          bk=true;
                          for (i=0; i<=17; i++)
                             if (GetPixelAtBmp(&formmark,nowx+33,nowy+i)==RGB(255,255,0)||GetPixelAtBmp(&formmark,nowx-1,nowy+i)==RGB(255,255,0))  {
                                bk=false; break;
                             };
                          //writeln(GetPixelAtBmp(&formmark,nowx+i,nowy)=RGB(255,255,0),' ',zom[x].y,' ',nowy,' ',zom[x].t:0:2);
                          if (!bk)  {
                             if (!zom[x].drop)  zom[x].t=0;
                             zom[x].drop=true; zom[x].zhuang=true;
                             zom[x].vy=0; zom[x].vx=0;
                             zom[x].nowx=zom[x].x; zom[x].nowy=zom[x].y;
                          };
                          if (bo)  { zom[x].y=nowy; };
                          if (bk)  { zom[x].x=nowx; };
                          //print;
                        };
                    };
            } else if (WParam==2)  {
                if (!pause&&!quit) {
                if (hweapon[weanow].running) {
                    if (mann.fire)  fire();
                } else {
                    nextfire=true;
                    mann.fire=false;
                    KillTimer(hWindow,2);
                }
                }
            } else if (WParam==3)  {
                if (!pause&&!quit) {
                hweapon[weanow].reltime--;
                for (i=0; i<5; i++)
                    if ((hweapon[weanow].reloadtime-hweapon[weanow].reltime)==hweapon[weanow].play[i]) {
                        PlaySound(hweapon[weanow].sound[i]);
                    }
                mann.showangle=360*(double)hweapon[weanow].reltime/(double)hweapon[weanow].reloadtime;
                if (mann.showcolor==RGB(255,0,0))  mann.showcolor=RGB(255,255,255); else mann.showcolor=RGB(255,0,0);
                if (hweapon[weanow].left>0&&hweapon[weanow].reltime<=0)  {
                    if (hweapon[weanow].left>=hweapon[weanow].danjia-hweapon[weanow].now) {
                        hweapon[weanow].left-=hweapon[weanow].danjia-hweapon[weanow].now;
                        hweapon[weanow].now=hweapon[weanow].danjia;
                    } else {
                        hweapon[weanow].now+=hweapon[weanow].left;
                        hweapon[weanow].left=0;
                    }
                    mann.reload=false;
                    mann.showangle=360;
                    mann.show=1;
                    KillTimer(hWindow,3); mann.showangle=0;
                } else if (hweapon[weanow].left<=0)  {
                    KillTimer(hWindow,3); mann.showangle=0; mann.reload=false;
                };
                }
            } else if (WParam==4)  {
                if (!pause&&!quit) {
                zombieleft--;
                if (zombieleft<=0) KillTimer(hWindow,4);
                CreateZombie(); //killtimer(hwindow,4);
                }
            } else if (WParam==5) {
                if (!pause&&!quit) {
                minaim=mann.x-320+mousex-320;
                setmin(minx+round((minaim-minx)/15.0));
                if (zhennow>0)
                    if (zhennow<4) zhennow++; else zhennow=0;
                //SendMessage(hWindow,WM_MOUSEMOVE,0,MAKELONG(mousex,mousey));
                if (mann.hp>0)  {//主角的状态处理
                    if (mann.toward)  {
                        if (mann.up<-0.5)  {
                            mann.up=mann.up+0.5;
                            mann.canup=mann.canup-0.5;
                        } else { mann.up=0; mann.canup=hweapon[weanow].up; };
                    } else {
                        if (mann.up>0.5)  {
                            mann.up=mann.up-0.5;
                            mann.canup=mann.canup+0.5;
                        } else { mann.up=0; mann.canup=-hweapon[weanow].up; };
                    };
                    if (mann.now==Walk)  {
                        if (!mann.walktoward)  {//向左走路
                            nowx=mann.x;
                            nowx-=2;  if (nowx<0) nowx=0;
                            bo=true;
                            for (i=1; i<=31; i++)//判断是否碰壁
                                if (GetPixelAtBmp(&formmark,nowx-1,mann.y+i)==RGB(255,255,0))  {
                                    bo=false; break;
                                };
                            if (bo)  mann.x=nowx;
                        } else {//向右走路d
                            nowx=mann.x;
                            nowx+=2; if (nowx>1072-26)  nowx=1072-26;
                            bo=true;
                            for (i=1; i<=31; i++)//判断是否碰壁
                                if (GetPixelAtBmp(&formmark,nowx+18,mann.y+i)==RGB(255,255,0))  {
                                    bo=false; break;
                                };
                            if (bo)  mann.x=nowx;
                        };
                        if (!mann.walktoward)  mann.v0=-15; else mann.v0=15;

                        if (!mann.drop)  {
                            bo=true;
                            for (i=5; i<=12; i++)
                                if (GetPixelAtBmp(&formmark,mann.x+i,mann.y+34)==RGB(255,255,0))  {
                                    bo=false; break;
                                };
                            if (bo)  {
                                mann.drop=true;
                                mann.v1=0; mann.t=0; t=0; QueryPerformanceCounter(&mann.lasttime);
                                mann.nowx=mann.x; mann.nowy=mann.y;
                            };
                        };
                        mann.walknow=(mann.walknow+1)%24;
                    };
                    if (mann.drop)  {//坠落处理
                        QueryPerformanceCounter(&mann.nowtime);
                        mann.t=mann.t+(double)(mann.nowtime.QuadPart-mann.lasttime.QuadPart)/frequent.QuadPart;
                        mann.lasttime=mann.nowtime;
                        //mann.x=mann.nowx+round(mann.v0*t);
                        nowy=mann.nowy-round((mann.v1*mann.t-4.9*mann.t*mann.t)*100/5);
                        bo=true;
                        for (i=5; i<=12; i++)
                            if ((GetPixelAtBmp(&formmark,mann.x+i,nowy)==RGB(255,255,0))||(GetPixelAtBmp(&formmark,mann.x+i,nowy+34)==RGB(255,255,0)))  {
                                bo=false; break;
                        };
                        if (!bo)  {
                            if ((GetPixelAtBmp(&formmark,mann.x+i,nowy)==RGB(255,255,0))&&(!mann.zhuang))  {
                                mann.drop=true; mann.zhuang=true;
                                mann.v1=0; mann.t=0; QueryPerformanceCounter(&mann.lasttime);
                                mann.nowx=mann.x; mann.nowy=nowy;
                            } else if (GetPixelAtBmp(&formmark,mann.x+i,nowy+34)==RGB(255,255,0))  {
                                mann.drop=false; mann.zhuang=false; mann.t=0;
                                if (mann.y<=160)  mann.y=160;
                                if (mann.y<=240&&mann.y>160)  mann.y=240;
                                if (mann.y<=320&&mann.y>240)  mann.y=320;
                                if (mann.y<=400&&mann.y>320)  mann.y=400;
                                if (mann.y>400)  mann.y=400;
                            } else bo=true;
                        };
                        if (bo) mann.y=nowy;
                    };
                };
                if ((hweapon[weanow].now>=0)&&(mann.show==1))  {
                    mann.showcolor=RGB(255,255,255);
                    mann.showangle=360*(double)hweapon[weanow].now/(double)hweapon[weanow].danjia;
                    mann.show=1;
                };
                for (i=1; i<=200; i++)
                    if (bull[i].flag)  {
                        int temp=round(bull[i].length+15);
                        for (j=round(bull[i].length); j<=temp; j++) {
                            if (!bull[i].flag)  break;
                            bull[i].length=j;
                            bull[i].x=round(bull[i].sx+(bull[i].length)*cos(bull[i].jiao*PI));
                            bull[i].y=round(bull[i].sy-(bull[i].length)*sin(bull[i].jiao*PI));
                            if ((bull[i].x<0)||(bull[i].y<0)||(bull[i].x>1072)||(bull[i].y>480))  bull[i].flag=false;
                            for (k=1; k<=200; k++)
                                if ((zom[k].flag)&&(zom[k].hp>0))  {
                                    //if ((bull[i].x>=zom[k].x)&&(bull[i].x<=zom[k].x+18)
                                    //      &&(bull[i].y>=zom[k].y)&&(bull[i].y<=zom[k].y+32)) {
                                    int result=GetItAtZombie(bull[i].x-zom[k].x,bull[i].y-zom[k].y,zom[k].lei,zom[k].walknow);
                                    if (result!=GI_NOTHING&&result!=GI_OUTOFRANGE) {
                                        bull[i].flag=false; DamageZombie(&zom[k],bull[i].damage);
                                        PutBlood(bull[i].x,bull[i].y,result);
                                        break;
                                    };
                                };
                            if (GetPixelAtBmp(&formmark,bull[i].x,bull[i].y)==RGB(255,255,0))  {
                                bull[i].flag=false;
                                //if random(2)=1
                                //sndPlaySound(pchar(ansistring('music\sound\hweapons\ric_conc-'+inttostr(random(2)+1)+'.wav')),1);
                            };
                        };
                        //bull[i].length=bull[i].length+15;
                        //if abs(bull[i].jiao)>90  bull[i].x=round(bull[i].sx-bull[i].length*sin(bull[i].jiao*PI))
                        //else bull[i].x=round(bull[i].sx+bull[i].length*sin(bull[i].jiao*PI));
                        //bull[i].y=round(bull[i].sy-bull[i].length*cos(bull[i].jiao*PI));
                    };
                QueryPerformanceCounter(&nowtime);
                deltat=(double)(nowtime.QuadPart-lasttime.QuadPart)/frequent.QuadPart;
                lasttime=nowtime;
                for (i=1; i<=200; i++)
                    if (gr[i].flag) {
                        gr[i].t=gr[i].t+deltat;
                        gr[i].x=gr[i].nowx+round(gr[i].t*gr[i].vx*100/5);
                        gr[i].y=gr[i].nowy-round((gr[i].vy*gr[i].t-4.9*gr[i].t*gr[i].t)*100/5);
                        if (GetPixelAtBmp(&formmark,gr[i].x,gr[i].y)==RGB(255,255,0))  {
                            if (getdis(gr[i].x,gr[i].y,mann.x+8,mann.y+13)<100)  {//榴弹太近可能反弹
                                if (getdis(gr[i].x,gr[i].y,mann.x+8,mann.y+13)<50)  gr[i].flag=false; //榴弹直接消失到地上
                                if (gr[i].boom)  {
                                    gr[i].boom=false;
                                    tt=gr[i].t;
                                    v11=(gr[i].vy*tt-4.9*tt*tt); tt=tt-0.1; //用位移差计算瞬时速度
                                    gr[i].vy=(((gr[i].vy*tt-4.9*tt*tt)-v11)/0.1*3/4); gr[i].t=0;
                                    if (gr[i].vy>20)  gr[i].flag=false;
                                    v11=0; tt=0;
                                    if (fabs(gr[i].vy)>0.5)  {
                                        gr[i].nowx=gr[i].x; gr[i].nowy=gr[i].y;
                                        zhennow=1; zhenhan=1;
                                        continue;
                                    };
                                } else gr[i].flag=false;
                            };
                            gr[i].flag=false;
                            if (gr[i].boom)  {
                                boomsum++; if (boomsum>15)  boomsum=1;
                                boom[boomsum].now=0; boom[boomsum].x=gr[i].x; boom[boomsum].y=gr[i].y;
                                now=GetIt(gr[i].x,gr[i].y);
                                if (now==GI_TOP)  {
                                    boom[boomsum].boomflag=1;
                                    for (k=1; k<=200; k++)        //榴弹在地上爆炸
                                        if (zom[k].flag)  {
                                            if ((getdis(zom[k].x+9,zom[k].y+16,gr[i].x,gr[i].y)<=100)
                                                &&getjiao(gr[i].x,gr[i].y,zom[k].x+9,zom[k].y+16)>=0&&getjiao(gr[i].x,gr[i].y,zom[k].x+9,zom[k].y+16)<=180)  {
                                                DamageZombie(&zom[k],round(gr[i].damage*(2/(0.1*getdis(zom[k].x+9,zom[k].y+16,gr[i].x,gr[i].y)+2))));
                                                zom[k].drop=true;
                                                double angle=getjiao(gr[i].x,gr[i].y,zom[k].x+9,zom[k].y+16);
                                                zom[k].vx=10*cos(angle*PI);
                                                zom[k].vy=10*sin(angle*PI);
                                                zom[k].nowx=zom[k].x; zom[k].nowy=zom[k].y; QueryPerformanceCounter(&zom[k].lasttime);
                                            };
                                        };
                                } else if (now==GI_BOTTOM)  {   //天花板炸开
                                    boom[boomsum].boomflag=2;
                                    for (k=1; k<=200; k++)
                                        if (zom[k].flag) {
                                            if ((getdis(zom[k].x+9,zom[k].y+16,gr[i].x,gr[i].y)<=100)
                                                &&getjiao(gr[i].x,gr[i].y,zom[k].x+9,zom[k].y+16)>=180&&getjiao(gr[i].x,gr[i].y,zom[k].x+9,zom[k].y+16)<=360)  {
                                                DamageZombie(&zom[k],round(gr[i].damage*(2/(0.1*getdis(zom[k].x+9,zom[k].y+16,gr[i].x,gr[i].y)+2))));
                                                zom[k].drop=true;
                                                double angle=getjiao(gr[i].x,gr[i].y,zom[k].x+9,zom[k].y+16);
                                                zom[k].vx=10*cos(angle*PI);
                                                zom[k].vy=10*sin(angle*PI);
                                                zom[k].nowx=zom[k].x; zom[k].nowy=zom[k].y; QueryPerformanceCounter(&zom[k].lasttime);
                                            };
                                        };
                                } else if (now==GI_LEFT)  {
                                    boom[boomsum].boomflag=3;
                                    for (k=1; k<=200; k++)
                                        if (zom[k].flag)  {
                                            if ((getdis(zom[k].x+9,zom[k].y+16,gr[i].x,gr[i].y)<=100)
                                                &&getjiao(gr[i].x,gr[i].y,zom[k].x+9,zom[k].y+16)>=90&&getjiao(gr[i].x,gr[i].y,zom[k].x+9,zom[k].y+16)<=270)  {
                                                DamageZombie(&zom[k],round(gr[i].damage*(2/(0.1*getdis(zom[k].x+9,zom[k].y+16,gr[i].x,gr[i].y)+2))));
                                                zom[k].drop=true;
                                                double angle=getjiao(gr[i].x,gr[i].y,zom[k].x+9,zom[k].y+16);
                                                zom[k].vx=10*cos(angle*PI);
                                                zom[k].vy=10*sin(angle*PI);
                                                zom[k].nowx=zom[k].x; zom[k].nowy=zom[k].y; QueryPerformanceCounter(&zom[k].lasttime);
                                            };
                                        };
                                } else if (now==GI_RIGHT)  {
                                    boom[boomsum].boomflag=4;
                                    for (k=1; k<=200; k++)
                                        if (zom[k].flag)  {
                                        if ((getdis(zom[k].x+9,zom[k].y+16,gr[i].x,gr[i].y)<=100)
                                            &&getjiao(gr[i].x,gr[i].y,zom[k].x+9,zom[k].y+16)>=270||getjiao(gr[i].x,gr[i].y,zom[k].x+9,zom[k].y+16)<=90)  {
                                            DamageZombie(&zom[k],round(gr[i].damage*(2/(0.1*getdis(zom[k].x+9,zom[k].y+16,gr[i].x,gr[i].y)+2))));
                                            zom[k].drop=true;
                                            double angle=getjiao(gr[i].x,gr[i].y,zom[k].x+9,zom[k].y+16);
                                            zom[k].vx=10*cos(angle*PI);
                                            zom[k].vy=10*sin(angle*PI);
                                            zom[k].nowx=zom[k].x; zom[k].nowy=zom[k].y; QueryPerformanceCounter(&zom[k].lasttime);
                                        };
                                    };
                                };
                                sprintf(temps,"music\\sound\\Explosions\\exp%d.wav",rand()%4+1);
                                PlaySound(temps);
                                zhennow=1; zhenhan=10;
                            };
                        };
                    };
                    #ifndef _SHOW_WAY_
                        printnow=true;
                    #endif // _SHOW_WAY_
                    }
            } else if (WParam==6) {
                if (turnblack>5) turnblack-=5;
                else {
                    KillTimer(hWindow,6);
                    turnblack=0;
                }
            } else if (WParam==7) {
                if (turnblack<250) turnblack+=5;
                else {
                    KillTimer(hWindow,7);
                    turnblack=255;
                    DisplayShop();
                    SaveTheGame();
                }
            } else if (WParam==8) {
                KillTimer(hWindow,8);
                SetTimer(hWindow,7,10,NULL);
            }
            break;
        case WM_MOUSEMOVE:
            if (!pause&&!quit) {
            mousex=LOWORD(LParam); mousey=HIWORD(LParam);
            if (mann.hp>0) {
                mann.guna=getjiao(mann.x-minx+8,mann.y+14,mousex,mousey);
                jiao=getjiao(mann.x-minx+8,mann.y+13,mousex,mousey);
                if (fabs(jiao)>90&&fabs(jiao)<270) {
                    if (mann.toward) mann.up=-mann.up;
                    mann.toward=false;
                } else {
                    if (!mann.toward) mann.up=-mann.up;
                    mann.toward=true;
                }
                mann.jiao=jiao;

            }
            printnow=true;
            }
            break;
        case WM_LBUTTONDOWN:
            if (!pause&&!quit) {
            if (mann.hp>0&&nextfire) {
                if (hweapon[weanow].running)
                    SetTimer(hWindow,2,round(1000/(hweapon[weanow].speed/60)),NULL);
                else {
                    nextfire=false;
                    SetTimer(hWindow,2,round(1000/(hweapon[weanow].speed/60)),NULL);
                }
                fire();
            }
            }
            break;
        case WM_LBUTTONUP:
            mann.fire=false;//print;
            if (hweapon[weanow].running) KillTimer(hWindow,2);
            break;
        case WM_CLOSE:
            QueryQuit();
            return false;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(Window, AMessage, WParam, LParam);
}
bool WinRegister(HINSTANCE hInstance)
{
    WNDCLASS WindowClass;
    WindowClass.style=CS_SAVEBITS;
    WindowClass.lpfnWndProc=&WindowProc;
    WindowClass.cbClsExtra=0;
    WindowClass.cbWndExtra=0;
    WindowClass.hInstance=hInstance;
    WindowClass.hIcon=(HICON)LoadImage(0,"image\\1.ico",IMAGE_ICON,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE);
    WindowClass.hCursor=LoadCursor(0,IDC_ARROW);
    WindowClass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
    WindowClass.lpszMenuName=NULL;
    WindowClass.lpszClassName=AppName;
    return RegisterClass(&WindowClass)!=0;
}
int WINAPI WinMain(
            HINSTANCE hInstance,
            HINSTANCE hPrevInstance,
            LPSTR lpCmdLine,
            int nShowCmd)
{
    char temps[50];
    int i;
    MSG messages;
    QueryPerformanceFrequency(&frequent);
    Useful_Init();
    Sound_Init();
    Blood_Init();
    Smog_Init();
    srand(time(0));
    Hinstance=hInstance;
    if (!WinRegister(hInstance)) {
        MessageBox(0,"Register failed",NULL,MB_OK);
        return 0;
    }
    hWindow=WinCreate(hInstance);
    if (hWindow==0) {
        MessageBox(0,"WinCreate failed",NULL,MB_OK);
        ExitThread(1);
    }
    pause=true; mainmenu=true; shop=false; Box=false; dead=false;
    CreateImaButton(&button1,"暂停",hWindow,0,10,10,50,20,101,"image\\button2\\");
    //CreateImaButton(&button2,"继续",hWindow,0,320-80,MenuKeepY,160,30,101,"image\\button2\\");
    CreateImaButton(&button2,"新游戏",hWindow,0,320-80,MenuKeepY,160,30,101,"image\\button2\\");
    ShowWindow(button1.hwindow,SW_SHOW);
    ShowWindow(button2.hwindow,SW_SHOW);
    //CreateImaButton(&button5,"返回主菜单",hWindow,0,320-80,MenuBackY,160,30,102,"image\\button2\\");
    //CreateImaButton(&button6,"快速存档",hWindow,0,320-80,MenuSaveY,160,30,103,"image\\button2\\");
    //CreateImaButton(&button7,"退出游戏",hWindow,0,320-80,MenuExitY,160,30,104,"image\\button2\\");
    CreateImaButton(&button5,"继续游戏",hWindow,0,320-80,MenuBackY,160,30,102,"image\\button2\\");
    CreateImaButton(&button6,"设置",hWindow,0,320-80,MenuSaveY,160,30,103,"image\\button2\\");
    CreateImaButton(&button7,"退出游戏",hWindow,0,320-80,MenuExitY,160,30,104,"image\\button2\\");
    CreateImaButton(&button3,"取消",hWindow,0,-82,BoxOKY,82,20,201,"image\\button2\\");
    CreateImaButton(&button4,"确定",hWindow,0,640,BoxOKY,83,20,202,"image\\button2\\");
    ShowWindow(button3.hwindow,SW_SHOW);
    ShowWindow(button4.hwindow,SW_SHOW);

    CreateImaButton(&bprevious,"<",hWindow,0,ShopWeaWindowLeft-40,(ShopWeaWindowBottom+ShopWeaWindowTop)/2+20,40,40,301,"image\\button2\\");
    CreateImaButton(&bnext,">",hWindow,0,ShopWeaWindowRight,(ShopWeaWindowBottom+ShopWeaWindowTop)/2+20,40,40,302,"image\\button2\\");
    CreateImaButton(&bbuy,"购买",hWindow,0,ShopWeaWindowRight-120,ShopWeaWindowBottom,40,40,303,"image\\button2\\");
    CreateImaButton(&bcontinue,"继续游戏",hWindow,0,640-200,480-60,60,40,304,"image\\button2\\");
    CreateImaButton(&bexit,"保存并退出",hWindow,0,640-120,480-60,90,40,305,"image\\button2\\");
    ShowWindow(bprevious.hwindow,SW_HIDE);
    ShowWindow(bnext.hwindow,SW_HIDE);
    ShowWindow(bbuy.hwindow,SW_HIDE);
    ShowWindow(bcontinue.hwindow,SW_HIDE);
    ShowWindow(bexit.hwindow,SW_HIDE);

    bullsum=1;
    for (i=1; i<=5; i++) {
        sprintf(temps,"image\\sky%d.png",i);
        loadpic(temps,&sky[i]);
    }
    for (i=1; i<=3; i++) {
        sprintf(temps,"image\\backcity%d.png",i);
        loadpic(temps,&backcity[i]);
    }
    for (i=1; i<=3; i++) {
        sprintf(temps,"image\\building%d.png",i);
        loadpic(temps,&building[i]);
    }
    loadpic("image\\building1 form.png",&form[1]);
    loadpic("image\\building2 form.png",&form[2]);
    loadpic("image\\building3 form.png",&form[3]);
    openbmp(form[2],&formmark);
    loadpic("image\\man1.png",&man);
    loadpic("image\\aek971.png",&weatest);
    for (i=1; i<=18; i++) {
        sprintf(temps,"image\\zom%d.png",i);
        loadpic(temps,&zombies[i]);
        openbmp(zombies[i],&formzombie[i]);
    }
    mann.y=480-48-32; mann.x=320-17+200;
    mann.toward=true;
    mann.now=Nothing;
    mann.show=1;
    mann.hp=100;
    girl.now=LieOnTheBox;
    girl.x=422;
    girl.y=253;
    wave=0;

    //Weapon Init
    ReadWeaponConfig();
    nextfire=true;
    for (i=1; i<=15; i++) {
        sprintf(temps,"image\\boom\\boom1_%d.bmp",i);
        loadpic(temps,&bopic[i]);
    }
    loadpic("image\\bull.png",&bul);
    loadpic("image\\lie.png",&lie);
    loadpic("image\\smog.png",&smogpic);
    loadpic("image\\menu.png",&menu);
    RECT r;
    HFONT font;
    r.top=2; r.left=2;
    r.bottom=22; r.right=197;
    SetTextColor(menu.color,RGB(255,255,255));
    SetBkMode(menu.color,1);
    MakeFont("幼圆",14,&font);
    SelectObject(menu.color,font);
    DrawText(menu.color,"丧尸之城",-1,&r,DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    loadpic("image\\box.png",&box);
    for (i=1; i<=100; i++) boom[i].now=16;
    weanow=1; boomsum=0; zhennow=0;
    minx=200; minaim=minx;
    zombieleft=30;
    //ReadTheGame();
    //QueryPerformanceCounter(&lasttime);

    print();
    SetTimer(hWindow,1,40,NULL);
    SetTimer(hWindow,5,10,NULL);
    turnblack=255;
    SetTimer(hWindow,6,10,NULL);
    //QueryPerformanceFrequency(frequent);
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* S} message to WindowProcedure */
        DispatchMessage(&messages);
    }
    closebmp(form[2],&formmark);
    Useful_Final();
    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}
