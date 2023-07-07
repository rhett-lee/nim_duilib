#ifndef UI_RENDER_RENDER_CONFIG_H_
#define UI_RENDER_RENDER_CONFIG_H_

#pragma once

namespace ui
{

/** Skia引擎
*/
#define duilib_kRenderType_Skia     0

/** GdiPlus引擎
*/
#define duilib_kRenderType_GdiPlus  1

/** 当前选择的渲染引擎(通过修改此值，来设置支持哪个引擎)
*/
#define duilib_kRenderType          duilib_kRenderType_Skia

}//namespace ui

#endif // UI_RENDER_RENDER_CONFIG_H_
