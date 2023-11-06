
#include "tile.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include <algorithm>
#include <stack>

#include "art.h"
#include "color.h"
#include "config.h"
#include "debug.h"
#include "draw.h"
#include "game_mouse.h"
#include "light.h"
#include "map.h"
#include "object.h"
#include "platform_compat.h"
#include "settings.h"
#include "svga.h"

#include "edge_border.h"

namespace fallout {

static EdgeBorder::Edge* MapEdgeData = nullptr;
static EdgeBorder::Edge* currentMapEdge; // reference

static long edgeVersion; // 0 - version 1 (obsolete), 1 - version 2 (current)
static bool isSetDefaultEdge;

int mapWidthModSize;
int mapHeightModSize;

int mapModWidth;
int mapModHeight;

bool EdgeBorderEnabled = true;

void GetWinMapHalfSize(int& outW, int& outH)
{
    int mapWinWidth = screenGetWidth();
    int mapWinHeight = screenGetHeight() - 100;

    int wHalf = mapWinWidth >> 1;
    int hHalf = mapWinHeight >> 1;

    mapWidthModSize = wHalf % 32;
    mapHeightModSize = hHalf % 24;

    outW = wHalf - mapWidthModSize; // truncated by 32 units
    outH = hHalf - mapHeightModSize; // reduced by the remainder
}

void GetTileCoordOffset(int tile, int& outX, int& outY)
{
    int x = tile % 200; // 20100%200 = 100
    int y = (tile / 200) + (x / 2); // 20100/200 = 100+(100/2) = y:150 ???
    y &= 0xFFFFFFFE; // even to down
    x = (2 * x) + 200 - y; // 2*100 = 200+200-150 = x:250 ???

    outY = 12 * y;
    outX = 16 * x;
}

void GetCoordFromOffset(int& inOutX, int& inOutY)
{
    int y = inOutY / 24;
    int x = (inOutX / 32) + y - 100;
    inOutX = x;
    inOutY = (2 * y) - (x / 2);
}

// Implementation from HRP by Mash
static void CalcEdgeData(EdgeBorder::Edge* edgeData, int w, int h)
{
    int x, y;

    GetTileCoordOffset(edgeData->tileRect.left, x, y); // upper left corner?
    edgeData->borderRect.left = x;

    GetTileCoordOffset(edgeData->tileRect.right, x, y); // upper right corner?
    edgeData->borderRect.right = x;

    GetTileCoordOffset(edgeData->tileRect.top, x, y);
    edgeData->borderRect.top = y;

    GetTileCoordOffset(edgeData->tileRect.bottom, x, y);
    edgeData->borderRect.bottom = y;

    int mapWinW = (screenGetWidth() / 2) - 1; // 1280/2 -1 = 639 (320)
    int mapWinH = ((screenGetHeight() - 100) / 2) - 1; // 620/2 -1 = 309 (190)

    edgeData->rect_2.left = edgeData->borderRect.left - mapWinW;
    edgeData->rect_2.right = edgeData->borderRect.right - mapWinW;
    edgeData->rect_2.top = edgeData->borderRect.top + mapWinH;
    edgeData->rect_2.bottom = edgeData->borderRect.bottom + mapWinH;

    int rectW = (edgeData->borderRect.left - edgeData->borderRect.right) / 2;
    int _rectW = rectW;
    if (rectW % 32) {
        rectW -= rectW % 32; // truncate
        _rectW = rectW + 32;
    }
    if (rectW < w) {
        edgeData->borderRect.left -= rectW;
        edgeData->borderRect.right += _rectW;
    } else {
        edgeData->borderRect.left -= w;
        edgeData->borderRect.right += w;
    }

    int rectH = (edgeData->borderRect.bottom - edgeData->borderRect.top) / 2;
    int _rectH = rectH;
    if (rectH % 24) {
        rectH -= rectH % 24;
        _rectH = rectH + 24;
    }
    if (rectH < h) {
        edgeData->borderRect.top += _rectH;
        edgeData->borderRect.bottom -= rectH;
    } else {
        edgeData->borderRect.top += h;
        edgeData->borderRect.bottom -= h;
    }

    if ((edgeData->borderRect.left < edgeData->borderRect.right) || (edgeData->borderRect.left - edgeData->borderRect.right) == 32) {
        edgeData->borderRect.left = edgeData->borderRect.right;
    }
    if ((edgeData->borderRect.bottom < edgeData->borderRect.top) || (edgeData->borderRect.bottom - edgeData->borderRect.top) == 24) {
        edgeData->borderRect.bottom = edgeData->borderRect.top;
    }

    x = edgeData->borderRect.right + ((edgeData->borderRect.left - edgeData->borderRect.right) / 2);
    edgeData->center.x = x - (x % 32);

    y = edgeData->borderRect.top + ((edgeData->borderRect.bottom - edgeData->borderRect.top) / 2);
    edgeData->center.y = y - (y % 24);
}



void EdgeBorder::SetDefaultEdgeData()
{
    int w, h;
    GetWinMapHalfSize(w, h);

    if (MapEdgeData == nullptr) MapEdgeData = new EdgeBorder::Edge[3];

    for (size_t i = 0; i < 3; i++) {
        EdgeBorder::Edge* edge = &MapEdgeData[i];

        edge->tileRect.left = 199;
        edge->tileRect.top = 0;
        edge->tileRect.right = 39800;
        edge->tileRect.bottom = 39999;

        CalcEdgeData(edge, w, h);

        edge->squareRect.left = 99;
        edge->squareRect.top = 0;
        edge->squareRect.right = 0;
        edge->squareRect.bottom = 99;

        edge->clipData = 0;
        edge->prevEdgeData = nullptr;
        edge->nextEdgeData = nullptr;
    }
    edgeVersion = 0;
    isSetDefaultEdge = true;
}

int EdgeBorder::CheckBorder(int tile)
{
    int x, y;
    GetTileCoordOffset(tile, x, y);

    if (x > currentMapEdge->borderRect.left || x < currentMapEdge->borderRect.right || y < currentMapEdge->borderRect.top || y > currentMapEdge->borderRect.bottom) {
        return 0; // block
    }

    int w = mapModWidth;
    int h = mapModHeight;
    mapModHeight = 0;
    mapModWidth = 0;

    if (x <= currentMapEdge->borderRect.left) {
        mapModWidth = -mapWidthModSize;
    } else if (x >= currentMapEdge->borderRect.right) {
        mapModWidth = mapWidthModSize;
    }

    if (y <= currentMapEdge->borderRect.top) {
        mapModHeight = -mapHeightModSize;
    } else if (y >= currentMapEdge->borderRect.bottom) {
        mapModHeight = mapHeightModSize;
    }

    return (w != mapModWidth || h != mapModHeight) ? 1 : -1; // 1 - for redrawing map
}


// Implementation from HRP by Mash
int EdgeBorder::GetCenterTile(int tile, int mapLevel)
{
    if (!isSetDefaultEdge) SetDefaultEdgeData(); // needed at game initialization

    int tX, tY;
    GetTileCoordOffset(tile, tX, tY);

    Edge* edgeData = &MapEdgeData[mapLevel];
    currentMapEdge = edgeData;

    if (edgeData->nextEdgeData) {
        long mapWinW = screenGetWidth();
        long mapWinH = screenGetHeight() - 100;

        long width = (mapWinW / 2) - 1;
        long height = (mapWinH / 2) + 1;
        Edge* edge = edgeData;

        while (tX >= (edge->rect_2.left + width) || tX <= (edge->rect_2.right + width) || tY <= (edge->rect_2.top - height) || tY >= (edge->rect_2.bottom - height)) {
            edge = edgeData->nextEdgeData;
            if (!edge) break;

            edgeData = edge;
            currentMapEdge = edge;
        }
    }

    int leftX = edgeData->borderRect.left;
    if (tX <= leftX) {
        long rightX = edgeData->borderRect.right;
        if (tX >= rightX) {
            edgeData->center.x = tX;
        } else {
            edgeData->center.x = rightX;
        }
    } else {
        edgeData->center.x = leftX;
    }

    int bottomY = edgeData->borderRect.bottom;
    if (tY <= bottomY) {
        int topY = edgeData->borderRect.top;
        if (tY >= topY) {
            edgeData->center.y = tY;
        } else {
            edgeData->center.y = topY;
        }
    } else {
        edgeData->center.y = bottomY;
    }

    mapModHeight = 0;
    mapModWidth = 0;

    if (edgeData->center.x <= edgeData->borderRect.left) {
        mapModWidth = -mapWidthModSize;
    } else if (edgeData->center.x >= edgeData->borderRect.right) {
        mapModWidth = mapWidthModSize;
    }

    if (edgeData->center.y <= edgeData->borderRect.top) {
        mapModHeight = -mapHeightModSize;
    } else if (edgeData->center.y >= edgeData->borderRect.bottom) {
        mapModHeight = mapHeightModSize;
    }

    int cX = edgeData->center.x;
    int cY = edgeData->center.y;
    GetCoordFromOffset(cX, cY);

    return cX + (cY * 200); // tile of center
}



// Implementation from HRP by Mash
 int EdgeBorder::LoadMapEdgeFileSub(char* edgPath)
{
    File* file = fileOpen(edgPath, "rb");
    if (!file) {
        return -1;
    }

    int getValue;
    fileReadInt32(file, &getValue);
    if (getValue != 'EDGE') return -1;

    fileReadInt32(file, &getValue);
    if (getValue == 1) {
        edgeVersion = 0; // old
    } else {
        if (getValue != 2) return -1; // incorrect version
        edgeVersion = 1;
    }

    getValue = 0;
    if (fileReadInt32(file, &getValue) || getValue) return -1; // error, incorrect map level

    int w, h;
    GetWinMapHalfSize(w, h);

    if (MapEdgeData) {
        MapEdgeData[0].Release();
        MapEdgeData[1].Release();
        MapEdgeData[2].Release();
    } else {
        MapEdgeData = new EdgeBorder::Edge[3];
    }

    int mapLevel = 0;
    do {
        EdgeBorder::Edge* edgeData = &MapEdgeData[mapLevel];

        if (edgeVersion) {
            // load rectangle data (version 2)
            if (fileReadInt32List(file, (int*)&edgeData->squareRect, 4) || fileReadInt32(file, (int*)&edgeData->clipData)) {
                return -1; // read error
            }
        } else {
            edgeData->squareRect.left = 99;
            edgeData->squareRect.top = 0;
            edgeData->squareRect.right = 0;
            edgeData->squareRect.bottom = 99;
            edgeData->clipData = 0;
        }

        if (getValue == mapLevel) {
            while (true) {
                long result = fileReadInt32List(file, (int*)&edgeData->tileRect, 4); // load rectangle data
                if (result != 0) return -1; // read error

                CalcEdgeData(edgeData, w, h);

                if (fileReadInt32(file, &getValue)) { // are there more rectangles on the current map level?
                    // the end of file is reached (read error)
                    if (mapLevel != 2) return -1;

                    getValue = -1;
                    break; // next level
                }
                if (getValue != mapLevel) break; // next level

                EdgeBorder::Edge* edge = new EdgeBorder::Edge;
                edge->prevEdgeData = edgeData;
                edge->nextEdgeData = nullptr;
                edge->clipData = 0;
                edge->squareRect = edgeData->squareRect; // rect copy
                edgeData->nextEdgeData = edge;
                edgeData = edge;
            }
        }
    } while (++mapLevel < 3);

    fileClose(file);
    return 0;
}



}

