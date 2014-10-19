//
//  SceneTiles.h
//  assignment3
//
//  Created by Charles Yang on 10/18/14.
//
//

#ifndef __assignment3__SceneTiles__
#define __assignment3__SceneTiles__

#include <stdio.h>

class SceneTiles{
public:

    enum MeshType {BLANK_TILE = 0, BASIC_TILE = 1};
    enum TileType {ROAD, SPIRE, WALL};




    static void setupTiles();
    static void drawTiles();



private:
    static int foo;

};



#endif /* defined(__assignment3__SceneTiles__) */
