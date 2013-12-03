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
// DESCRIPTION: General wad loading mechanics
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "wad.h"

//
// kexWadFile::~kexWadFile
//

kexWadFile::~kexWadFile(void) {
    Close();
}

//
// kexWadFile::Open
//

bool kexWadFile::Open(const char *fileName) {
    if(!file.Open(fileName)) {
        return false;
    }

    memcpy(&header, file.Buffer(), sizeof(wadHeader_t));
    lumps = (lump_t*)file.GetOffset(2);
    wadName = fileName;
    return true;
}

//
// kexWadFile::Close
//

void kexWadFile::Close(void) {
    if(file.Opened()) {
        file.Close();
    }
}

//
// kexWadFile::GetLumpFromName
//

lump_t *kexWadFile::GetLumpFromName(const char *name) {
    char n[9];

    for(int i = 0; i < header.lmpcount; i++) {
        // could be optimized here but I really don't feel like
        // wasting time on this
        strncpy(n, lumps[i].name, 8);
        n[8] = 0;

        if(!strncmp(n, name, 8)) {
            return &lumps[i];
        }
    }

    return NULL;
}

//
// kexWadFile::GetLumpData
//

byte *kexWadFile::GetLumpData(const lump_t *lump) {
    file.SetOffset(lump->filepos);
    return &file.Buffer()[lump->filepos];
}

//
// kexWadFile::GetLumpData
//

byte *kexWadFile::GetLumpData(const char *name) {
    lump_t *lump = GetLumpFromName(name);

    if(!lump) {
        return NULL;
    }

    file.SetOffset(lump->filepos);
    return &file.Buffer()[lump->filepos];
}

//
// kexWadFile::SetCurrentMap
//

void kexWadFile::SetCurrentMap(const int map) {
    lump_t *lump = GetLumpFromName(Va("MAP%02d", map));
    
    if(lump == NULL) {
        Error("kexWadFile::SetCurrentMap: MAP%02d not found\n", map);
        return;
    }
    
    mapLumpID = lump - lumps;
}

//
// kexWadFile::GetMapLump
//

lump_t *kexWadFile::GetMapLump(mapLumps_t lumpID) {
    if(mapLumpID + lumpID >= header.lmpcount) {
        return NULL;
    }

    return &lumps[mapLumpID + lumpID];
}

//
// kexWadFile::BuildNewWad
//

void kexWadFile::BuildNewWad(byte *lightmapLump, const int size) {
    wadHeader_t newHeader;
    lump_t lump;
    lump_t *lmLump;
    kexArray<lump_t> lumpList;
    kexArray<byte*> dataList;
    int pos;
    kexBinFile wadFile;
    kexStr backupName;

    newHeader.id[0] = 'P';
    newHeader.id[1] = 'W';
    newHeader.id[2] = 'A';
    newHeader.id[3] = 'D';

    pos = sizeof(wadHeader_t);

    newHeader.lmpcount = 0;
    newHeader.lmpdirpos = pos;

    lmLump = GetMapLump(ML_LIGHTMAP);

    for(int i = 0; i < header.lmpcount; i++) {
        // don't re-add old lightmap lump
        if(lmLump != NULL && i == (mapLumpID + ML_LIGHTMAP)) {
            continue;
        }

        lump = lumps[i];
        lump.filepos = pos;
        lumpList.Push(lump);
        dataList.Push(GetLumpData(lumps[i].name));

        newHeader.lmpcount++;
        newHeader.lmpdirpos += lump.size;
        pos += lump.size;

        if(i == (mapLumpID + ML_MACROS)) {
            lump.filepos = pos;
            lump.size = size;
            strncpy(lump.name, "LIGHTMAP", 8);
            lumpList.Push(lump);
            dataList.Push(lightmapLump);

            newHeader.lmpcount++;
            newHeader.lmpdirpos += lump.size;
            pos += lump.size;
        }
    }

    backupName = wadName;
    backupName.StripExtension();
    wadFile.Duplicate(backupName + "_backup.wad");

    wadFile.Create(wadName);
    wadFile.Write8(newHeader.id[0]);
    wadFile.Write8(newHeader.id[1]);
    wadFile.Write8(newHeader.id[2]);
    wadFile.Write8(newHeader.id[3]);
    wadFile.Write32(newHeader.lmpcount);
    wadFile.Write32(newHeader.lmpdirpos);

    for(unsigned int i = 0; i < lumpList.Length(); i++) {
        byte *data = dataList[i];

        for(int j = 0; j < lumpList[i].size; j++) {
            wadFile.Write8(data[j]);
        }
    }

    for(unsigned int i = 0; i < lumpList.Length(); i++) {
        wadFile.Write32(lumpList[i].filepos);
        wadFile.Write32(lumpList[i].size);
        for(int j = 0; j < 8; j++) {
            wadFile.Write8(lumpList[i].name[j]);
        }
    }

    wadFile.Close();
}
