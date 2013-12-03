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
//-----------------------------------------------------------------------------
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "wad.h"
#include "mapData.h"

//
// kexDoomMap::kexDoomMap
//

kexDoomMap::kexDoomMap(void) {
    this->mapLines          = NULL;
    this->mapVerts          = NULL;
    this->mapSides          = NULL;
    this->mapSectors        = NULL;
    this->mapSegs           = NULL;
    this->mapSSects         = NULL;
    this->nodes             = NULL;
    this->leafs             = NULL;
    this->ssLeafLookup      = NULL;
    this->ssLeafCount       = NULL;
    this->segSurfaces[0]    = NULL;
    this->segSurfaces[1]    = NULL;
    this->segSurfaces[2]    = NULL;
    this->leafSurfaces[0]   = NULL;
    this->leafSurfaces[1]   = NULL;

    this->numLeafs      = 0;
    this->numLines      = 0;
    this->numVerts      = 0;
    this->numSides      = 0;
    this->numSectors    = 0;
    this->numSegs       = 0;
    this->numSSects     = 0;
    this->numNodes      = 0;
}

//
// kexDoomMap::~kexDoomMap
//

kexDoomMap::~kexDoomMap(void) {
}

//
// kexDoomMap::BuildMapFromWad
//

void kexDoomMap::BuildMapFromWad(kexWadFile &wadFile) {
    wadFile.GetMapLump<mapThing_t>("THINGS", &mapThings, &numThings);
    wadFile.GetMapLump<mapVertex_t>("VERTEXES", &mapVerts, &numVerts);
    wadFile.GetMapLump<mapLineDef_t>("LINEDEFS", &mapLines, &numLines);
    wadFile.GetMapLump<mapSideDef_t>("SIDEDEFS", &mapSides, &numSides);
    wadFile.GetMapLump<mapSector_t>("SECTORS", &mapSectors, &numSectors);
    wadFile.GetMapLump<mapSeg_t>("SEGS", &mapSegs, &numSegs);
    wadFile.GetMapLump<mapSubSector_t>("SSECTORS", &mapSSects, &numSSects);
    wadFile.GetMapLump<mapNode_t>("NODES", &nodes, &numNodes);
    wadFile.GetMapLump<mapLightInfo_t>("LIGHTS", &lightInfos, &numLightInfos);

    if(mapSegs == NULL) {
        Error("kexDoomMap::BuildMapFromWad: SEGS lump not found\n");
        return;
    }
    if(mapSSects == NULL) {
        Error("kexDoomMap::BuildMapFromWad: SSECTORS lump not found\n");
        return;
    }
    if(nodes == NULL) {
        Error("kexDoomMap::BuildMapFromWad: NODES lump not found\n");
        return;
    }

    printf("------------- Level Info -------------\n");
    printf("Vertices: %i\n", numVerts);
    printf("Segments: %i\n", numSegs);
    printf("Subsectors: %i\n", numSSects);
    printf("Light infos: %i\n\n", numLightInfos);

    BuildLeafs(wadFile);
}

//
// kexDoomMap::BuildLeafs
//

void kexDoomMap::BuildLeafs(kexWadFile &wadFile) {
    lump_t  *lump;
    int     i;
    int     j;
    short   *mlf;
    int     length;
    int     size;
    int     count;

    lump = wadFile.GetLumpFromName("LEAFS");

    if(lump == NULL) {
        Error("kexDoomMap::BuildLeafs: LEAFS lump not found\n");
        return;
    }

    printf("------------- Building leaves from subsectors -------------\n");

    length = lump->size;
    mlf = (short*)wadFile.GetLumpData(lump);

    count = 0;
    size = 0;

    if(length) {
        short   *src = mlf;
        int     next;

        while(((byte*)src - (byte*)mlf) < length) {
            count++;
            size += (word)*src;
            next = (*src << 2) + 2;
            src += (next >> 1);

            printf(".");
        }
    }

    if(count != numSSects) {
        Error("kexDoomMap::BuildLeafs: leaf/subsector inconsistancy %d/%d\n",
            count, numSSects);
    }

    if(count <= 0) {
        return;
    }

    leafs = (leaf_t*)Mem_Calloc((size * 2) * sizeof(leaf_t), hb_static);
    numLeafs = numSSects;

    ssLeafLookup = (int*)Mem_Calloc(sizeof(int) * numSSects, hb_static);
    ssLeafCount = (int*)Mem_Calloc(sizeof(int) * numSSects, hb_static);

    count = 0;

    int lfNum = 0;

    for(i = 0; i < numLeafs; i++) {
        ssLeafCount[i] = (word)*mlf++;
        ssLeafLookup[i] = lfNum;

        if(ssLeafCount[i]) {
            int vertex;
            int seg;
            
            for(j = 0; j < ssLeafCount[i]; j++, lfNum++) {
                vertex = (word)*mlf++;
                if(vertex > numVerts) {
                    Error("kexDoomMap::BuildLeafs: vertex out of range: %i - %i\n",
                        vertex, numVerts);
                }
                
                leafs[lfNum].vertex = &mapVerts[vertex];
                
                seg = *mlf++;
                if(seg == -1) {
                    leafs[lfNum].seg = NULL;
                }
                else {
                    if(seg > numSegs) {
                        Error("kexDoomMap::BuildLeafs: seg out of range: %i - %i\n",
                            seg, numSegs);
                    }
                    
                    leafs[lfNum].seg = &mapSegs[(word)seg];
                }
            }

            printf(".");
        }
    }

    printf("\n\n");
}

//
// kexDoomMap::GetSideDef
//

mapSideDef_t *kexDoomMap::GetSideDef(const mapSeg_t *seg) {
    mapLineDef_t *line;

    if(seg->linedef == -1) {
        return NULL;
    }

    line = &mapLines[seg->linedef];
    return &mapSides[line->sidenum[seg->side]];
}

//
// kexDoomMap::GetFrontSector
//

mapSector_t *kexDoomMap::GetFrontSector(const mapSeg_t *seg) {
    mapSideDef_t *side = GetSideDef(seg);

    if(side == NULL) {
        return NULL;
    }

    return &mapSectors[side->sector];
}

//
// kexDoomMap::GetBackSector
//

mapSector_t *kexDoomMap::GetBackSector(const mapSeg_t *seg) {
    mapLineDef_t *line;

    if(seg->linedef == -1) {
        return NULL;
    }

    line = &mapLines[seg->linedef];

    if(line->flags & ML_TWOSIDED) {
        mapSideDef_t *backSide = &mapSides[line->sidenum[seg->side^1]];
        return &mapSectors[backSide->sector];
    }

    return NULL;
}

//
// kexDoomMap::GetSectorFromSubSector
//

mapSector_t *kexDoomMap::GetSectorFromSubSector(const mapSubSector_t *sub) {
    mapSector_t *sector = NULL;

    // try to find a sector that the subsector belongs to
    for(int i = 0; i < sub->numsegs; i++) {
        mapSeg_t *seg = &mapSegs[sub->firstseg + i];
        if(seg->side != -1) {
            sector = GetFrontSector(seg);
            break;
        }
    }

    return sector;
}

//
// kexDoomMap::PointInSubSector
//

mapSubSector_t *kexDoomMap::PointInSubSector(const int x, const int y) {
    mapNode_t   *node;
    int         side;
    int         nodenum;
    kexVec3     dp1;
    kexVec3     dp2;
    float       d;
    
    // single subsector is a special case
    if(!numNodes) {
        return &mapSSects[0];
    }
    
    nodenum = numNodes - 1;
    
    while(!(nodenum & NF_SUBSECTOR) ) {
        node = &nodes[nodenum];
        
        kexVec3 pt1(F(node->x << 16), F(node->y << 16), 0);
        kexVec3 pt2(F(node->dx << 16), F(node->dy << 16), 0);
        kexVec3 pos(F(x << 16), F(y << 16), 0);

        dp1 = pt1 - pos;
        dp2 = (pt2 + pt1) - pos;
        d = dp1.Cross(dp2).z;

        side = FLOATSIGNBIT(d);

        nodenum = node->children[side ^ 1];
    }
    
    return &mapSSects[nodenum & ~NF_SUBSECTOR];
}
