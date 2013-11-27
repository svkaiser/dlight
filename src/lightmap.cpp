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
#include "surfaces.h"
#include "trace.h"
#include "lightmap.h"

#define EXPORT_TEXELS_OBJ

//
// kexLightmapBuilder::kexLightmapBuilder
//

kexLightmapBuilder::kexLightmapBuilder(void) {
    this->textureWidth  = 128;
    this->textureHeight = 128;
    this->allocBlocks   = NULL;
    this->numTextures   = 0;
    this->samples       = 16;
    this->extraSamples  = 2;
}

//
// kexLightmapBuilder::~kexLightmapBuilder
//

kexLightmapBuilder::~kexLightmapBuilder(void) {
}

//
// kexLightmapBuilder::NewTexture
//

void kexLightmapBuilder::NewTexture(void) {
    numTextures++;

    if(allocBlocks == NULL) {
        allocBlocks = (int*)Mem_Calloc(sizeof(int) * textureWidth, hb_static);
    }

    memset(allocBlocks, 0, sizeof(int) * textureWidth);
}

//
// kexLightMapBuilder::MakeRoomForBlock
//

bool kexLightmapBuilder::MakeRoomForBlock(const int width, const int height,
                                          int *x, int *y) {
    int i;
    int j;
    int bestRow1;
    int bestRow2;

    if(allocBlocks == NULL) {
        return false;
    }

    bestRow1 = textureHeight;

    for(i = 0; i <= textureWidth - width; i++) {
        bestRow2 = 0;

        for(j = 0; j < width; j++) {
            if(allocBlocks[i + j] >= bestRow1) {
                break;
            }

            if(allocBlocks[i + j] > bestRow2) {
                bestRow2 = allocBlocks[i + j];
            }
        }

        // found a free block
        if(j == width) {
            *x = i;
            *y = bestRow1 = bestRow2;
        }
    }

    if(bestRow1 + height > textureHeight) {
        // no room
        return false;
    }

    for(i = 0; i < width; i++) {
        // store row offset
        allocBlocks[*x + i] = bestRow1 + height;
    }

    return true;
}

//
// kexLightmapBuilder::GetBoundsFromSurface
//

kexBBox kexLightmapBuilder::GetBoundsFromSurface(const surface_t *surface) {
    kexVec3 low(M_INFINITY, M_INFINITY, M_INFINITY);
    kexVec3 hi(-M_INFINITY, -M_INFINITY, -M_INFINITY);

    kexBBox bounds;
    bounds.Clear();

    for(int i = 0; i < surface->numVerts; i++) {
        for(int j = 0; j < 3; j++) {
            if(surface->verts[i][j] < low[j]) {
                low[j] = surface->verts[i][j];
            }
            if(surface->verts[i][j] > hi[j]) {
                hi[j] = surface->verts[i][j];
            }
        }
    }

    bounds.min = low;
    bounds.max = hi;

    return bounds;
}

//
// kexLightmapBuilder::BuildSurfaceParams
//

void kexLightmapBuilder::BuildSurfaceParams(surface_t *surface) {
    kexPlane *plane;
    kexBBox bounds;
    kexVec3 roundedSize;
    int i;
    lightmapAxis_t axis;
    kexVec3 tCoords[2];
    kexVec3 tNormal;
    kexVec3 tDelta;
    kexVec3 tOrigin;
    int width;
    int height;
    int x;
    int y;
    float d;

    plane = &surface->plane;
    bounds = GetBoundsFromSurface(surface);

    // round off dimentions
    for(i = 0; i < 3; i++) {
        bounds.min[i] = samples * kexMath::Floor(bounds.min[i] / samples);
        bounds.max[i] = samples * kexMath::Ceil(bounds.max[i] / samples);

        roundedSize[i] = (bounds.max[i] - bounds.min[i]) / samples + 1;
    }

    tCoords[0].Clear();
    tCoords[1].Clear();

    tNormal.Set(
        kexMath::Fabs(plane->a),
        kexMath::Fabs(plane->b),
        kexMath::Fabs(plane->c));

    // figure out what axis the plane lies on
    if(tNormal.x >= tNormal.y && tNormal.x >= tNormal.z) {
        axis = AXIS_YZ;
    }
    else if(tNormal.y >= tNormal.x && tNormal.y >= tNormal.z) {
        axis = AXIS_XZ;
    }
    else {
        axis = AXIS_XY;
    }

    switch(axis) {
        case AXIS_YZ:
            width = (int)roundedSize.y;
            height = (int)roundedSize.z;
            tCoords[0].y = 1.0f / samples;
            tCoords[1].z = 1.0f / samples;
            break;
        case AXIS_XZ:
            width = (int)roundedSize.x;
            height = (int)roundedSize.z;
            tCoords[0].x = 1.0f / samples;
            tCoords[1].z = 1.0f / samples;
            break;
        case AXIS_XY:
            width = (int)roundedSize.x;
            height = (int)roundedSize.y;
            tCoords[0].x = 1.0f / samples;
            tCoords[1].y = 1.0f / samples;
            break;
    }

    // clamp width
    if(width > textureWidth) {
        tCoords[0] *= ((float)textureWidth / (float)width);
        width = textureWidth;
    }

    // clamp height
    if(height > textureHeight) {
        tCoords[1] *= ((float)textureHeight / (float)height);
        height = textureHeight;
    }

    // make room for new lightmap block
    if(!MakeRoomForBlock(width, height, &x, &y)) {
        NewTexture();
        if(!MakeRoomForBlock(width, height, &x, &y)) {
            Error("Lightmap allocation failed\n");
            return;
        }
    }

    surface->lightmapCoords = (float*)Mem_Calloc(sizeof(float) *
        surface->numVerts * 2, hb_static);

    // calculate texture coordinates
    for(i = 0; i < surface->numVerts; i++) {
        tDelta = surface->verts[i] - bounds.min;
        surface->lightmapCoords[i + 0 * 2] =
            (tDelta.Dot(tCoords[0]) + x + 0.5f) / (float)textureWidth;
        surface->lightmapCoords[i + 1 * 2] =
            (tDelta.Dot(tCoords[1]) + y + 0.5f) / (float)textureHeight;
    }

    tOrigin = bounds.min;

    // project tOrigin and tCoords so they lie on the plane
    d = (plane->Distance(bounds.min) - plane->d) / plane->Normal()[axis];
    tOrigin[axis] -= d;

    for(i = 0; i < 2; i++) {
        tCoords[i].Normalize();
        d = plane->Distance(tCoords[i]) / plane->Normal()[axis];
        tCoords[i][axis] -= d;
    }

    surface->lightmapNum = numTextures - 1;
    surface->lightmapDims[0] = width;
    surface->lightmapDims[1] = height;
    surface->lightmapOffs[0] = x;
    surface->lightmapOffs[1] = y;
    surface->lightmapOrigin = tOrigin;
    surface->lightmapSteps[0] = tCoords[0] * (float)samples;
    surface->lightmapSteps[1] = tCoords[1] * (float)samples;
}

//
// kexLightmapBuilder::TraceSurface
//

void kexLightmapBuilder::TraceSurface(surface_t *surface) {
    kexVec3 **colorSamples;
    int sampleWidth;
    int sampleHeight;
    kexVec3 normal;
    kexVec3 pos;
    int i;
    int j;

    sampleWidth = textureWidth * extraSamples;
    sampleHeight = textureHeight * extraSamples;

    colorSamples = (kexVec3**)Mem_Alloca(sizeof(kexVec3*) * sampleWidth);

    for(i = 0; i < textureWidth * extraSamples; i++) {
        colorSamples[i] = (kexVec3*)Mem_Alloca(sizeof(kexVec3) * sampleHeight);
    }

    sampleWidth = surface->lightmapDims[0];
    sampleHeight = surface->lightmapDims[1];

    normal = surface->plane.Normal();

#ifdef EXPORT_TEXELS_OBJ
    FILE *f = fopen("texels.obj", "w");
    int indices = 0;
#endif

    for(i = 0; i < sampleHeight; i++) {
        for(j = 0; j < sampleWidth; j++) {
            pos.x = surface->lightmapOrigin.x + normal.x +
                j * surface->lightmapSteps[0].x +
                i * surface->lightmapSteps[1].x;
            pos.y = surface->lightmapOrigin.y + normal.y +
                j * surface->lightmapSteps[0].y +
                i * surface->lightmapSteps[1].y;
            pos.z = surface->lightmapOrigin.z + normal.z +
                j * surface->lightmapSteps[0].z +
                i * surface->lightmapSteps[1].z;

#ifdef EXPORT_TEXELS_OBJ
            ExportTexelsToObjFile(f, pos, indices);
            indices += 8;
#endif

            // TODO
        }
    }

#ifdef EXPORT_TEXELS_OBJ
    fclose(f);
#endif

    // TODO

    Mem_GC();
}

//
// kexLightmapBuilder::ExportTexelsToObjFile
//

void kexLightmapBuilder::ExportTexelsToObjFile(FILE *f, const kexVec3 &org, int indices) {
#define BLOCK(idx, offs) (org[idx] + offs)/256.0f
    fprintf(f, "o texel_%03d\n", indices);
    fprintf(f, "v %f %f %f\n", -BLOCK(1, -6), BLOCK(2, -6), -BLOCK(0, 6));
    fprintf(f, "v %f %f %f\n", -BLOCK(1, -6), BLOCK(2, 6), -BLOCK(0, 6));
    fprintf(f, "v %f %f %f\n", -BLOCK(1, -6), BLOCK(2, 6), -BLOCK(0, -6));
    fprintf(f, "v %f %f %f\n", -BLOCK(1, -6), BLOCK(2, -6), -BLOCK(0, -6));
    fprintf(f, "v %f %f %f\n", -BLOCK(1, 6), BLOCK(2, -6), -BLOCK(0, 6));
    fprintf(f, "v %f %f %f\n", -BLOCK(1, 6), BLOCK(2, 6), -BLOCK(0, 6));
    fprintf(f, "v %f %f %f\n", -BLOCK(1, 6), BLOCK(2, 6), -BLOCK(0, -6));
    fprintf(f, "v %f %f %f\n", -BLOCK(1, 6), BLOCK(2, -6), -BLOCK(0, -6));
    fprintf(f, "f %i %i %i %i\n", indices+1, indices+2, indices+3, indices+4);
    fprintf(f, "f %i %i %i %i\n", indices+5, indices+8, indices+7, indices+6);
    fprintf(f, "f %i %i %i %i\n", indices+1, indices+5, indices+6, indices+2);
    fprintf(f, "f %i %i %i %i\n", indices+2, indices+6, indices+7, indices+3);
    fprintf(f, "f %i %i %i %i\n", indices+3, indices+7, indices+8, indices+4);
    fprintf(f, "f %i %i %i %i\n\n", indices+5, indices+1, indices+4, indices+8);
#undef BLOCK
}
