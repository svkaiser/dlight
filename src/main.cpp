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
// DESCRIPTION: Main application
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "wad.h"
#include "mapData.h"
#include "surfaces.h"
#include "trace.h"
#include "lightmap.h"

//
// isDigit
//

static bool isDigit(int x) {
    return ((x) >= '0') && ((x) <= '9');
}

//
// isSpace
//

static bool isSpace(int x) {
    return
        ((x) == ' ') ||
        ((x) == '\t') ||
        ((x) == '\r') ||
        ((x) == '\n') ||
        ((x) == '\f') ||
        ((x) == '\v');
}

//
// Error
//

void Error(char *error, ...) {
    va_list argptr;

    va_start(argptr,error);
    vprintf(error,argptr);
    va_end(argptr);
    printf("\n");
    exit(1);
}

//
// Va
//

char *Va(char *str, ...) {
    va_list v;
    static char vastr[1024];
	
    va_start(v, str);
    vsprintf(vastr, str,v);
    va_end(v);
    
    return vastr;	
}

//
// Main
//

int main(int argc, char **argv) {
    kexWadFile wadFile;
    kexDoomMap doomMap;
    kexLightmapBuilder builder;
    int size;

    if(!argv[1]) {
        return 0;
    }

    if(!wadFile.Open(argv[1])) {
        return 1;
    }

    printf("------------- Building level structures -------------\n\n");
    doomMap.BuildMapFromWad(wadFile);

    printf("------------- Allocating surfaces from level -------------\n\n");
    Surface_AllocateFromMap(doomMap);

    printf("------------- Creating lightmaps -------------\n\n");
    builder.CreateLightmaps(doomMap);
    builder.WriteTexturesToTGA();
    byte *lm = builder.CreateLightmapLump(&size);

    FILE *f = fopen("LIGHTMAP.lmp", "wb");
    fwrite(lm, 1, size, f);
    fclose(f);

    printf("Done\n\n");

    wadFile.Close();
    Mem_Purge(hb_static);

    return 0;
}
