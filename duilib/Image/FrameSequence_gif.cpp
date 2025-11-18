/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "FrameSequence_gif.h"
#include <string.h>
#include <algorithm>

namespace ui
{

static const Color8888 COLOR_8888_ALPHA_MASK = 0xff000000; // TODO: handle endianness
static const Color8888 TRANSPARENT = 0x0;

// TODO: handle endianness
#define ARGB_TO_COLOR8888(a, r, g, b) \
    ((a) << 24 | (b) << 16 | (g) << 8 | (r))

#define GIF_DEBUG 0

static Color8888 gifColorToColor8888(const GifColorType& color) {
    return ARGB_TO_COLOR8888(0xff, color.Red, color.Green, color.Blue);
}

static long getDelayMs(GraphicsControlBlock& gcb) {
    return gcb.DelayTime * 10;
}

static bool willBeCleared(const GraphicsControlBlock& gcb) {
    return gcb.DisposalMode == DISPOSE_BACKGROUND || gcb.DisposalMode == DISPOSE_PREVIOUS;
}

////////////////////////////////////////////////////////////////////////////////
// Frame sequence
////////////////////////////////////////////////////////////////////////////////

FrameSequence_gif::FrameSequence_gif() :
    mLoopCount(-1), mBgColor(TRANSPARENT), mPreservedFrames(NULL), mRestoringFrames(NULL),
    mPreserveBuffer(NULL), mPreserveBufferFrame(-1) {
}

bool FrameSequence_gif::Init(GifFileType* gif)
{
    Clear();
    mGif = gif;
    if (!mGif) {
        return false;
    }

    if (DGifSlurp(mGif) != GIF_OK) {
        mGif = NULL;
        return false;
    }

    long durationMs = 0;
    int lastUnclearedFrame = -1;
    mPreservedFrames = new bool[mGif->ImageCount];
    mRestoringFrames = new int[mGif->ImageCount];

    GraphicsControlBlock gcb;
    for (int i = 0; i < mGif->ImageCount; i++) {
        const SavedImage& image = mGif->SavedImages[i];

        // find the loop extension pair
        for (int j = 0; (j + 1) < image.ExtensionBlockCount; j++) {
            ExtensionBlock* eb1 = image.ExtensionBlocks + j;
            ExtensionBlock* eb2 = image.ExtensionBlocks + j + 1;
            if (eb1->Function == APPLICATION_EXT_FUNC_CODE
                    // look for "NETSCAPE2.0" app extension
                    && eb1->ByteCount == 11
                    && !memcmp((const char*)(eb1->Bytes), "NETSCAPE2.0", 11)
                    // verify extension contents and get loop count
                    && eb2->Function == CONTINUE_EXT_FUNC_CODE
                    && eb2->ByteCount == 3
                    && eb2->Bytes[0] == 1) {
                mLoopCount = (int)(eb2->Bytes[2] << 8) + (int)(eb2->Bytes[1]);
            }
        }

        DGifSavedExtensionToGCB(mGif, i, &gcb);

        // timing
        durationMs += getDelayMs(gcb);

        // preserve logic
        mPreservedFrames[i] = false;
        mRestoringFrames[i] = -1;
        if (gcb.DisposalMode == DISPOSE_PREVIOUS && lastUnclearedFrame >= 0) {
            mPreservedFrames[lastUnclearedFrame] = true;
            mRestoringFrames[i] = lastUnclearedFrame;
        }
        if (!willBeCleared(gcb)) {
            lastUnclearedFrame = i;
        }
    }

    const ColorMapObject* cmap = mGif->SColorMap;
    if (cmap) {
        // calculate bg color
        GraphicsControlBlock gcb2;
        DGifSavedExtensionToGCB(mGif, 0, &gcb2);
        if (gcb2.TransparentColor == NO_TRANSPARENT_COLOR
                && mGif->SBackGroundColor < cmap->ColorCount) {
            mBgColor = gifColorToColor8888(cmap->Colors[mGif->SBackGroundColor]);
        }
    }
    return true;
}

void FrameSequence_gif::Clear()
{
    mGif = nullptr;
    if (mPreservedFrames != nullptr) {
        delete[] mPreservedFrames;
        mPreservedFrames = nullptr;
    }
    if (mRestoringFrames != nullptr) {
        delete[] mRestoringFrames;
        mRestoringFrames = nullptr;
    }
    
    if (mPreserveBuffer != nullptr) {
        delete[] mPreserveBuffer;
        mPreserveBuffer = nullptr;
    }    
}

bool FrameSequence_gif::IsOpaque() const {
    return (mBgColor & COLOR_8888_ALPHA_MASK) == COLOR_8888_ALPHA_MASK;
}

FrameSequence_gif::~FrameSequence_gif()
{
    Clear();
}

////////////////////////////////////////////////////////////////////////////////
// draw helpers
////////////////////////////////////////////////////////////////////////////////

// return true if area of 'target' is completely covers area of 'covered'
static bool checkIfCover(const GifImageDesc& target, const GifImageDesc& covered) {
    return target.Left <= covered.Left
            && covered.Left + covered.Width <= target.Left + target.Width
            && target.Top <= covered.Top
            && covered.Top + covered.Height <= target.Top + target.Height;
}

static void copyLine(Color8888* dst, const unsigned char* src, const ColorMapObject* cmap,
                     int transparent, int width) {
    for (; width > 0; width--, src++, dst++) {
        if (*src != transparent && *src < cmap->ColorCount) {
            *dst = gifColorToColor8888(cmap->Colors[*src]);
        }
    }
}

static void setLineColor(Color8888* dst, Color8888 color, int width) {
    for (; width > 0; width--, dst++) {
        *dst = color;
    }
}

static void getCopySize(const GifImageDesc& imageDesc, int maxWidth, int maxHeight,
        GifWord& copyWidth, GifWord& copyHeight) {
    copyWidth = imageDesc.Width;
    if (imageDesc.Left + copyWidth > maxWidth) {
        copyWidth = maxWidth - imageDesc.Left;
    }
    copyHeight = imageDesc.Height;
    if (imageDesc.Top + copyHeight > maxHeight) {
        copyHeight = maxHeight - imageDesc.Top;
    }
}

void FrameSequence_gif::savePreserveBuffer(Color8888* outputPtr, int outputPixelStride, int frameNr) {
    if (frameNr == mPreserveBufferFrame) return;

    mPreserveBufferFrame = frameNr;
    const int width = GetWidth();
    const int height = GetHeight();
    if (!mPreserveBuffer) {
        mPreserveBuffer = new Color8888[width * height];
    }
    for (int y = 0; y < height; y++) {
        memcpy(mPreserveBuffer + width * y,
                outputPtr + outputPixelStride * y,
                width * 4);
    }
}

void FrameSequence_gif::restorePreserveBuffer(Color8888* outputPtr, int outputPixelStride) {
    const int width = GetWidth();
    const int height = GetHeight();
    if (!mPreserveBuffer) {
        return;
    }
    for (int y = 0; y < height; y++) {
        memcpy(outputPtr + outputPixelStride * y,
                mPreserveBuffer + width * y,
                width * 4);
    }
}

bool FrameSequence_gif::DrawFrame(int frameNr, Color8888* outputPtr, int outputPixelStride, int previousFrameNr)
{
    GifFileType* gif = getGif();
    if (!gif) {
        return false;
    }

    const int height = GetHeight();
    const int width = GetWidth();

    GraphicsControlBlock gcb;

    int start = std::max(previousFrameNr + 1, 0);

    for (int i = std::max(start - 1, 0); i < frameNr; i++) {
        int neededPreservedFrame = getRestoringFrame(i);
        if (neededPreservedFrame >= 0 && (mPreserveBufferFrame != neededPreservedFrame)) {
            start = 0;
        }
    }

    for (int i = start; i <= frameNr; i++) {
        DGifSavedExtensionToGCB(gif, i, &gcb);
        const SavedImage& frame = gif->SavedImages[i];

        if (i == 0) {
            //clear bitmap
            Color8888 bgColor = getBackgroundColor();
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    outputPtr[y * outputPixelStride + x] = bgColor;
                }
            }
        } else {
            GraphicsControlBlock prevGcb;
            DGifSavedExtensionToGCB(gif, i - 1, &prevGcb);
            const SavedImage& prevFrame = gif->SavedImages[i - 1];
            bool prevFrameDisposed = willBeCleared(prevGcb);

            bool newFrameOpaque = gcb.TransparentColor == NO_TRANSPARENT_COLOR;
            bool prevFrameCompletelyCovered = newFrameOpaque
                    && checkIfCover(frame.ImageDesc, prevFrame.ImageDesc);

            if (prevFrameDisposed && !prevFrameCompletelyCovered) {
                switch (prevGcb.DisposalMode) {
                case DISPOSE_BACKGROUND: {
                    Color8888* dst = outputPtr + prevFrame.ImageDesc.Left +
                            prevFrame.ImageDesc.Top * outputPixelStride;

                    GifWord copyWidth, copyHeight;
                    getCopySize(prevFrame.ImageDesc, width, height, copyWidth, copyHeight);
                    for (; copyHeight > 0; copyHeight--) {
                        setLineColor(dst, TRANSPARENT, copyWidth);
                        dst += outputPixelStride;
                    }
                } break;
                case DISPOSE_PREVIOUS: {
                    restorePreserveBuffer(outputPtr, outputPixelStride);
                } break;
                }
            }

            if (getPreservedFrame(i - 1)) {
                // currently drawn frame will be restored by a following DISPOSE_PREVIOUS draw, so
                // we preserve it
                savePreserveBuffer(outputPtr, outputPixelStride, i - 1);
            }
        }

        bool willBeCleared = gcb.DisposalMode == DISPOSE_BACKGROUND
                || gcb.DisposalMode == DISPOSE_PREVIOUS;
        if (i == frameNr || !willBeCleared) {
            const ColorMapObject* cmap = gif->SColorMap;
            if (frame.ImageDesc.ColorMap) {
                cmap = frame.ImageDesc.ColorMap;
            }

            // If a cmap is missing, the frame can't be decoded, so we skip it.
            if (cmap) {
                const unsigned char* src = (unsigned char*)frame.RasterBits;
                Color8888* dst = outputPtr + frame.ImageDesc.Left +
                        frame.ImageDesc.Top * outputPixelStride;
                GifWord copyWidth, copyHeight;
                getCopySize(frame.ImageDesc, width, height, copyWidth, copyHeight);
                for (; copyHeight > 0; copyHeight--) {
                    copyLine(dst, src, cmap, gcb.TransparentColor, copyWidth);
                    src += frame.ImageDesc.Width;
                    dst += outputPixelStride;
                }
            }
        }
    }
    return true;
}

} //namespace ui
