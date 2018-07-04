#pragma once

#include <cstring>

#define SPEEDTREE_ROOTS 0
#define SPEEDTREE_TRUNKS 1
#define SPEEDTREE_BRANCHES 2
#define SPEEDTREE_LEAVES 3

enum SpeedTreePoly
{
    P_SPEEDTREE_ROOTS = 0,
    P_SPEEDTREE_TRUNKS,
    P_SPEEDTREE_BRANCHES,
    P_SPEEDTREE_LEAVES
};

#define SPEEDTREE_POLY_COUNT 4

#define SPEEDTREE_SHADER "TheSpeedtreeShader"
