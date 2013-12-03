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

#ifndef __WAD_H__
#define __WAD_H__

#include "kexlib/binFile.h"

typedef enum {
    ML_HEADER               = 0,
    ML_THINGS,
    ML_LINEDEFS,
    ML_SIDEDEFS,
    ML_VERTEXES,
    ML_SEGS,
    ML_SUBSECTORS,
    ML_NODES,
    ML_SECTORS,
    ML_REJECT,
    ML_BLOCKMAP,
    ML_LEAFS,
    ML_LIGHTS,
    ML_MACROS,
    ML_LIGHTMAP,
    ML_NUMLUMPS
} mapLumps_t;

typedef struct {
    char                id[4];
    int                 lmpcount;
    int                 lmpdirpos;
} wadHeader_t;

typedef struct {
    int                 filepos;
    int                 size;
    char                name[8];
} lump_t;

class kexWadFile {
public:
                        ~kexWadFile(void);

    wadHeader_t         header;
    lump_t              *lumps;
    unsigned int        size;
    int                 mapLumpID;
    kexStr              wadName;

    lump_t              *GetLumpFromName(const char *name);
    lump_t              *GetMapLump(mapLumps_t lumpID);
    byte                *GetLumpData(const lump_t *lump);
    byte                *GetLumpData(const char *name);
    void                SetCurrentMap(const int map);    
    bool                Open(const char *fileName);
    void                Close(void);
    void                BuildNewWad(byte *lightmapLump, const int size);

    template<typename type>
    void                GetMapLump(mapLumps_t lumpID, type **ptr, int *count) {
        if(mapLumpID + lumpID >= header.lmpcount) {
            return;
        }
        lump_t *lump = GetMapLump(lumpID);
        if(lump == NULL) {
            return;
        }
        *ptr = (type*)GetLumpData(lump);
        *count = lump->size / sizeof(type);
    }

private:
    kexBinFile          file;
};

#endif
