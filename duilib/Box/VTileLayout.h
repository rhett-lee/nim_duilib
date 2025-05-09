#ifndef UI_BOX_VTILE_LAYOUT_H_
#define UI_BOX_VTILE_LAYOUT_H_

#include "duilib/Box/Layout.h"

namespace ui 
{

/** 瓦片布局(纵向)
*/
class UILIB_API VTileLayout : public Layout
{
    typedef Layout BaseClass;
public:
    VTileLayout();

    /** 布局类型
    */
    virtual LayoutType GetLayoutType() const override { return LayoutType::VTileLayout; }

    /** 调整内部所有控件的位置信息
     * @param[in] items 控件列表
     * @param[in] rc 当前容器位置信息, 包含内边距，但不包含外边距
     * @return 返回排列后最终盒子的宽度和高度信息
     */
    virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;

    /** 根据内部子控件大小估算容器自身大小，拉伸类型的子控件被忽略，不计入大小估算
     * @param[in] items 子控件列表
     * @param [in] szAvailable 可用大小，包含分配给该控件的内边距，但不包含分配给控件的外边距
     * @return 返回排列后最终布局的大小信息（宽度和高度）；
               包含items中子控件的外边距，包含items中子控件的内边距；
               包含Box控件本身的内边距；
               不包含Box控件本身的外边距；
               返回值中不包含拉伸类型的子控件大小。
     */
    virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable) override;
    
    /** 设置布局属性
     * @param [in] strName 要设置的属性名
     * @param [in] strValue 要设置的属性值
     * @param [in] dpiManager DPI管理接口
     * @return true 设置成功，false 属性不存在
     */
    virtual bool SetAttribute(const DString& strName, 
                              const DString& strValue,
                              const DpiManager& dpiManager) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] dpiManager DPI缩放管理器
    */
    virtual void ChangeDpiScale(const DpiManager& dpiManager, uint32_t nOldDpiScale) override;

public:
    /** 获取子项大小，该宽度和高度，是包含了控件的外边距和内边距的
     */
    const UiSize& GetItemSize() const;

    /** 设置子项大小
     * @param[in] szItem 子项大小数据，该宽度和高度，是包含了控件的外边距和内边距的
     * @param [in] bArrange 当变化的时候，是否需要重排
     */
    void SetItemSize(UiSize szItem, bool bArrange = true);

    /** 获取列数量
     */
    int32_t GetColumns() const;

    /** 设置显示几列数据
     * @param[in] nCols 要设置显示几列的数值
     */
    void SetColumns(int32_t nCols);

    /** 设置是否自动计算列数
     */
    void SetAutoCalcColumns(bool bAutoCalcColumns);

    /** 判断是否自动计算列数
     */
    bool IsAutoCalcColumns() const;

    /** 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内
    */
    void SetScaleDown(bool bScaleDown);

    /** 判断超出边界的时候，是否按比例缩小
    */
    bool IsScaleDown() const;

    /** 设置是否自动计算子项的宽度（仅当设置为固定列时有效）
    */
    void SetAutoCalcItemWidth(bool bAutoCalcItemWidth);

    /** 是否自动计算子项的宽度
    */
    bool IsAutoCalcItemWidth() const;

    /** 当前是否为自由布局
    */
    bool IsFreeLayout() const;

    /** 计算子项的宽度
    */
    bool AutoCalcItemWidth(int32_t nColumns, int32_t nMarginX, int32_t szAvailable, int32_t& nItemWidth) const;

private:
    /** 未处理的子控件接口和其宽高信息
    */
    struct ItemSizeInfo
    {
        Control* pControl = nullptr; //子控件接口
        int32_t cx = 0;              //子控件的宽度
        int32_t cy = 0;              //子控件的高度
    };

    /** 获取估算大小时的可用宽高
    * @param [in] pControl 空间接口
    * @param [in] szItem 每个瓦片控件的宽度和高度(设置值)
    * @param [in] rc 瓦片控件所在容器的可用区域矩形
    * @return 返回该空间的估算大小（宽和高）
    */
    static UiSize CalcEstimateSize(Control* pControl, const UiSize& szItem, const UiRect& rc);

    /** 获取基本参数：瓦片的列数
    * @param [in] normalItems 子控件列表
    * @param [in] rc 瓦片控件所在容器的矩形
    * @param [in] tileWidth 每个瓦片控件的宽度(配置值)
    * @param [in] childMarginX 子控件的X轴间隔
    * @param [in] childMarginY 子控件的Y轴间隔    
    * @param [out] nColumns 返回总列数
    */
    static void CalcTileColumns(const std::vector<ItemSizeInfo>& normalItems, const UiRect& rc,
                                int32_t tileWidth, int32_t childMarginX, int32_t childMarginY,
                                int32_t& nColumns);

    /** 估算浮动控件的大小
    */
    static UiSize64 EstimateFloatSize(Control* pControl, const UiRect& rc);

    /** 处理浮动子控件，并返回未处理的子控件列表
    * @param [in] items 子控件列表
    * @param [in] rect 外部可用矩形大小
    * @param [in] szItem 每个瓦片控件的宽度和高度(设置值)
    * @param [in] isCalcOnly 如果为true表示仅计算区域，对控件位置不做调整；如果为false，表示对控件位置做调整。
    * @param [out] normalItems 返回未处理的子控件列表，及其大小信息
    * @return 返回浮动控件所占的区域宽度和高度
    */
    static UiSize64 ArrangeFloatChild(const std::vector<Control*>& items,
                                      const UiRect& rc,
                                      const UiSize& szItem,
                                      bool isCalcOnly, 
                                      std::vector<ItemSizeInfo>& normalItems);

    /** 获取基本参数：瓦片高度，布局排列过程中，在每行开始时，计算本行的高度
    * @param [in] normalItems 子控件列表
    * @param [in] iterBegin 子控件开始的迭代器
    * @param [in] nColumns 总列数
    * @param [in] szItem 瓦片控件宽度和高度（设置值）
    * @return 返回高度值，包含了外边距Margin.top + Margin.bottom值
    */
    static int32_t CalcTileRowHeight(const std::vector<ItemSizeInfo>& normalItems,
                                     const std::vector<ItemSizeInfo>::const_iterator iterBegin,
                                      int32_t nColumns,
                                     const UiSize& szItem);

    /** 计算瓦片控件的显示坐标和大小
    * @param [in] itemSizeInfo 瓦片控件的接口, 及控件的大小信息
    * @param [in] tileWidth 配置的瓦片控件宽度
    * @param [in] tileHeight 配置的瓦片控件高度（取行高）
    * @param [in] ptTile 当前瓦片控件左上角的坐标
    * @param [in] bScaleDown 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内
    * @param [out] szTilePos 瓦片控件的显示坐标、宽度和高度
    * @return 返回瓦片控件目标区域的大小（宽和高），宽度和高度包含了控件的外边距
    */
    static UiSize CalcTilePosition(const ItemSizeInfo& itemSizeInfo,
                                   int32_t tileWidth,
                                   int32_t tileHeight,
                                   const UiPoint& ptTile,
                                   bool bScaleDown, 
                                   UiRect& szTilePos);

private:
    /** 对子控件布局的内部实现函数
    * @param [in] items 子控件列表
    * @param [in] rect 可用矩形大小，包含分配给该控件的内边距，但不包含分配给控件的外边距
    * @param [in] isCalcOnly 如果为true表示仅计算区域，对控件位置不做调整；如果为false，表示对控件位置做调整。
    * @param [in] inColumnWidths 每列的宽度值，可用为空
    * @param [out] outColumnWidths 本次布局，使用的每列宽度值
    * @return 返回区域的宽度和高度
    */
    UiSize64 ArrangeChildNormal(const std::vector<Control*>& items, 
                                UiRect rect,
                                bool isCalcOnly,
                                const std::vector<int32_t>& inColumnWidths,
                                std::vector<int32_t>& outColumnWidths) const;

    /** 使用自由布局排列控件(无固定列数，尽量充分利用展示空间，显示尽可能多的内容)
    * @param [in] items 子控件列表
    * @param [in] rect 可用矩形大小，包含分配给该控件的内边距，但不包含分配给控件的外边距
    * @param [in] isCalcOnly 如果为true表示仅计算区域，对控件位置不做调整；如果为false，表示对控件位置做调整。
    * @return 返回区域的宽度和高度
    */
    UiSize64 ArrangeChildFreeLayout(const std::vector<Control*>& items, 
                                    UiRect rect, 
                                    bool isCalcOnly) const;

private:
    //子项大小, 该宽度和高度，是包含了控件的外边距和内边距的
    UiSize m_szItem;

    //显示几列数据
    int32_t m_nColumns;

    //自动计算列数
    bool m_bAutoCalcColumns;

    //当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内
    bool m_bScaleDown;

    //是否自动计算子项的宽度（根据父控件总体宽度自动适应，仅当设置为固定列时有效）
    bool m_bAutoCalcItemWidth;
};

} // namespace ui

#endif // UI_BOX_VTILE_LAYOUT_H_
