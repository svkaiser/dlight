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
// DESCRIPTION: Prepares geometry from map structures
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "wad.h"
#include "mapData.h"
#include "surfaces.h"

//#define EXPORT_OBJ

kexArray<surface_t*> surfaces;

typedef struct {
	mapVertex_t     *vertex;
	mapSeg_t        *seg;
} leaf_t;

static int *ssLeafLookup;
static int *ssLeafCount;

typedef struct {
    mapLineDef_t    *mapLines;
    int             numLines;
    mapVertex_t     *mapVerts;
    int             numVerts;
    mapSideDef_t    *mapSides;
    int             numSides;
    mapSector_t     *mapSectors;
    int             numSectors;
    mapSeg_t        *mapSegs;
    int             numSegs;
    mapSubSector_t  *mapSSects;
    int             numSSects;
    leaf_t          *leafs;
    int             numLeafs;
} mapLumps_t;

//
// Surface_AllocateFromSeg
//

void Surface_AllocateFromSeg(mapLumps_t *mapLumps, mapSeg_t *seg) {
    mapLineDef_t *line;
    mapSideDef_t *side;
    surface_t *surf;
    float top, bTop;
    float bottom, bBottom;
    mapSector_t *front;
    mapSector_t *back;

    if(seg->linedef == -1) {
        return;
    }

    line = &mapLumps->mapLines[seg->linedef];
    side = &mapLumps->mapSides[line->sidenum[seg->side]];

    front = &mapLumps->mapSectors[side->sector];

    if(line->flags & ML_TWOSIDED) {
        mapSideDef_t *backSide = &mapLumps->mapSides[line->sidenum[seg->side^1]];
        back = &mapLumps->mapSectors[backSide->sector];
    }
    else {
        back = NULL;
    }

    top = front->ceilingheight;
    bottom = front->floorheight;

    if(back) {
        bTop = back->ceilingheight;
        bBottom = back->floorheight;

        // bottom seg
        if(bottom < bBottom) {
            if(side->bottomtexture != -1) {
                surf = (surface_t*)Mem_Calloc(sizeof(surface_t), hb_static);
                surf->numVerts = 4;
                surf->verts = (kexVec3*)Mem_Calloc(sizeof(kexVec3) * 4, hb_static);

                surf->verts[0].x = surf->verts[2].x = F(mapLumps->mapVerts[seg->v1].x);
                surf->verts[0].y = surf->verts[2].y = F(mapLumps->mapVerts[seg->v1].y);
                surf->verts[1].x = surf->verts[3].x = F(mapLumps->mapVerts[seg->v2].x);
                surf->verts[1].y = surf->verts[3].y = F(mapLumps->mapVerts[seg->v2].y);
                surf->verts[0].z = surf->verts[1].z = bBottom;
                surf->verts[2].z = surf->verts[3].z = bottom;

                surf->plane.SetNormal(surf->verts[0], surf->verts[1], surf->verts[2]);
                surf->plane.SetDistance(surf->verts[0]);

                surfaces.Push(surf);
            }

            bottom = bBottom;
        }

        // top seg
        if(top > bTop) {
            if(side->toptexture != -1) {
                surf = (surface_t*)Mem_Calloc(sizeof(surface_t), hb_static);
                surf->numVerts = 4;
                surf->verts = (kexVec3*)Mem_Calloc(sizeof(kexVec3) * 4, hb_static);

                surf->verts[0].x = surf->verts[2].x = F(mapLumps->mapVerts[seg->v1].x);
                surf->verts[0].y = surf->verts[2].y = F(mapLumps->mapVerts[seg->v1].y);
                surf->verts[1].x = surf->verts[3].x = F(mapLumps->mapVerts[seg->v2].x);
                surf->verts[1].y = surf->verts[3].y = F(mapLumps->mapVerts[seg->v2].y);
                surf->verts[0].z = surf->verts[1].z = top;
                surf->verts[2].z = surf->verts[3].z = bTop;

                surf->plane.SetNormal(surf->verts[0], surf->verts[1], surf->verts[2]);
                surf->plane.SetDistance(surf->verts[0]);

                surfaces.Push(surf);
            }
            
            top = bTop;
        }
    }

    // middle seg
    if(side->midtexture != -1) {
        surf = (surface_t*)Mem_Calloc(sizeof(surface_t), hb_static);
        surf->numVerts = 4;
        surf->verts = (kexVec3*)Mem_Calloc(sizeof(kexVec3) * 4, hb_static);

        surf->verts[0].x = surf->verts[2].x = F(mapLumps->mapVerts[seg->v1].x);
        surf->verts[0].y = surf->verts[2].y = F(mapLumps->mapVerts[seg->v1].y);
        surf->verts[1].x = surf->verts[3].x = F(mapLumps->mapVerts[seg->v2].x);
        surf->verts[1].y = surf->verts[3].y = F(mapLumps->mapVerts[seg->v2].y);
        surf->verts[0].z = surf->verts[1].z = top;
        surf->verts[2].z = surf->verts[3].z = bottom;

        surf->plane.SetNormal(surf->verts[0], surf->verts[1], surf->verts[2]);
        surf->plane.SetDistance(surf->verts[0]);

        surfaces.Push(surf);
    }
}

//
// Surface_BuildLeafs
//

void Surface_BuildLeafs(mapLumps_t *mapLumps, kexWadFile &wadFile) {
    lump_t          *lump;
    int             i;
    int             j;
    short           *mlf;
    int             length;
    int             size;
    int             count;

    lump = wadFile.GetLumpFromName("LEAFS");

    if(lump == NULL) {
        return;
    }

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
        }
    }

    if(count != mapLumps->numSSects) {
        Error("Surface_BuildLeafs: leaf/subsector inconsistancy %d/%d\n",
            count, mapLumps->numSSects);
    }

    if(count <= 0) {
        return;
    }

    mapLumps->leafs = (leaf_t*)Mem_Calloc((size * 2) * sizeof(leaf_t), hb_static);
    mapLumps->numLeafs = mapLumps->numSSects;

    ssLeafLookup = (int*)Mem_Calloc(sizeof(int) * mapLumps->numSSects, hb_static);
    ssLeafCount = (int*)Mem_Calloc(sizeof(int) * mapLumps->numSSects, hb_static);

    count = 0;

    int lfNum = 0;

    for(i = 0; i < mapLumps->numLeafs; i++) {
        ssLeafCount[i] = (word)*mlf++;
        ssLeafLookup[i] = lfNum;

        if(ssLeafCount[i]) {
            int vertex;
            int seg;
            
            for(j = 0; j < ssLeafCount[i]; j++, lfNum++) {
                vertex = (word)*mlf++;
                if(vertex > mapLumps->numVerts) {
                    Error("Surface_BuildLeafs: vertex out of range: %i - %i\n",
                        vertex, mapLumps->numVerts);
                }
                
                mapLumps->leafs[lfNum].vertex = &mapLumps->mapVerts[vertex];
                
                seg = *mlf++;
                if(seg == -1) {
                    mapLumps->leafs[lfNum].seg = NULL;
                }
                else {
                    if(seg > mapLumps->numSegs) {
                        Error("Surface_BuildLeafs: seg out of range: %i - %i\n",
                            seg, mapLumps->numSegs);
                    }
                    
                    mapLumps->leafs[lfNum].seg = &mapLumps->mapSegs[(word)seg];
                }
            }
        }
    }
}

//
// Surface_AllocateFromLeaf
//

void Surface_AllocateFromLeaf(mapLumps_t *mapLumps) {
    surface_t *surf;
    leaf_t *leaf;
    mapSector_t *sector = NULL;
    int i;
    int j;

    for(i = 0; i < mapLumps->numSSects; i++) {
        if(ssLeafCount[i] < 3) {
            continue;
        }

        for(j = 0; j < mapLumps->mapSSects[i].numsegs; j++) {
            mapSeg_t *seg = &mapLumps->mapSegs[mapLumps->mapSSects[i].firstseg + j];
            if(seg->side != -1) {
                mapLineDef_t *line = &mapLumps->mapLines[seg->linedef];
                mapSideDef_t *side = &mapLumps->mapSides[line->sidenum[seg->side]];
                sector = &mapLumps->mapSectors[side->sector];
                break;
            }
        }

        surf = (surface_t*)Mem_Calloc(sizeof(surface_t), hb_static);
        surf->numVerts = ssLeafCount[i];
        surf->verts = (kexVec3*)Mem_Calloc(sizeof(kexVec3) * surf->numVerts, hb_static);

        // floor verts
        for(j = 0; j < surf->numVerts; j++) {
            leaf = &mapLumps->leafs[ssLeafLookup[i] + j];

            surf->verts[j].x = F(leaf->vertex->x);
            surf->verts[j].y = F(leaf->vertex->y);
            surf->verts[j].z = sector->floorheight;
        }

        surf->plane.SetNormal(surf->verts[0], surf->verts[1], surf->verts[2]);
        surf->plane.SetDistance(surf->verts[0]);
        surfaces.Push(surf);

        surf = (surface_t*)Mem_Calloc(sizeof(surface_t), hb_static);
        surf->numVerts = ssLeafCount[i];
        surf->verts = (kexVec3*)Mem_Calloc(sizeof(kexVec3) * surf->numVerts, hb_static);

        // ceiling verts
        for(j = 0; j < surf->numVerts; j++) {
            leaf = &mapLumps->leafs[ssLeafLookup[i] + (surf->numVerts - 1) - j];

            surf->verts[j].x = F(leaf->vertex->x);
            surf->verts[j].y = F(leaf->vertex->y);
            surf->verts[j].z = sector->ceilingheight;
        }

        surf->plane.SetNormal(surf->verts[0], surf->verts[1], surf->verts[2]);
        surf->plane.SetDistance(surf->verts[0]);
        surfaces.Push(surf);
    }
}

//
// Surface_AllocateFromMap
//

void Surface_AllocateFromMap(kexWadFile &wadFile) {
    mapLumps_t      mapLumps;
    lump_t          *lump;

    lump = wadFile.GetLumpFromName("VERTEXES");
    mapLumps.mapVerts = (mapVertex_t*)wadFile.GetLumpData(lump);
    mapLumps.numVerts = lump->size / sizeof(mapVertex_t);

    lump = wadFile.GetLumpFromName("LINEDEFS");
    mapLumps.mapLines = (mapLineDef_t*)wadFile.GetLumpData(lump);
    mapLumps.numLines = lump->size / sizeof(mapLineDef_t);

    lump = wadFile.GetLumpFromName("SIDEDEFS");
    mapLumps.mapSides = (mapSideDef_t*)wadFile.GetLumpData(lump);
    mapLumps.numSides = lump->size / sizeof(mapSideDef_t);

    lump = wadFile.GetLumpFromName("SECTORS");
    mapLumps.mapSectors = (mapSector_t*)wadFile.GetLumpData(lump);
    mapLumps.numSectors = lump->size / sizeof(mapSector_t);

    lump = wadFile.GetLumpFromName("SEGS");
    mapLumps.mapSegs = (mapSeg_t*)wadFile.GetLumpData(lump);
    mapLumps.numSegs = lump->size / sizeof(mapSeg_t);

    lump = wadFile.GetLumpFromName("SSECTORS");
    mapLumps.mapSSects = (mapSubSector_t*)wadFile.GetLumpData(lump);
    mapLumps.numSSects = lump->size / sizeof(mapSubSector_t);

    for(int i = 0; i < mapLumps.numSegs; i++) {
        Surface_AllocateFromSeg(&mapLumps, &mapLumps.mapSegs[i]);
    }

#ifdef EXPORT_OBJ
    FILE *f = fopen("temp.obj", "w");
    fprintf(f, "o object1\n");
    int curLen = surfaces.Length();
    for(unsigned int i = 0; i < surfaces.Length(); i++) {
        for(int j = 0; j < surfaces[i]->numVerts; j++) {
            fprintf(f, "v %f %f %f\n",
                surfaces[i]->verts[j].x / 256.0f,
                surfaces[i]->verts[j].z / 256.0f,
                surfaces[i]->verts[j].y / 256.0f);
        }
    }

    int tri;

    for(unsigned int i = 0; i < surfaces.Length(); i++) {
        fprintf(f, "f %i %i %i\n", 0+(i*4)+1, 1+(i*4)+1, 2+(i*4)+1);
        fprintf(f, "f %i %i %i\n", 3+(i*4)+1, 2+(i*4)+1, 1+(i*4)+1);

        tri = 3+(i*4)+1;
    }

    tri++;
#endif

    Surface_BuildLeafs(&mapLumps, wadFile);
    Surface_AllocateFromLeaf(&mapLumps);

#ifdef EXPORT_OBJ
    fprintf(f, "o object2\n");

    for(unsigned int i = curLen; i < surfaces.Length(); i++) {
        for(int j = 0; j < surfaces[i]->numVerts; j++) {
            fprintf(f, "v %f %f %f\n",
                surfaces[i]->verts[j].x / 256.0f,
                surfaces[i]->verts[j].z / 256.0f,
                surfaces[i]->verts[j].y / 256.0f);
        }
    }

    for(unsigned int i = curLen; i < surfaces.Length(); i++) {
        fprintf(f, "f ");
        for(int j = 0; j < surfaces[i]->numVerts; j++) {
            fprintf(f, "%i ", tri++);
        }
        fprintf(f, "\n");
    }

    fclose(f);
#endif
}
