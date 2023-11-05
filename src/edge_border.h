
#ifndef EDGE_BORDER_H
#define EDGE_BORDER_H

namespace fallout {

extern int mapModWidth;
extern int mapModHeight;
extern bool EdgeBorderEnabled;

typedef struct tagPOINT {
    int x;
    int y;
} POINT;

static class EdgeBorder {
public:
    class Edge {
    public:
        POINT center; // x/y center of current map screen?
        Rect borderRect; // right is less than left
        Rect rect_2;
        Rect tileRect;
        Rect squareRect; // angel clipping
        int clipData; // angel clip type
        Edge* prevEdgeData = nullptr; // unused (used in 3.06)
        Edge* nextEdgeData = nullptr;

        void Release()
        {
            Edge* edge = this->nextEdgeData;
            while (edge) {
                Edge* edgeNext = edge->nextEdgeData;
                delete edge;
                edge = edgeNext;
            };
            this->nextEdgeData = nullptr;
        }

        ~Edge()
        {
            if (nextEdgeData) delete nextEdgeData;
        }
    };

    static Edge* CurrentMapEdge();
    static int EdgeVersion();

    static int GetCenterTile(int tile, int mapLevel);

    static int CheckBorder(int tile);

    static void SetDefaultEdgeData();
    static int LoadMapEdgeFileSub(char* edgPath);
};

} // namespace fallout

#endif
