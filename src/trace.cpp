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
#include "mapData.h"
#include "trace.h"

//
// kexTrace::kexTrace
//

kexTrace::kexTrace(void) {
    this->map = NULL;
}

//
// kexTrace::~kexTrace
//

kexTrace::~kexTrace(void) {
}

//
// kexTrace::Init
//

void kexTrace::Init(kexDoomMap &doomMap) {
    map = &doomMap;
}

//
// kexTrace::CheckPosition
//

void kexTrace::CheckPosition(const kexVec3 position) {
    start = position;
    end.Clear();
    dir.Clear();
    hitNormal.Clear();
    hitVector.Clear();
    fraction = 1;

    if(map == NULL) {
        return;
    }

    RecursiveCheckPosition(map->numNodes - 1);
}

//
// kexTrace::RecursiveCheckPosition
//

void kexTrace::RecursiveCheckPosition(int num) {
    mapNode_t *node;

    if(num & NF_SUBSECTOR) {
        int subNum = num & (~NF_SUBSECTOR);

        surface_t *floorSurface = map->leafSurfaces[0][subNum];
        surface_t *ceilingSurface = map->leafSurfaces[1][subNum];

        // TODO

        fraction = 0;
        hitVector = start;
        return;
    }

    node = &map->nodes[num];

    kexVec3 pt1(F(node->x << 16), F(node->y << 16), 0);
    kexVec3 pt2(F(node->dx << 16), F(node->dy << 16), 0);

    kexVec3 dp1 = pt1 - start;
    kexVec3 dp2 = (pt2 + pt1) - start;

    // determine side from the z component
    float d = dp1.Cross(dp2).z;

    RecursiveCheckPosition(node->children[FLOATSIGNBIT(d)]);

    if(d == 0) {
        RecursiveCheckPosition(node->children[FLOATSIGNBIT(d) ^ 1]);
    }
}
