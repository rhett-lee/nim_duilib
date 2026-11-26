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

/** 32位颜色值，格式为 0xAARRGGBB（A 为 Alpha，R/G/B 为各颜色分量）
 */
typedef uint32_t Color8888;

/** GIF 动画帧序列的解码与绘制辅助类，功能要点如下：
 *  1. 初始化时解析 GIF 数据，并缓存每帧的处置方式（Disposal）与循环次数
 *  2. 支持将指定帧绘制到外部提供的像素缓冲区，含透明色处理与帧间内容恢复
 *  3. 提供画面宽度、高度、帧总数、循环次数等基础信息
 *  说明：本类不负责释放外部传入的 GifFileType 对象
 */
class FrameSequence_gif{
public:
    FrameSequence_gif();
    ~FrameSequence_gif();

    /** 初始化：解析传入的 GIF 数据并缓存各帧信息
    * @param [in] gif 已经打开的 GIF 文件对象，本类不负责释放该对象
    * @return 返回 true 表示解析成功，否则返回 false
    */
    bool Init(GifFileType* gif);

    /** 清空已解析的帧缓存与内部状态（不释放传入的 GifFileType 对象）
    */
    void Clear();

    /** 获取 GIF 画面的宽度（像素）
    * @return 返回画面宽度，未初始化时返回0
    */
    int GetWidth() const {
        return mGif ? mGif->SWidth : 0;
    }

    /** 获取 GIF 画面的高度（像素）
    * @return 返回画面高度，未初始化时返回0
    */
    int GetHeight() const {
        return mGif ? mGif->SHeight : 0;
    }

    /** 判断 GIF 背景色是否为完全不透明
    * @return 返回 true 表示背景完全不透明，否则返回 false
    */
    bool IsOpaque() const;

    /** 获取 GIF 的帧总数
    * @return 返回帧总数，未初始化时返回0
    */
    int GetFrameCount() const {
        return mGif ? mGif->ImageCount : 0;
    }

    /** 获取 GIF 中配置的循环次数
    * @return 返回循环次数；-1 表示 GIF 中未指定循环次数
    */
    int GetDefaultLoopCount() const {
        return mLoopCount;
    }

    /** 绘制指定帧到像素缓冲区
    * @param [in] frameNr 要绘制的帧索引，范围：[0, GetFrameCount())
    * @param [out] outputPtr 输出像素缓冲区，尺寸需能容纳 width * height 个像素
    * @param [in] outputPixelStride 输出缓冲区每行的像素跨度（相邻两行起始位置的像素距离）
    * @param [in] previousFrameNr 上一次绘制的帧索引，-1 表示首次绘制（用于增量绘制与帧恢复）
    * @return 返回 true 表示绘制成功，否则返回 false
    */
    bool DrawFrame(int frameNr, Color8888* outputPtr, int outputPixelStride, int previousFrameNr);

private:
    /** 获取关联的 GIF 文件对象
    */
    GifFileType* getGif() const { return mGif; }

    /** 获取 GIF 的背景色
    */
    Color8888 getBackgroundColor() const { return mBgColor; }

    /** 判断指定帧的数据是否需要保留（供后续帧恢复使用）
    */
    bool getPreservedFrame(int frameIndex) const { return mPreservedFrames[frameIndex]; }

    /** 获取指定帧需要恢复的保留帧索引，-1 表示无需恢复
    */
    int getRestoringFrame(int frameIndex) const { return mRestoringFrames[frameIndex]; }

private:
    /** 保存当前输出缓冲区的内容到内部保留缓冲区
    * @param [in] outputPtr 输出像素缓冲区
    * @param [in] outputPixelStride 输出缓冲区每行的像素跨度
    * @param [in] frameNr 当前帧索引
    */
    void savePreserveBuffer(Color8888* outputPtr, int outputPixelStride, int frameNr);

    /** 将内部保留缓冲区的内容恢复到输出缓冲区
    * @param [out] outputPtr 输出像素缓冲区
    * @param [in] outputPixelStride 输出缓冲区每行的像素跨度
    */
    void restorePreserveBuffer(Color8888* outputPtr, int outputPixelStride);

private:
    GifFileType* mGif;
    int mLoopCount;
    Color8888 mBgColor;

    // 每帧一个 bool 值：为 true 表示该帧的数据会被后续 DISPOSE_PREVIOUS 帧使用（需要保留）
    bool* mPreservedFrames;

    // 每帧一个 int 值：值 >= 0 时表示该帧需要恢复的保留帧索引
    int* mRestoringFrames;

private:
    Color8888* mPreserveBuffer;
    int mPreserveBufferFrame;
};

} //namespace ui

#endif //UI_IMAGE_FRAMESQUENCE_GIF_H
