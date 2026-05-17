#include "save.h"
extern int bullsum,grasum,hweasum,weasum,zombieleft,zombiesum,killzombie,wave;
extern int minx,minaim,weanow,boomsum;
extern weanode weapon[30],hweapon[30];
extern boomnode boom[100];
extern bullnode bull[211];
extern granode gr[211];
extern zombie zom[211];
extern bloodnode blood[5000];
extern smognode smog[100];
extern people mann;
extern bool shop;
void ReadWeaponConfig()
{
    FILE *f;
    int i,readima;
    int weanow;
    char com[256],param[256],st[256];
    weasum=0;
    f=fopen("image\\weapon\\weapon.ini","r");
    while (!feof(f)) {
        fgets(st,256,f);
        if (st[strlen(st)-1]=='\n') st[strlen(st)-1]='\0';
        for (i=0; i<strlen(st); i++) {
            if (st[i]=='=') break;
            if (st[i]>='A'&&st[i]<='Z')
                st[i]=st[i]+32;
        }
        if (strchr(st,'=')==NULL) {
            strcpy(com,""); strcpy(param,"");
        } else {
            strcpy(com,st); com[pos('=',com)-1]='\0';
            strcpy(param,strchr(st,'=')+1);
        }
        if (strcmp(com,"#weapon")==0) {
            weanow=StrToInt(param);
            weasum++;
        } else if (strcmp(com,"name")==0) strcpy(weapon[weanow].name,param);
        else if (strcmp(com,"imagepath")==0) {
            strcpy(weapon[weanow].imagepath,param);
            loadpic(param,&weapon[weanow].image);
        } else if (strcmp(com,"shopimage")==0) {
            strcpy(weapon[weanow].simagepath,param);
            loadpic(param,&weapon[weanow].simage);
        } else if (strcmp(com,"routex")==0) weapon[weanow].routex=StrToInt(param);
        else if (strcmp(com,"routey")==0) weapon[weanow].routey=StrToInt(param);
        else if (strcmp(com,"type")==0) weapon[weanow].lei=StrToInt(param);
        else if (strcmp(com,"magazine")==0) weapon[weanow].danjia=StrToInt(param);
        else if (strcmp(com,"reloadtime")==0) weapon[weanow].reloadtime=StrToInt(param);
        else if (strcmp(com,"up")==0) weapon[weanow].up=StrToReal(param);
        else if (strcmp(com,"damage")==0) weapon[weanow].damage=StrToInt(param);
        else if (strcmp(com,"running")==0) weapon[weanow].running=StrToInt(param);
        else if (strcmp(com,"speed")==0) weapon[weanow].speed=StrToInt(param);
        else if (strcmp(com,"diameter")==0) weapon[weanow].diameter=StrToReal(param);
        else if (strcmp(com,"price")==0) weapon[weanow].price=StrToInt(param);
        else if (strcmp(com,"sound")==0) strcpy(weapon[weanow].shoot,param);
        else if (strcmp(com,"sound1")==0) strcpy(weapon[weanow].sound[0],param);
        else if (strcmp(com,"sound2")==0) strcpy(weapon[weanow].sound[1],param);
        else if (strcmp(com,"sound3")==0) strcpy(weapon[weanow].sound[2],param);
        else if (strcmp(com,"sound4")==0) strcpy(weapon[weanow].sound[3],param);
        else if (strcmp(com,"sound5")==0) strcpy(weapon[weanow].sound[4],param);
        else if (strcmp(com,"play1")==0) weapon[weanow].play[0]=StrToInt(param);
        else if (strcmp(com,"play2")==0) weapon[weanow].play[1]=StrToInt(param);
        else if (strcmp(com,"play3")==0) weapon[weanow].play[2]=StrToInt(param);
        else if (strcmp(com,"play4")==0) weapon[weanow].play[3]=StrToInt(param);
        else if (strcmp(com,"play5")==0) weapon[weanow].play[4]=StrToInt(param);
    }
    fclose(f);
}
//int bullsum,grasum,weasum,zombieleft;
//int minx,minaim,weanow,boomsum;
//boomnode boom[100];
//bullnode bull[211];
//granode gr[211];
//zombie zom[211];
void SaveTheGame()
{
    FILE *f;
    int now=0,i;
    f=fopen("save.dat","w");
/*
typedef struct {
    int now,walknow;
    int x,y,nowx,nowy,show;
    bool toward,drop,fire,walktoward,reload,zhuang;
    double guna,v0,v1,t,jiao,showangle,up,canup,hp;
    COLORREF showcolor;
    byte tans;
    LARGE_INTEGER nowtime,lasttime;
} people;
*/
    fprintf(f,"#Man Data\n");
    fprintf(f,"%d %d %d %d %d %d %d %d\n",mann.now,mann.walknow,mann.x,mann.y,mann.nowx,mann.nowy,mann.show,mann.money);
    fprintf(f,"%d %d %d %d\n",mann.toward,mann.drop,mann.walktoward,mann.zhuang);
    fprintf(f,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",mann.guna,mann.v0,mann.v1,mann.t,mann.jiao,mann.showangle,mann.up,mann.canup,mann.hp);

    fprintf(f,"#Weapon Data\n");
    fprintf(f,"%d\n",hweasum);
    for (i=1; i<=hweasum; i++) {
        fprintf(f,"%s\n",hweapon[i].name);
        fprintf(f,"%d %d %d\n", hweapon[i].left,hweapon[i].now,hweapon[i].danjia);
    }
    fprintf(f,"#Some Data\n");
    fprintf(f,"%d %d %d %d %d\n",minx,minaim,weanow,wave,shop);
    fprintf(f,"#Zombie Data\n");
    fprintf(f,"%d %d %d\n",zombieleft,zombiesum,killzombie);
    for (i=1; i<=200; i++)
        if (zom[i].flag) {
            fprintf(f,"#%d\n",++now);
            fprintf(f,"%d %d %d %d %d %d %d %d %d\n",zom[i].now,zom[i].walknow,zom[i].x,zom[i].y,zom[i].nowx,zom[i].nowy,zom[i].lei,zom[i].hp,zom[i].showtime);
            fprintf(f,"%d %d %d\n",zom[i].drop,zom[i].walktoward,zom[i].zhuang);
            fprintf(f,"%lf %lf %lf %lf %lf %lf\n",zom[i].vx,zom[i].vy,zom[i].t,zom[i].jiao,zom[i].acttime,zom[i].sumacttime);
            fprintf(f,"%d %d %d\n",zom[i].trans,zom[i].next.act,zom[i].next.y);
        }
/*typedef struct {
    int sx,sy,x,y,damage;
    double jiao,length;
    bool flag;
} bullnode;*/
    fprintf(f,"#Bull Data\n");
    now=0;
    for (i=1; i<=200; i++)
        if (bull[i].flag) {
            fprintf(f,"#%d\n",++now);
            fprintf(f,"%d %d %d %d %d\n",bull[i].sx,bull[i].sy,bull[i].x,bull[i].y,bull[i].damage);
            fprintf(f,"%lf %lf\n",bull[i].jiao,bull[i].length);
        }
/*typedef struct {
    int nowx,nowy,x,y,damage;
    double vx,vy,t,jiao;
    bool flag,boom;
} granode;*/
    fprintf(f,"#Grenade Data\n");
    now=0;
    for (i=1; i<=200; i++)
        if (gr[i].flag) {
            fprintf(f,"#%d\n",++now);
            fprintf(f,"%d %d %d %d %d %d\n",gr[i].nowx,gr[i].nowy,gr[i].x,gr[i].y,gr[i].damage,gr[i].boom);
            fprintf(f,"%lf %lf %lf %lf\n",gr[i].vx,gr[i].vy,gr[i].t,gr[i].jiao);
        }
/*typedef struct {
    int x,y;
    double now;
    int boomflag;
} boomnode;*/
    fprintf(f,"#Boom Data\n");
    now=0;
    for (i=1; i<=15; i++)
        if (boom[i].now<16) {
            fprintf(f,"#%d\n",++now);
            fprintf(f,"%d %d %d\n",boom[i].x,boom[i].y,boom[i].boomflag);
            fprintf(f,"%lf\n",boom[i].now);
        }
/*typedef struct {
    int nowx,nowy,x,y;
    double vx,vy,t,lefttime;
    bool flag,drop;
    LARGE_INTEGER nowtime,lasttime;
} bloodnode;*/
    fprintf(f,"#Blood Data\n");
    now=0;
    for (i=0; i<5000; i++)
        if (blood[i].flag) {
            fprintf(f,"#%d\n",++now);
            fprintf(f,"%d %d %d %d %d\n",blood[i].nowx,blood[i].nowy,blood[i].x,blood[i].y,blood[i].drop);

        }
/*typedef struct {
    int nowx,nowy,x,y,trans;
    double vx,vy,t,lefttime;
    bool flag;
    LARGE_INTEGER nowtime,lasttime;
} smognode;*/
    fprintf(f,"#Smog Data\n");
    now=0;
    for (i=0; i<100; i++)
        if (smog[i].flag) {
            fprintf(f,"#%d\n",++now);
            fprintf(f,"%d %d %d %d %d\n",smog[i].nowx,smog[i].nowy,smog[i].x,smog[i].y,smog[i].trans);
            fprintf(f,"%lf %lf %lf %lf\n",smog[i].vx,smog[i].vy,smog[i].t,smog[i].lefttime);

        }
    fclose(f);
}
void ReadTheGame()
{
    FILE *f;
    int i,x,j;
    char st[256];
    bool notread=false;
    f=fopen("save.dat","r");
    if (f==NULL) return;
    while (!feof(f)) {
        if (!notread) fgets(st,256,f);
        if (st[strlen(st)-1]=='\n') st[strlen(st)-1]='\0';
        for (i=0; i<strlen(st); i++)
            if (st[i]>='A'&&st[i]<='Z')
                st[i]=st[i]+32;
        if (strcmp(st,"#man data")==0) {
            fscanf(f,"%d %d %d %d %d %d %d %d\n",&mann.now,&mann.walknow,&mann.x,&mann.y,&mann.nowx,&mann.nowy,&mann.show,&mann.money);
            fscanf(f,"%d %d %d %d\n",&mann.toward,&mann.drop,&mann.walktoward,&mann.zhuang);
            fscanf(f,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",&mann.guna,&mann.v0,&mann.v1,&mann.t,&mann.jiao,&mann.showangle,&mann.up,&mann.canup,&mann.hp);
            QueryPerformanceCounter(&mann.lasttime);
        } else if (strcmp(st,"#weapon data")==0) {
            fscanf(f,"%d\n",&hweasum);
            for (i=1; i<=hweasum; i++) {
                fgets(hweapon[i].name,256,f);
                hweapon[i].name[strlen(hweapon[i].name)-1]='\0';
                for (j=1; j<=weasum; j++)
                    if (!strcmp(hweapon[i].name,weapon[j].name)) {
                        hweapon[i]=weapon[j]; break;
                    }
                fscanf(f,"%d %d %d\n", &hweapon[i].left,&hweapon[i].now,&hweapon[i].danjia);
            }
        } else if (strcmp(st,"#some data")==0) {
            fscanf(f,"%d %d %d %d %d\n",&minx,&minaim,&weanow,&wave,&shop);
        } else if (strcmp(st,"#zombie data")==0) {
            fscanf(f,"%d %d %d\n",&zombieleft,&zombiesum,&killzombie);
            for (x=1; x<=200; x++) {
                fgets(st,256,f);
                if (st[1]>='0'&&st[1]<='9')
                    sscanf(st,"#%d",&i);
                else {
                    notread=true;
                    break;
                }
                zom[i].flag=true;
                fscanf(f,"%d %d %d %d %d %d %d %d %d\n",&zom[i].now,&zom[i].walknow,&zom[i].x,&zom[i].y,&zom[i].nowx,&zom[i].nowy,&zom[i].lei,&zom[i].hp,&zom[i].showtime);
                fscanf(f,"%d %d %d\n",&zom[i].drop,&zom[i].walktoward,&zom[i].zhuang);
                fscanf(f,"%lf %lf %lf %lf %lf %lf\n",&zom[i].vx,&zom[i].vy,&zom[i].t,&zom[i].jiao,&zom[i].acttime,&zom[i].sumacttime);
                fscanf(f,"%d %d %d\n",&zom[i].trans,&zom[i].next.act,&zom[i].next.y);
                QueryPerformanceCounter(&zom[i].lasttime);
            }
        } else if (strcmp(st,"#bull data")==0) {
            for (x=1; x<=200; x++) {
                fgets(st,256,f);
                if (st[1]>='0'&&st[1]<='9')
                    sscanf(st,"#%d",&i);
                else {
                    notread=true;
                    break;
                }
                bull[i].flag=true;
                fscanf(f,"%d %d %d %d %d\n",&bull[i].sx,&bull[i].sy,&bull[i].x,&bull[i].y,&bull[i].damage);
                fscanf(f,"%lf %lf\n",&bull[i].jiao,&bull[i].length);
            }
        } else if (strcmp(st,"#grenade data")==0) {
            for (x=1; x<=200; x++) {
                fgets(st,256,f);
                if (st[1]>='0'&&st[1]<='9')
                    sscanf(st,"#%d",&i);
                else {
                    notread=true;
                    break;
                }
                gr[i].flag=true;
                fscanf(f,"%d %d %d %d %d %d\n",&gr[i].nowx,&gr[i].nowy,&gr[i].x,&gr[i].y,&gr[i].damage,&gr[i].boom);
                fscanf(f,"%lf %lf %lf %lf\n",&gr[i].vx,&gr[i].vy,&gr[i].t,&gr[i].jiao);
            }
        } else if (strcmp(st,"#boom data")==0) {
            for (x=1; x<=15; x++) {
                fgets(st,256,f);
                if (st[1]>='0'&&st[1]<='9')
                    sscanf(st,"#%d",&i);
                else {
                    notread=true;
                    break;
                }
                fscanf(f,"%d %d %d\n",&boom[i].x,&boom[i].y,&boom[i].boomflag);
                fscanf(f,"%lf\n",&boom[i].now);
            }
        } else if (strcmp(st,"#blood data")==0) {
            for (x=0; x<5000; x++) {
                fgets(st,256,f);
                if (st[1]>='0'&&st[1]<='9')
                    sscanf(st,"#%d",&i);
                else {
                    notread=true;
                    break;
                }
                blood[i].flag=true;
                fscanf(f,"%d %d %d %d %d\n",&blood[i].nowx,&blood[i].nowy,&blood[i].x,&blood[i].y,&blood[i].drop);
                QueryPerformanceCounter(&blood[i].lasttime);
            }
        } else  if (strcmp(st,"#smog data")==0) {
            for (x=0; x<100; x++) {
                fgets(st,256,f);
                if (st[1]>='0'&&st[1]<='9')
                    sscanf(st,"#%d",&i);
                else {
                    notread=true;
                    break;
                }
                smog[i].flag=true;
                fscanf(f,"%d %d %d %d %d\n",&smog[i].nowx,&smog[i].nowy,&smog[i].x,&smog[i].y,&smog[i].trans);
                fscanf(f,"%lf %lf %lf %lf\n",&smog[i].vx,&smog[i].vy,&smog[i].t,&smog[i].lefttime);
                QueryPerformanceCounter(&smog[i].lasttime);
            }
        }
    }
    if (hweapon[weanow].now<=0) {
        mann.showangle=0; mann.showcolor=RGB(255,255,255);
    }
    fclose(f);
}
