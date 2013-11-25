//
// Copyright (c) 2013-2014 Samuel Villarreal
// svkaiser@gmail.com
// 
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 
//    1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 
 //   2. Altered source versions must be plainly marked as such, and must not be
 //   misrepresented as being the original software.
// 
//    3. This notice may not be removed or altered from any source
//    distribution.
// 

#ifndef __MAPDATA_H__
#define __MAPDATA_H__

#define ML_BLOCKING             1               // Solid, is an obstacle.
#define ML_BLOCKMONSTERS        2               // Blocks monsters only.
#define ML_TWOSIDED             4               // Backside will not be present at all if not two sided.

typedef struct {
    int         x;
    int         y;
} mapVertex_t;

typedef struct {
    short	    textureoffset;
    short	    rowoffset;
    word	    toptexture;
    word	    bottomtexture;
    word	    midtexture;
    short	    sector;
} mapSideDef_t;

#define NO_SIDE_INDEX   ((word)-1)

typedef struct {
    word        v1;
    word        v2;
    int         flags;
    short       special;
    short       tag;
    //
    // support more than 32768 sidedefs
    // use the unsigned value and special case the -1
    // sidenum[1] will be -1 (NO_INDEX) if one sided
    //
    word        sidenum[2]; // sidenum[1] will be -1 if one sided
} mapLineDef_t;

typedef	struct {
    short	    floorheight;
    short	    ceilingheight;
    word	    floorpic;
    word	    ceilingpic;
    word	    colors[5];
    short	    special;
    short	    tag;
    word	    flags;
} mapSector_t;

#define	NF_SUBSECTOR 0x8000

typedef struct {
    // Partition line from (x,y) to x+dx,y+dy)
    short	    x;
    short	    y;
    short	    dx;
    short	    dy;
    
    // Bounding box for each child,
    // clip against view frustum.
    short	    bbox[2][4];
    
    // If NF_SUBSECTOR its a subsector,
    // else it's a node of another subtree.
    word	    children[2];
    
} mapNode_t;

typedef struct {
    word        v1;
    word        v2;
    short	    angle;
    word        linedef;
    short	    side;
    short	    offset;
} mapSeg_t;

typedef struct {
    word        numsegs;
    word        firstseg;
} mapSubSector_t;

typedef struct {
    short	    x;
    short	    y;
    short	    z;
    short	    angle;
    short	    type;
    short	    options;
    short	    tid;
} mapThing_t;

typedef struct {
    float       normal[3];
    int         lightmapNum;
    int         lightmapOffs[2];
    int         lightmapDims[2];
    float       lightmapOrigin[3];
    float       lightmapSteps[3][3];
    float       textureCoords[2];
    int         type;
} mapSurface_t;

#endif
