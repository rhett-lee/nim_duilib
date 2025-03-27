#ifndef EXAMPLES_HBITMAP_CONTROL_H_
#define EXAMPLES_HBITMAP_CONTROL_H_

// duilib
#include "duilib/duilib.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

/** @class HBitmapControl
  * @brief 把HBITMAP作为背景的控件,控件不负责销毁HBITMAP
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @author Redrain
  * @date 2019/3/20
  */
class HBitmapControl : public ui::Control
{
    typedef ui::Control BaseClass;
public:
    explicit HBitmapControl(ui::Window* pWindow);

    /**
    * 重写父控件绘制函数
    * @param[in] hDC 目标DC
    * @param[in] rcPaint 可绘制区域
    * @return void    无返回值
    */
    virtual void Paint(ui::IRender* pRender, const ui::UiRect& rcPaint) override;

    /**
    * 设置显示在背景的HBITMAP对象
    * @param[in] bitmap 位图对象
    * @return void    无返回值
    */
    void SetBitmapImage(HBITMAP bitmap);

private:
    HBITMAP m_hBitmap;
};

#endif //(DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#endif //EXAMPLES_HBITMAP_CONTROL_H_
