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

#ifndef __LIGHTMAP_H__
#define __LIGHTMAP_H__

#include "surfaces.h"

typedef enum {
    AXIS_YZ     = 0,
    AXIS_XZ,
    AXIS_XY
} lightmapAxis_t;

class kexLightmapBuilder {
public:
                        kexLightmapBuilder(void);
                        ~kexLightmapBuilder(void);

    void                NewTexture(void);
    bool                MakeRoomForBlock(const int width, const int height, int *x, int *y);

private:
    kexBBox             GetBoundsFromSurface(const surface_t *surface);
    void                BuildSurfaceParams(surface_t *surface);
    void                TraceSurface(surface_t *surface);

    kexArray<byte*>     textures;
    int                 textureWidth;
    int                 textureHeight;
    int                 *allocBlocks;
    int                 numTextures;
    int                 samples;
    int                 extraSamples;
};

#endif
