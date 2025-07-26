#ifndef UI_CORE_SHADOW_H_
#define UI_CORE_SHADOW_H_

#include "duilib/Core/UiSize.h"
#include "duilib/Core/UiRect.h"
#include <string>

namespace ui 
{

class Box;
class Control;
class Window;
class DpiManager;

/** 窗口阴影属性
*/
class UILIB_API Shadow
{
public:
    /** 阴影类型
    */
    enum ShadowType
    {
        kShadowFirst        = 0,            //有效值的起始值

        kShadowBig          = 0,            //大阴影，直角（适合普通窗口）
        kShadowBigRound     = 1,            //大阴影，圆角（适合普通窗口）
        kShadowSmall        = 2,            //小阴影，直角（适合普通窗口）
        kShadowSmallRound   = 3,            //小阴影，圆角（适合普通窗口）
        kShadowMenu         = 4,            //小阴影，直角（适合弹出式窗口，比如菜单等）
        kShadowMenuRound    = 5,            //小阴影，圆角（适合弹出式窗口，比如菜单等）
        kShadowNone         = 6,            //无阴影，有边框，直角
        kShadowNoneRound    = 7,            //无阴影，有边框，圆角
        kShadowCustom       = 8,                //用户自定义阴影（设置时会清除默认的阴影属性，后续需要调用SetShadowImage,SetShadowCorner,SetShadowBorderRound设置阴影属性）
        kShadowCount,                           //有效值的最大值

        kShadowDefault      = kShadowBigRound   //默认阴影（未设置时，默认使用此值）
    };

    /** 根据字符串获取对应的阴影类型
    */
    static bool GetShadowType(const DString& typeString, ShadowType& nShadowType);

    /** 获取默认的阴影类型对应的参数
    * @param [in] nShadowType 阴影类型
    * @param [out] szBorderRound 返回圆角大小，未经DPI缩放
    * @param [out] rcShadowCorner 返回阴影素材的九宫格属性，未经DPI缩放
    * @param [out] shadowImage 返回阴影图片的属性，包含阴影图片的九宫格属性
    * @param [in] pShadowObj 关联的阴影对象
    */
    static bool GetShadowParam(ShadowType nShadowType,
                               UiSize& szBorderRound,
                               UiPadding& rcShadowCorner,
                               DString& shadowImage,
                               Shadow* pShadowObj = nullptr);

public:
    /** 构造函数
     @param [in] pWindow 关联的窗口
    */
    explicit Shadow(Window* pWindow);

    /** 设置是否支持阴影效果
     * @param[in] bShadowAttached 设置 true 为支持阴影效果，false 为不支持阴影效果
     */
    void SetShadowAttached(bool bShadowAttached);

    /** 判断是否已经支持阴影效果
     */
    bool IsShadowAttached() const;

    /** 当前阴影效果值，是否为默认值
    */
    bool IsUseDefaultShadowAttached() const;

    /** 设置当前阴影效果值，是否为默认值
    */
    void SetUseDefaultShadowAttached(bool bDefault);

    /** 设置阴影类型
    */
    void SetShadowType(Shadow::ShadowType nShadowType);

    /** 获取阴影类型
    */
    Shadow::ShadowType GetShadowType() const;

    /** 设置阴影素材的九宫格描述
    * @param [in] rcShadowCorner 阴影图片的九宫格属性，未经DPI缩放的值
    */
    void SetShadowCorner(const UiPadding& rcShadowCorner);

    /** 获取已经设置的阴影九宫格属性
     *@return 返回通过SetShadowCorner函数设置的九宫格属性，未经DPI缩放的值
     */
    UiPadding GetShadowCorner() const;

    /** 获取当前的阴影九宫格属性（已经做过DPI缩放）
     *@return 如果阴影未Attached或者窗口最大化，返回UiPadding(0, 0, 0, 0)，否则返回设置的九宫格属性（已经做过DPI缩放）
     */
    UiPadding GetCurrentShadowCorner() const;

    /** 设置阴影的圆角大小
    * @param [in] szBorderRound 阴影的圆角大小，未经DPI缩放的值
    */
    void SetShadowBorderRound(UiSize szBorderRound);

    /** 获取阴影的圆角大小
    * @return 返回阴影的圆角大小，未经DPI缩放的值
    */
    UiSize GetShadowBorderRound() const;

    /** 设置阴影图片属性
     */
    void SetShadowImage(const DString& shadowImage);

    /** 获取阴影图片属性
     */
    const DString& GetShadowImage() const;

    /** 设置阴影的边框大小(未经DPI缩放)
    */
    void SetShadowBorderSize(int32_t nShadowBorderSize);

    /** 获取阴影的边框大小(未经DPI缩放)
    */
    int32_t GetShadowBorderSize() const;

    /** 设置阴影的边框颜色
    */
    void SetShadowBorderColor(const DString& shadowBorderColor);

    /** 获取阴影的边框颜色
    */
    const DString& GetShadowBorderColor() const;

public:
    /** 将阴影附加到窗口的顶层容器
     * @param[in] pXmlRoot 窗口的顶层容器，XML配置里面的顶层容器
     */
    Box* AttachShadow(Box* pXmlRoot);

    /** 获取附加阴影后的容器指针
    */
    Box* GetShadowBox() const;

    /** 判断是否有附加的阴影Box
    */
    bool HasShadowBox() const;

    /** 获取XML配置里面的顶层容器（即AttachShadow传入的Box）
    */
    Box* GetAttachedXmlRoot() const;

    /** 设置窗口最大化还是还原状态
     * @param[in] isMaximized 设置为 true 表示最大化，false 为还原初始状态
     */
    void MaximizedOrRestored(bool isMaximized);

    /** 清理图片缓存
     */
    void ClearImageCache();

    /** DPI发生变化，更新控件大小和布局
    * @param [in] dpi DPI管理器
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    void ChangeDpiScale(const DpiManager& dpi, uint32_t nOldDpiScale, uint32_t nNewDpiScale);

    /** 设置窗口贴边属性
    */
    void SetWindowPosSnap(bool bLeftSnap, bool bRightSnap, bool bTopSnap, bool bBottomSnap);

    /** 设置阴影是否支持窗口贴边操作
    */
    void SetEnableShadowSnap(bool bEnable);

    /** 获取阴影是否支持窗口贴边操作
    */
    bool IsEnableShadowSnap() const;

private:
    /** 将阴影附加到窗口
     */
    void DoAttachShadow(Box* pNewRoot, Box* pOrgRoot, bool bNewAttach, bool isMaximized) const;

    /** 附加阴影事件
    */
    void OnShadowAttached(Shadow::ShadowType nShadowType);

    /** 更新阴影属性
    */
    void UpdateShadow();

    /** 更新窗口的贴边属性
    */
    void UpdateWindowPosSnap();

private:
    //是否支持阴影效果
    bool m_bShadowAttached;

    //当前阴影效果值，是否为默认值
    bool m_bUseDefaultShadowAttached;

    //当前是否为最大化状态
    bool m_isMaximized;

    /** 阴影是否支持窗口贴边操作
    */
    bool m_bEnableShadowSnap;

    /** 窗口贴边属性(左侧，上侧，右侧，下侧)
    */
    bool m_bLeftSnap;
    bool m_bTopSnap;
    bool m_bRightSnap;
    bool m_bBottomSnap;

    //阴影图片属性
    DString m_shadowImage;

    //阴影九宫格属性(未经DPI缩放)
    UiPadding m_rcShadowCorner;

    //阴影的圆角大小(未经DPI缩放)
    UiSize m_szBorderRound;

    /** 阴影的边框大小(未经DPI缩放)
    */
    int32_t m_nShadowBorderSize;

    /** 阴影的边框颜色
    */
    DString m_shadowBorderColor;

    //阴影容器接口
    Box* m_pShadowBox;

    //关联的窗口
    Window* m_pWindow;

    //阴影类型
    Shadow::ShadowType m_nShadowType;
};

}

#endif // UI_CORE_SHADOW_H_
