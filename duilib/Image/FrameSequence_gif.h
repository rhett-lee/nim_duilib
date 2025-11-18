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

#ifndef UI_IMAGE_FRAMESQUENCE_GIF_H
#define UI_IMAGE_FRAMESQUENCE_GIF_H

#include "duilib/third_party/giflib/gif_lib.h"
#include <cstdint>

namespace ui
{

typedef uint32_t Color8888;

class FrameSequence_gif{
public:
    FrameSequence_gif();
    ~FrameSequence_gif();

    bool Init(GifFileType* gif);

    void Clear();

    int GetWidth() const {
        return mGif ? mGif->SWidth : 0;
    }

    int GetHeight() const {
        return mGif ? mGif->SHeight : 0;
    }

    bool IsOpaque() const;

    int GetFrameCount() const {
        return mGif ? mGif->ImageCount : 0;
    }

    int GetDefaultLoopCount() const {
        return mLoopCount;
    }

    bool DrawFrame(int frameNr, Color8888* outputPtr, int outputPixelStride, int previousFrameNr);

private:
    GifFileType* getGif() const { return mGif; }
    Color8888 getBackgroundColor() const { return mBgColor; }
    bool getPreservedFrame(int frameIndex) const { return mPreservedFrames[frameIndex]; }
    int getRestoringFrame(int frameIndex) const { return mRestoringFrames[frameIndex]; }

private:
    void savePreserveBuffer(Color8888* outputPtr, int outputPixelStride, int frameNr);
    void restorePreserveBuffer(Color8888* outputPtr, int outputPixelStride);

private:
    GifFileType* mGif;
    int mLoopCount;
    Color8888 mBgColor;

    // array of bool per frame - if true, frame data is used by a later DISPOSE_PREVIOUS frame
    bool* mPreservedFrames;

    // array of ints per frame - if >= 0, points to the index of the preserve that frame needs
    int* mRestoringFrames;

private:
    Color8888* mPreserveBuffer;
    int mPreserveBufferFrame;
};

} //namespace ui

#endif //UI_IMAGE_FRAMESQUENCE_GIF_H
