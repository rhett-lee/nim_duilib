#ifndef EXAMPLES_RENDER_TEST1_H_
#define EXAMPLES_RENDER_TEST1_H_

// duilib
#include "duilib/duilib.h"

namespace ui {

/** 图片绘制测试控件：以表格形式展示 IRender 中图片绘制接口的各种用法
*/
class RenderTest1 : public ui::Control
{
    typedef ui::Control BaseClass;
public:
    explicit RenderTest1(ui::Window* pWindow);
    virtual ~RenderTest1() override;

public:
    /** 绘制控件自身
    * @param[in] pRender 渲染接口
    * @param[in] rcPaint 指定绘制区域
    */
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;

    /** 计算控件大小：返回展示全部绘制示例所需的大小（用于配合纵向滚动条）
    * @param [in] szAvailable 可用大小，不包含内边距，不包含外边距
    */
    virtual UiEstSize EstimateSize(UiSize szAvailable) override;

private:
    /** 单元格的绘制类型（对应 IRender 中图片绘制接口的一种用法）
    */
    enum class CellType
    {
        kNormal,            //正常绘制图片
        kFade,              //半透明绘制图片
        kScaleNearest,      //缩小绘制（最近邻采样）
        kScaleLinear,       //缩小绘制（双线性采样）
        kRotate,            //旋转绘制（使用变换矩阵）
        kClip,              //裁剪绘制（使用剪辑区域）
        kBitBlt,            //BitBlt
        kStretchBlt,        //StretchBlt
        kAlphaBlend,        //AlphaBlend
        kTiled,             //平铺绘制
        kTiledMargin,       //平铺绘制（指定边距）
        kNinePatch,         //九宫格拉伸绘制
        kNinePatchTiled,    //九宫格+平铺绘制（指定内边距）
        kWindowShadow,      //九宫格边框绘制（窗口阴影模式）
    };

    /** 单元格布局参数（各值均为DPI缩放后的像素值）
    */
    struct LayoutInfo
    {
        int32_t nColumns = 4;       //列数
        int32_t nCellWidth = 0;     //单元格宽度（图片绘制区域宽度）
        int32_t nCellHeight = 0;    //单元格高度（图片绘制区域高度）
        int32_t nCaptionHeight = 0; //单元格标题高度
        int32_t nGap = 0;           //单元格之间的间距
        int32_t nMargin = 0;        //内容区域的外边距
        int32_t nRows = 0;          //总行数
    };

    /** 单元格的绘制上下文
    */
    struct PaintContext
    {
        IRender* pRender = nullptr;         //渲染接口
        IRenderFactory* pRenderFactory = nullptr; //渲染接口工厂
        const UiRect* pRcPaint = nullptr;   //需要绘制的区域
        IBitmap* pBitmap = nullptr;         //原图位图
        IBitmap* pBitmapLinear = nullptr;   //原图位图（双线性采样缩放后的图片，用于展示采样质量差异）
        IBitmap* pTiledBitmap = nullptr;    //小图位图（用于平铺绘制）
        IRender* pSrcRender = nullptr;      //原图的Render对象（BitBlt/StretchBlt/AlphaBlend使用）
        UiRect rcImageSource;               //原图的源区域
        UiRect rcTiledSource;               //小图的源区域
    };

    /** 获取图片对象（按需加载，并处理DPI变化）
    */
    Image* GetImage();

    /** 获取图片的显示尺寸
    * @param [out] nImageWidth 返回图片宽度
    * @param [out] nImageHeight 返回图片高度
    * @return 获取成功返回true
    */
    bool GetImageSize(int32_t& nImageWidth, int32_t& nImageHeight);

    /** 计算单元格布局参数
    * @param [in] nImageWidth 图片宽度
    * @param [in] nImageHeight 图片高度
    */
    LayoutInfo CalcLayout(int32_t nImageWidth, int32_t nImageHeight) const;

    /** 获取单元格的绘制类型
    */
    static CellType GetCellType(int32_t nIndex);

    /** 获取单元格总数
    */
    static int32_t GetCellCount();

    /** 获取单元格的标题
    */
    static const char* GetCellCaption(CellType cellType);

    /** 绘制一个单元格
    * @param [in] context 绘制上下文
    * @param [in] cellType 单元格的绘制类型
    * @param [in] rcCell 单元格的图片绘制区域（客户区坐标）
    */
    void PaintCell(const PaintContext& context, CellType cellType, const UiRect& rcCell);

    /** 绘制单元格的标题
    */
    void PaintCellCaption(IRender* pRender, const char* pCaption, const UiRect& rcCaption);

private:
    /** 绘制的图片
    */
    std::unique_ptr<Image> m_pImage;
};

} //end of namespace ui
#endif //EXAMPLES_RENDER_TEST1_H_
