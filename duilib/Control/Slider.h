#ifndef UI_CONTROL_SLIDER_H_
#define UI_CONTROL_SLIDER_H_

#include "duilib/Control/Progress.h"
#include <string>

namespace ui
{

/** 滑块控件
*/
class UILIB_API Slider : public Progress
{
    typedef Progress BaseClass;
public:
    explicit Slider(Window* pWindow);

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual UiRect GetProgressPos() override;
    virtual void HandleEvent(const EventArgs& msg) override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void PaintStateImages(IRender* pRender) override;
    virtual void PaintBkColor(IRender* pRender) override;
    virtual void ClearImageCache() override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 获取步进步长
     * @return 返回步进步长
     */
    int GetChangeStep();

    /** 设置步进步长
     * @param[in] step 要设置的步长
     */
    void SetChangeStep(int step);

    /** 设置滑块的大小
     * @param [in] szXY 要设置的大小
     * @param [in] bNeedDpiScale 是否 DPI 缩放
     */
    void SetThumbSize(UiSize szXY, bool bNeedDpiScale);

    /** 获取滑块的大小
    */
    UiSize GetThumbSize() const;

    /** 获取滑块的矩形信息
     * @return 返回滑块的矩形信息
     */
    UiRect GetThumbRect() const;

    /** 获取指定状态下滑块的图片
     * @param[in] stateType 要获取的状态标识，参考 ControlStateType 枚举
     * @return 返回图片路径
     */
    DString GetThumbStateImage(ControlStateType stateType) const;

    /** 设置指定状态下滑块的图片
     * @param[in] stateType 要设置的状态标识，参考 ControlStateType 枚举
     * @param[in] pStrImage 要设置的图片位置
     */
    void SetThumbStateImage(ControlStateType stateType, const DString& pStrImage);

    /** 获取进度条内边距
     */
    const UiPadding& GetProgressBarPadding() const;

    /** 设置进度条内边距
     * @param [in] padding 要设置的内边距信息
     * @param [in] bNeedDpiScale 是否 DPI 缩放
     */
    void SetProgressBarPadding(UiPadding padding, bool bNeedDpiScale);

    /** 监听进度条进度改变事件
     * @param[in] callback 进度条进度改变后调用的回调函数
     */
    void AttachValueChange(const EventCallback& callback) { AttachEvent(kEventValueChange, callback);    }

protected:
    int m_nStep;
    UiSize m_szThumb;
    StateImage m_thumbStateImage;
    UiPadding m_rcProgressBarPadding;
    UiString m_sImageModify;
};

}

#endif // UI_CONTROL_SLIDER_H_
