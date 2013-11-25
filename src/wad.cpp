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
    lumpcache = (byte**)Mem_Calloc(sizeof(byte*) * header.lmpcount, hb_static);

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
