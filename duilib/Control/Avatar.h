#ifndef UI_CONTROL_AVATAR_H_
#define UI_CONTROL_AVATAR_H_

#include "duilib/Core/Control.h"

namespace ui
{
class Image;

/** 头像形状
*/
enum class AvatarShape : uint8_t
{
    kCircle = 0,    //圆形（默认，非正方形区域时为椭圆）
    kSquare = 1     //圆角方形
};

/** Avatar 头像控件
 *  用圆形或圆角方形展示"图片 / 图标 / 文字"三类头像内容，语义对齐 Ant Design 的 Avatar。
 *  内容优先级：image（等比 cover 填满并按形状裁剪）→ icon（等比 contain 居中）→ text/text_id（文字）→ 空（只画底色）。
 *  背景色优先级：color（显式）> bkcolor（通用属性）> 自动调色板（仅文字头像，按文本哈希取 bg_avatar_1..6）> bg_avatar。
 *  文字字号按控件短边的百分比（text_scale）自动换算，超出宽度时先缩小字号，最后用省略号收尾。
 *  本控件为叶子自绘控件，不含子控件；不派发专用事件（需要点击时可在外部挂 kEventClick）。
 */
class DUILIB_API Avatar : public Control
{
    typedef Control BaseClass;
public:
    explicit Avatar(Window* pWindow);
    Avatar(const Avatar& r) = delete;
    Avatar& operator=(const Avatar& r) = delete;
    ~Avatar() override;

    /** 获取控件类型
    */
    virtual DString GetType() const override;

    /** 设置XML属性
    */
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 获取用于估算控件大小（宽和高）的图片接口
    */
    virtual Image* GetEstimateImage() override;

    /** 清理图片缓存（主题切换后重新加载头像图片与图标）
    */
    virtual void ClearImageCache() override;

    /** 语言发生变化，刷新界面文字显示相关的内容
    */
    virtual void OnLanguageChanged(bool bRedraw) override;

public:
    /** 设置头像尺寸（同时设置固定宽高），默认 32
     * @param[in] nSize 尺寸值（未经 DPI 缩放的原始像素），小于等于 0 时忽略
     */
    void SetSize(int32_t nSize);

    /** 获取头像尺寸（未经 DPI 缩放的原始像素）
     */
    int32_t GetSize() const { return m_nSize; }

    /** 设置头像形状
     */
    void SetShape(AvatarShape shape);

    /** 获取头像形状
     */
    AvatarShape GetShape() const { return m_shape; }

    /** 设置方形头像的圆角半径（未经 DPI 缩放的原始像素），默认 8
     */
    void SetCorner(int32_t nCorner);

    /** 获取方形头像的圆角半径
     */
    int32_t GetCorner() const { return m_nCorner; }

    /** 设置头像图片（支持完整的图片属性串，如 "file='xxx.png' svg_replace_colors='...'"）
     */
    void SetImage(const DString& strImage);

    /** 获取头像图片的图片属性串
     */
    DString GetImage() const { return m_strImageString; }

    /** 设置头像图标（支持完整的图片属性串）
     */
    void SetIcon(const DString& strIcon);

    /** 获取头像图标的图片属性串
     */
    DString GetIcon() const { return m_strIconString; }

    /** 设置图标边长占控件短边的百分比，默认 50
     */
    void SetIconScale(int32_t nIconScale);

    /** 获取图标边长占控件短边的百分比
     */
    int32_t GetIconScale() const { return m_nIconScale; }

    /** 设置文字内容（优先于 text_id）
     */
    void SetText(const DString& strText);

    /** 获取解析后的文字内容（text 优先，其次 text_id）
     */
    DString GetText() const;

    /** 设置文字内容的多语言 ID
     */
    void SetTextId(const DString& strTextId);

    /** 获取文字内容的多语言 ID
     */
    DString GetTextId() const { return m_strTextId; }

    /** 设置文字字体 ID（决定字体族与粗斜体），默认 system_regular_14
     */
    void SetFontId(const DString& strFontId);

    /** 获取文字字体 ID
     */
    DString GetFontId() const { return m_strFontId; }

    /** 设置文字颜色（语义色名），默认 text_avatar
     */
    void SetTextColor(const DString& strTextColor);

    /** 获取文字颜色
     */
    DString GetTextColor() const { return m_strTextColor; }

    /** 设置字号占控件短边的百分比，默认 50
     */
    void SetTextScale(int32_t nTextScale);

    /** 获取字号占控件短边的百分比
     */
    int32_t GetTextScale() const { return m_nTextScale; }

    /** 设置头像底色（语义色名），优先级最高
     */
    void SetColor(const DString& strColor);

    /** 获取头像底色
     */
    DString GetColor() const { return m_strColor; }

    /** 设置文字头像是否按文本自动配色（取 bg_avatar_1..6 调色板），默认 true
     */
    void SetAutoPalette(bool bAutoPalette);

    /** 是否为文字头像自动配色
     */
    bool IsAutoPalette() const { return m_bAutoPalette; }

protected:
    /** 绘制背景色（按头像形状填充）
     */
    virtual void PaintBkColor(IRender* pRender) override;

    /** 绘制状态颜色（用户配置了状态色时，按头像形状填充）
     */
    virtual void PaintStateColors(IRender* pRender) override;

    /** 绘制头像内容（图片 / 图标 / 文字）
     */
    virtual void PaintText(IRender* pRender) override;

    /** 绘制边框
     */
    virtual void PaintBorder(IRender* pRender) override;

private:
    /** 计算头像形状的圆角半径（圆形为半宽/半高，方形为 corner）
     */
    void GetShapeRound(const UiRect& rc, float& fRoundWidth, float& fRoundHeight) const;

    /** 获取当前应绘制的底色（color > bkcolor > 调色板 > bg_avatar）
     */
    UiColor GetAvatarBkColor() const;

    /** 获取自动调色板对应的语义色名（未启用调色板或无文字时返回空）
     */
    DString GetPaletteColorName() const;

    /** 按状态色回退规则取当前状态实际生效的颜色名
     */
    DString GetStateColorName(ControlStateType stateType) const;

    /** 绘制图片内容（cover：等比填满矩形并居中裁切）
     */
    void PaintCoverImage(IRender* pRender, Image* pImage, const UiRect& rcContent);

    /** 绘制图标内容（contain：等比缩小到图标方框内居中）
     */
    void PaintContainImage(IRender* pRender, Image* pImage, const UiRect& rcContent);

    /** 绘制文字内容（按 text_scale 换算字号，超宽时缩小或用省略号）
     */
    void PaintAvatarText(IRender* pRender, const UiRect& rcContent);

    /** 应用头像尺寸到固定宽高（做 DPI 缩放）
     * @param [in] bArrange 是否立即触发父容器重新布局（运行期修改尺寸时必须为 true，
     *                      否则固定宽高虽然变化，但控件矩形不会更新，界面看不出变化）
     */
    void UpdateFixedSize(bool bArrange);

private:
    /** 头像尺寸（未经 DPI 缩放的原始像素）
     */
    int32_t m_nSize;

    /** 头像形状
     */
    AvatarShape m_shape;

    /** 方形头像的圆角半径（未经 DPI 缩放的原始像素）
     */
    int32_t m_nCorner;

    /** 头像图片的图片属性串
     */
    DString m_strImageString;

    /** 头像图片
     */
    std::unique_ptr<Image> m_pImage;

    /** 头像图标的图片属性串
     */
    DString m_strIconString;

    /** 头像图标
     */
    std::unique_ptr<Image> m_pIcon;

    /** 图标边长占控件短边的百分比
     */
    int32_t m_nIconScale;

    /** 文字内容
     */
    DString m_strText;

    /** 文字内容的多语言 ID
     */
    DString m_strTextId;

    /** 文字字体 ID
     */
    DString m_strFontId;

    /** 文字颜色（语义色名）
     */
    DString m_strTextColor;

    /** 字号占控件短边的百分比
     */
    int32_t m_nTextScale;

    /** 头像底色（语义色名）
     */
    DString m_strColor;

    /** 文字头像是否按文本自动配色
     */
    bool m_bAutoPalette;
};

} // namespace ui

#endif // UI_CONTROL_AVATAR_H_
