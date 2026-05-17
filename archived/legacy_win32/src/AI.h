#ifndef _AI_H_
#define _AI_H_

#include <windows.h>
#include <stdbool.h>
#include <math.h>
#include "window.h"
int GetFloor(int y);
int GetFloorPixel(int y);
bool CreateZombie();
void DamageZombie(zombie *zom,int damage);
int GetTarget(int x,int y);
listnode BFS(int x,int y);
#endif
