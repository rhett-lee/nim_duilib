#ifndef UI_CONTROL_PROPERTY_GRID_H_
#define UI_CONTROL_PROPERTY_GRID_H_

#include "duilib/Box/VBox.h"
#include "duilib/Core/FontManager.h"
#include "duilib/Core/ControlPtrT.h"
#include "duilib/Control/Split.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/RichText.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/TreeView.h"
#include "duilib/Control/Combo.h"
#include "duilib/Control/ComboButton.h"
#include "duilib/Control/DateTime.h"
#include "duilib/Control/IPAddress.h"
#include "duilib/Control/HotKey.h"
#include "duilib/Utils/FileDialog.h"

namespace ui
{
/** 属性表控件的支持的属性
*/
class PropertyGridGroup;
class PropertyGridProperty;
class PropertyGridTextProperty;         //文本和数字
class PropertyGridComboProperty;        //下拉框
class PropertyGridFontProperty;         //字体名称
class PropertyGridFontSizeProperty;     //字体大小
class PropertyGridColorProperty;        //颜色
class PropertyGridDateTimeProperty;     //日期时间
class PropertyGridIPAddressProperty;    //IP地址
class PropertyGridHotKeyProperty;       //热键
class PropertyGridFileProperty;         //文件路径
class PropertyGridDirectoryProperty;    //文件夹

/** 属性的参数 (支持多语言版)
*/
struct PropertyGridParam
{
    DString m_propertyName;     //属性的名称
    DString m_propertyNameId;   //属性的名称ID（支持多语言版）
    DString m_propertyValue;    //属性的取值
    DString m_propertyValueId;  //属性的取值ID（支持多语言版）
    DString m_description;      //属性的描述信息
    DString m_descriptionId;    //属性的描述信息ID（支持多语言版）
    size_t m_nPropertyData = 0; //用户自定义数据
};

/** 属性表控件
*/
class DUILIB_API PropertyGrid : public VBox
{
    typedef VBox BaseClass;
public:
    explicit PropertyGrid(Window* pWindow);

    /** 获取控件类型
    */
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

public:
    /** 设置是否显示表头
    * @param [in] bEnable true表示显示表头，false表示隐藏表头
    * @param [in] sLeftColumn 表头左侧一列的名称
    * @param [in] sRightColumn 表头右侧一列的名称
    * @param [in] bTextId true表示上述名称为文本ID（支持多语言版），false为普通文本
    */
    void SetEnableHeaderCtrl(bool bEnable,
                             const DString& sLeftColumn = _T(""), 
                             const DString& sRightColumn = _T(""),
                             bool bTextId = false);
    /** 判断当前是否显示表头
    */
    bool IsEnableHeaderCtrl() const { return m_bHeaderCtrl; }

    /** 获取Header接口
    */
    Control* GetHeaderCtrl() const { return m_pHeaderCtrl; }

    /** 获取Header中的左侧一列
    */
    Label* GetHeaderLeft() const { return m_pHeaderLeft; }

    /** 获取Header中的右侧一列
    */
    Label* GetHeaderRight() const { return m_pHeaderRight; }

    /** 获取Header中的分割条
    */
    Split* GetHeaderSplit() const { return m_pHeaderSplit; }

public:
    /** 增加一个分组
    * @param [in] groupName 分组的名称
    * @param [in] description 分组的描述信息
    * @param [in] nGroupData 用户自定义数据
    * @return 返回该分组的接口，可用于添加属性
    */
    PropertyGridGroup* AddGroup(const DString& groupName, 
                                const DString& description = _T(""),
                                size_t nGroupData = 0);

    /** 增加一个分组（支持多语言版）
    * @param [in] groupNameId 分组的名称ID
    * @param [in] descriptionId 分组的描述信息ID
    * @param [in] nGroupData 用户自定义数据
    * @return 返回该分组的接口，可用于添加属性
    */
    PropertyGridGroup* AddGroupById(const DString& groupNameId, 
                                    const DString& descriptionId = _T(""),
                                    size_t nGroupData = 0);

    /** 获取所有的分组
    * @param [out] groups 返回当前所有的分组列表
    */
    void GetGroups(std::vector<PropertyGridGroup*>& groups) const;

    /** 删除分组
    * @param [in] pGroup 待删除的分组
    */
    bool RemoveGroup(PropertyGridGroup* pGroup);

    /** 删除所有分组
    */
    void RemoveAllGroups();

    /** 添加一个属性(由调用方创建属性)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] pProperty 属性接口
    */
    bool AddProperty(PropertyGridGroup* pGroup, PropertyGridProperty* pProperty);

    /** 增加一个属性(文本、数字类型)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    * @return 返回该属性的接口
    */
    PropertyGridTextProperty* AddTextProperty(PropertyGridGroup* pGroup,
                                              const DString& propertyName, 
                                              const DString& propertyValue,
                                              const DString& description = _T(""),
                                              size_t nPropertyData = 0);

    /** 增加一个属性(文本、数字类型)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyNameId 属性的名称ID
    * @param [in] propertyValue 属性的值或者属性值的ID，参见bPropertyValueId参数说明
    * @param [in] descriptionId 属性的描述信息ID
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] bPropertyValueId true表示propertyValue的值时属性值的资源ID（支持多语言版），false表示为普通文本值
    * @return 返回该属性的接口
    */
    PropertyGridTextProperty* AddTextPropertyById(PropertyGridGroup* pGroup,
                                                  const DString& propertyNameId, 
                                                  const DString& propertyValue,
                                                  const DString& descriptionId = _T(""),
                                                  size_t nPropertyData = 0,
                                                  bool bPropertyValueId = true);

    /** 增加一个属性(文本、数字类型)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] param 该属性的取值
    */
    PropertyGridTextProperty* AddTextProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param);
    

    /** 增加一个属性(下拉框)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    * @return 返回该属性的接口
    */
    PropertyGridComboProperty* AddComboProperty(PropertyGridGroup* pGroup,
                                                const DString& propertyName, 
                                                const DString& propertyValue,
                                                const DString& description = _T(""),
                                                size_t nPropertyData = 0);

    /** 增加一个属性(下拉框)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyNameId 属性的名称ID
    * @param [in] propertyValue 属性的值或者属性值的ID，参见bPropertyValueId参数说明
    * @param [in] descriptionId 属性的描述信息ID
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] bPropertyValueId true表示propertyValue的值时属性值的资源ID（支持多语言版），false表示为普通文本值
    * @return 返回该属性的接口
    */
    PropertyGridComboProperty* AddComboPropertyById(PropertyGridGroup* pGroup,
                                                    const DString& propertyNameId, 
                                                    const DString& propertyValue,
                                                    const DString& descriptionId = _T(""),
                                                    size_t nPropertyData = 0,
                                                    bool bPropertyValueId = true);

    /** 增加一个属性(下拉框)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] param 该属性的取值设置参数
    */
    PropertyGridComboProperty* AddComboProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param);

    /** 增加一个属性(字体名称)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值（字体名称）
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    * @return 返回该属性的接口
    */
    PropertyGridFontProperty* AddFontProperty(PropertyGridGroup* pGroup,
                                              const DString& propertyName, 
                                              const DString& propertyValue,
                                              const DString& description = _T(""),
                                              size_t nPropertyData = 0);

    /** 增加一个属性(字体名称)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyNameId 属性的名称ID
    * @param [in] propertyValue 属性的值（字体名称）或者属性值的ID，参见bPropertyValueId参数说明
    * @param [in] descriptionId 属性的描述信息ID
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] bPropertyValueId true表示propertyValue的值时属性值的资源ID（支持多语言版），false表示为普通文本值
    * @return 返回该属性的接口
    */
    PropertyGridFontProperty* AddFontPropertyById(PropertyGridGroup* pGroup,
                                                  const DString& propertyNameId, 
                                                  const DString& propertyValue,
                                                  const DString& descriptionId = _T(""),
                                                  size_t nPropertyData = 0,
                                                  bool bPropertyValueId = true);

    /** 增加一个属性(字体名称)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] param 该属性的取值设置参数
    */
    PropertyGridFontProperty* AddFontProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param);

    /** 增加一个属性(字体大小)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值（字体大小）
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    * @return 返回该属性的接口
    */
    PropertyGridFontSizeProperty* AddFontSizeProperty(PropertyGridGroup* pGroup,
                                                      const DString& propertyName, 
                                                      const DString& propertyValue,
                                                      const DString& description = _T(""),
                                                      size_t nPropertyData = 0);

    /** 增加一个属性(字体大小)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyNameId 属性的名称ID
    * @param [in] propertyValue 属性的值（字体大小）或者属性值的ID，参见bPropertyValueId参数说明
    * @param [in] descriptionId 属性的描述信息ID
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] bPropertyValueId true表示propertyValue的值时属性值的资源ID（支持多语言版），false表示为普通文本值
    * @return 返回该属性的接口
    */
    PropertyGridFontSizeProperty* AddFontSizePropertyById(PropertyGridGroup* pGroup,
                                                          const DString& propertyNameId, 
                                                          const DString& propertyValue,
                                                          const DString& descriptionId = _T(""),
                                                          size_t nPropertyData = 0,
                                                          bool bPropertyValueId = true);

    /** 增加一个属性(字体大小)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] param 该属性的取值设置参数
    */
    PropertyGridFontSizeProperty* AddFontSizeProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param);

    /** 增加一个属性(颜色)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值（字体大小）
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    * @return 返回该属性的接口
    */
    PropertyGridColorProperty* AddColorProperty(PropertyGridGroup* pGroup,
                                                const DString& propertyName, 
                                                const DString& propertyValue,
                                                const DString& description = _T(""),
                                                size_t nPropertyData = 0);

    /** 增加一个属性(颜色)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyNameId 属性的名称ID
    * @param [in] propertyValue 属性的值或者属性值的ID，参见bPropertyValueId参数说明
    * @param [in] descriptionId 属性的描述信息ID
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] bPropertyValueId true表示propertyValue的值时属性值的资源ID（支持多语言版），false表示为普通文本值
    * @return 返回该属性的接口
    */
    PropertyGridColorProperty* AddColorPropertyById(PropertyGridGroup* pGroup,
                                                    const DString& propertyNameId, 
                                                    const DString& propertyValue,
                                                    const DString& descriptionId = _T(""),
                                                    size_t nPropertyData = 0,
                                                    bool bPropertyValueId = true);

    /** 增加一个属性(颜色)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] param 该属性的取值设置参数
    */
    PropertyGridColorProperty* AddColorProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param);

    /** 增加一个属性(日期时间)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyName 属性的名称
    * @param [in] dateTimeValue 属性的值（日期时间值）
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] editFormat 日期的编辑格式
    * @return 返回该属性的接口
    */
    PropertyGridDateTimeProperty* AddDateTimeProperty(PropertyGridGroup* pGroup,
                                                      const DString& propertyName, 
                                                      const DString& dateTimeValue,
                                                      const DString& description = _T(""),
                                                      size_t nPropertyData = 0,
                                                      DateTime::EditFormat editFormat = DateTime::EditFormat::kDateCalendar);

    /** 增加一个属性(日期时间)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyNameId 属性的名称
    * @param [in] dateTimeValue 属性的值（日期时间值）或者属性值的ID，参见bPropertyValueId参数说明
    * @param [in] descriptionId 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] editFormat 日期的编辑格式
    * @param [in] bPropertyValueId true表示propertyValue的值时属性值的资源ID（支持多语言版），false表示为普通文本值
    * @return 返回该属性的接口
    */
    PropertyGridDateTimeProperty* AddDateTimePropertyById(PropertyGridGroup* pGroup,
                                                          const DString& propertyNameId, 
                                                          const DString& dateTimeValue,
                                                          const DString& descriptionId = _T(""),
                                                          size_t nPropertyData = 0,
                                                          bool bPropertyValueId = true,
                                                          ui::DateTime::EditFormat editFormat = ui::DateTime::EditFormat::kDateCalendar);

    /** 增加一个属性(日期时间)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] param 该属性的取值设置参数
    */
    PropertyGridDateTimeProperty* AddDateTimeProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param,
                                                      ui::DateTime::EditFormat editFormat = ui::DateTime::EditFormat::kDateCalendar);
   
    /** 增加一个属性(IP地址)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    * @return 返回该属性的接口
    */
    PropertyGridIPAddressProperty* AddIPAddressProperty(PropertyGridGroup* pGroup,
                                                        const DString& propertyName, 
                                                        const DString& propertyValue,
                                                        const DString& description = _T(""),
                                                        size_t nPropertyData = 0);

    /** 增加一个属性(IP地址)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyNameId 属性的名称ID
    * @param [in] propertyValue 属性的值或者属性值的ID，参见bPropertyValueId参数说明
    * @param [in] descriptionId 属性的描述信息ID
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] bPropertyValueId true表示propertyValue的值时属性值的资源ID（支持多语言版），false表示为普通文本值
    * @return 返回该属性的接口
    */
    PropertyGridIPAddressProperty* AddIPAddressPropertyById(PropertyGridGroup* pGroup,
                                                            const DString& propertyNameId, 
                                                            const DString& propertyValue,
                                                            const DString& descriptionId = _T(""),
                                                            size_t nPropertyData = 0,
                                                            bool bPropertyValueId = true);

    /** 增加一个属性(IP地址)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] param 该属性的取值设置参数
    */
    PropertyGridIPAddressProperty* AddIPAddressProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param);

    /** 增加一个属性(热键)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    * @return 返回该属性的接口
    */
    PropertyGridHotKeyProperty* AddHotKeyProperty(PropertyGridGroup* pGroup,
                                                  const DString& propertyName, 
                                                  const DString& propertyValue,
                                                  const DString& description = _T(""),
                                                  size_t nPropertyData = 0);

    /** 增加一个属性(热键)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyNameId 属性的名称ID
    * @param [in] propertyValue 属性的值或者属性值的ID，参见bPropertyValueId参数说明
    * @param [in] descriptionId 属性的描述信息ID
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] bPropertyValueId true表示propertyValue的值时属性值的资源ID（支持多语言版），false表示为普通文本值
    * @return 返回该属性的接口
    */
    PropertyGridHotKeyProperty* AddHotKeyPropertyById(PropertyGridGroup* pGroup,
                                                      const DString& propertyNameId, 
                                                      const DString& propertyValue,
                                                      const DString& descriptionId = _T(""),
                                                      size_t nPropertyData = 0,
                                                      bool bPropertyValueId = true);

    /** 增加一个属性(热键)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] param 该属性的取值设置参数
    */
    PropertyGridHotKeyProperty* AddHotKeyProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param);

    /** 增加一个属性(文件路径)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值(文件路径)
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] bOpenFileDialog true表示打开文件，false表示保存文件
    * @param [in] fileTypes 对话框可以打开或保存的文件类型
    * @param [in] nFileTypeIndex 选择的文件类型，有效范围：[0, fileTypes.size())
    * @param [in] defaultExt 默认的文件类型, 举例："doc;docx"
    * @return 返回该属性的接口
    */
    PropertyGridFileProperty* AddFileProperty(PropertyGridGroup* pGroup,
                                              const DString& propertyName, 
                                              const DString& propertyValue,                                              
                                              const DString& description = _T(""),
                                              size_t nPropertyData = 0,
                                              bool bOpenFileDialog = true,
                                              const std::vector<FileDialog::FileType>& fileTypes = std::vector<FileDialog::FileType>(),
                                              int32_t nFileTypeIndex = -1,
                                              const DString& defaultExt = _T(""));

    /** 增加一个属性(文件路径)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyNameId 属性的名称ID
    * @param [in] propertyValue 属性的值(文件路径)或者属性值的ID，参见bPropertyValueId参数说明
    * @param [in] descriptionId 属性的描述信息ID
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] bOpenFileDialog true表示打开文件，false表示保存文件
    * @param [in] fileTypes 对话框可以打开或保存的文件类型
    * @param [in] nFileTypeIndex 选择的文件类型，有效范围：[0, fileTypes.size())
    * @param [in] defaultExt 默认的文件类型, 举例："doc;docx"
    * @param [in] bPropertyValueId true表示propertyValue的值时属性值的资源ID（支持多语言版），false表示为普通文本值
    * @return 返回该属性的接口
    */
    PropertyGridFileProperty* AddFilePropertyById(PropertyGridGroup* pGroup,
                                                  const DString& propertyNameId, 
                                                  const DString& propertyValue,
                                                  const DString& descriptionId = _T(""),
                                                  size_t nPropertyData = 0,
                                                  bool bPropertyValueId = true,
                                                  bool bOpenFileDialog = true,
                                                  const std::vector<FileDialog::FileType>& fileTypes = std::vector<FileDialog::FileType>(),
                                                  int32_t nFileTypeIndex = -1,
                                                  const DString& defaultExt = _T(""));

    /** 增加一个属性(文件路径)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] param 该属性的取值设置参数
    */
    PropertyGridFileProperty* AddFileProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param,
                                              bool bOpenFileDialog = true,
                                              const std::vector<FileDialog::FileType>& fileTypes = std::vector<FileDialog::FileType>(),
                                              int32_t nFileTypeIndex = -1,
                                              const DString& defaultExt = _T(""));

    /** 增加一个属性(文件夹)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    * @return 返回该属性的接口
    */
    PropertyGridDirectoryProperty* AddDirectoryProperty(PropertyGridGroup* pGroup,
                                                        const DString& propertyName, 
                                                        const DString& propertyValue,
                                                        const DString& description = _T(""),
                                                        size_t nPropertyData = 0);

    /** 增加一个属性(文件夹)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] propertyNameId 属性的名称ID
    * @param [in] propertyValue 属性的值或者属性值的ID，参见bPropertyValueId参数说明
    * @param [in] descriptionId 属性的描述信息ID
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] bPropertyValueId true表示propertyValue的值时属性值的资源ID（支持多语言版），false表示为普通文本值
    * @return 返回该属性的接口
    */
    PropertyGridDirectoryProperty* AddDirectoryPropertyById(PropertyGridGroup* pGroup,
                                                            const DString& propertyNameId,
                                                            const DString& propertyValue,
                                                            const DString& descriptionId = _T(""),
                                                            size_t nPropertyData = 0,
                                                            bool bPropertyValueId = true);

    /** 增加一个属性(文件夹)(支持多语言版)
    * @param [in] pGroup 该属性所属的分组
    * @param [in] param 该属性的取值设置参数
    */
    PropertyGridDirectoryProperty* AddDirectoryProperty(PropertyGridGroup* pGroup, const PropertyGridParam& param);

    /** 设置左侧一列的宽度
    * @param [in] nLeftColumnWidth 左侧一列的宽度
    * @param [in] bNeedDpiScale 是否需要对列宽值进行DPI自适应
    */
    void SetLeftColumnWidth(int32_t nLeftColumnWidth, bool bNeedDpiScale);

    /** 获取左侧一列的宽度值
    */
    int32_t GetLeftColumnWidth() const;

    /** 设置属性值的字体Id（正常状态）
    */
    void SetPropertyNormalFontId(const DString& fontId);

    /** 获取属性值的字体ID（正常状态）
    */
    DString GetProptertyNormalFontId() const;

    /** 设置属性值的字体Id（已修改状态）
    */
    void SetPropertyModifiedFontId(const DString& fontId);

    /** 获取属性值的字体ID（已修改状态）
    */
    DString GetProptertyModifiedFontId() const;

public:
    /** 设置是否显示描述区域
    */
    void SetEnableDescriptionArea(bool bEnable);

    /** 判断是否显示描述区域
    */
    bool IsEnableDescriptionArea() const { return m_bDescriptionArea; }

    /** 设置描述区域的高度值
    * @param [in] nHeight 高度值
    * @param [in] bNeedDpiScale 是否需要对列宽值进行DPI自适应
    */
    void SetDescriptionAreaHeight(int32_t nHeight, bool bNeedDpiScale);

    /** 获取当前描述区域的高度值
    */
    int32_t GetDescriptionAreaHeight() const;

    /** 获取描述控件的接口
    */
    RichText* GetDescriptionArea() const { return m_pDescriptionArea; }

    /** 描述控件的分割条接口
    *   @return 返回描述区域下方的分割条指针，若 XML 中未声明 `duilib_property_grid_description_area_split` 子控件则返回 nullptr
    */
    Split* GetDescriptionAreaSplit() const { return m_pDescriptionAreaSplit; }

    /** 描述区是否可用（XML 中声明了 duilib_property_grid_description_area 子控件）
    */
    bool IsDescriptionArea() const { return m_bDescriptionArea; }

public:
    /** 获取属性表的树控件接口（用于管理数据）
    */
    TreeView* GetTreeView() const { return m_pTreeView; }

    /** 横向网格线的宽度
    * @param [in] nLineWidth 网格线的宽度，如果为0表示不显示横向网格线
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetRowGridLineWidth(float fLineWidth, bool bNeedDpiScale);
    float GetRowGridLineWidth() const;

    /** 横向网格线的颜色
    * @param [in] color 横向网格线的颜色
    */
    void SetRowGridLineColor(const DString& color);
    DString GetRowGridLineColor() const;

    /** 纵向网格线的宽度
    * @param [in] nLineWidth 网格线的宽度，如果为0表示不显示纵向网格线
    * @param [in] bNeedDpiScale 如果为true表示需要对宽度进行DPI自适应
    */
    void SetColumnGridLineWidth(float nLineWidth, bool bNeedDpiScale);
    float GetColumnGridLineWidth() const;

    /** 纵向网格线的颜色
    * @param [in] color 纵向网格线的颜色
    */
    void SetColumnGridLineColor(const DString& color);
    DString GetColumnGridLineColor() const;

    /** 表头的Class
    */
    void SetHeaderClass(const DString& headerClass);
    DString GetHeaderClass() const;

    /** 分组的Class
    */
    void SetGroupClass(const DString& groupClass);
    DString GetGroupClass() const;

    /** 分组的文本控件Class
    */
    void SetGroupLabelClass(const DString& groupLabelClass);
    DString GetGroupLabelClass() const;

    /** 属性的Class
    */
    void SetPropertyClass(const DString& propertyClass);
    DString GetPropertyClass() const;

    /** 属性的名称文本控件Class
    */
    void SetPropertyNameLabelClass(const DString& propertyNameLabelClass);
    DString GetPropertyNameLabelClass() const;

    /** 属性的值文本控件Class
    */
    void SetPropertyValueLabelClass(const DString& propertyValueLabelClass);
    DString GetPropertyValueLabelClass() const;

protected:
    /** 初始化函数
     */
    virtual void OnInit() override;

    /** 绘制子控件
    */
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

    /** 绘制网格线
    */
    void PaintGridLines(IRender* pRender);

    /** 拖动列表头改变列宽的事件响应函数
    */
    void OnHeaderColumnResized();

    /** 调整一个属性节点的列宽
    */
    void ResizePropertyColumn(TreeNode* pPropertyNode, int32_t nLeftColumnWidth);

private:
    /** 获取左侧列宽的值
    */
    int32_t GetLeftColumnWidthValue() const;

private:
    /** 配置XML文件
    */
    UiString m_configXml;

private:
    /** 是否显示Header
    */
    bool m_bHeaderCtrl;

    /** Header接口
    */
    Control* m_pHeaderCtrl;

    /** Header中的左侧一列
    */
    Label* m_pHeaderLeft;

    /** Header中的右侧一列
    */
    Label* m_pHeaderRight;

    /** Header中的分割条
    */
    Split* m_pHeaderSplit;

    /** 左侧一列的宽度
    */
    int32_t m_nLeftColumnWidth;

private:
    /** 描述控件的分割条
    */
    Split* m_pDescriptionAreaSplit;

    /** 描述控件
    */
    RichText* m_pDescriptionArea;

    /** 是否显示描述区域
    */
    bool m_bDescriptionArea;

private:
    /** 属性表的树控件接口
    */
    TreeView* m_pTreeView;

    /** 表头的Class
    */
    UiString m_headerClass;

    /** 分组的Class
    */
    UiString m_groupClass;

    /** 分组的Class
    */
    UiString m_groupLabelClass;

    /** 属性的Class
    */
    UiString m_propertyClass;

    /** 属性的名称文本控件Class
    */
    UiString m_propertyNameLabelClass;

    /** 属性的值文本控件Class
    */
    UiString m_propertyValueLabelClass;

private:
    /** 属性值的字体Id（正常状态）
    */
    UiString m_propertyNormalFontId;

    /** 属性值的字体Id（已修改状态）
    */
    UiString m_propertyModifiedFontId;

private:
    /** 横向网格线的宽度
    */
    float m_fRowGridLineWidth;

    /** 横向网格线的颜色
    */
    UiString m_rowGridLineColor;

    /** 纵向网格线的宽度
    */
    float m_fColumnGridLineWidth;

    /** 纵向网格线的颜色
    */
    UiString m_columnGridLineColor;
};

/** 属性表的分组, 基本结构
*   <PropertyGridGroup>
*        <HBox>
*            <LabelBox/>
*        </HBox>
*   </PropertyGridGroup>
*/
class DUILIB_API PropertyGridGroup : public TreeNode
{
    typedef TreeNode BaseClass;
public:
    /** 构造一个组
    * @param [in] pWindow 关联的窗口
    */
    explicit PropertyGridGroup(Window* pWindow);

    /** 构造一个组（不支持多语言版）
    * @param [in] groupName 组的名称
    * @param [in] description 组的描述信息
    * @param [in] nGroupData 用户自定义数据
    */
    PropertyGridGroup(Window* pWindow,
                      const DString& groupName,
                      const DString& description = _T(""),
                      size_t nGroupData = 0);

    /** 构造一个组（支持多语言版）
    * @param [in] bGroupNameId true表示groupName为语言ID（支持多语言版），false表示为普通文本
    * @param [in] groupName 组的名称
    * @param [in] bDescriptionId true表示description为语言ID（支持多语言版），false表示为普通文本
    * @param [in] description 组的描述信息    
    * @param [in] nGroupData 用户自定义数据
    */
    PropertyGridGroup(Window* pWindow,
                      bool bGroupNameId,
                      const DString& groupName,
                      bool bDescriptionId,
                      const DString& description,
                      size_t nGroupData = 0);

public:
    /** 获取属性名称
    */
    DString GetGroupName() const;

    /** 设置属性名称文本
    */
    void SetGroupName(const DString& groupName);

    /** 获取属性名称ID
    */
    DString GetGroupNameId() const;

    /** 设置属性名称文本ID
    */
    void SetGroupNameId(const DString& groupNameId);

    /** 获取组的描述信息
    */
    DString GetDescription() const;

    /** 设置组的描述属性
    */
    void SetDescription(const DString& description);

    /** 获取组的描述信息ID
    */
    DString GetDescriptionId() const;

    /** 设置组的描述属性ID
    */
    void SetDescriptionId(const DString& descriptionId);

    /** 获取用户自定义数据
    */
    size_t GetGroupData() const { return m_nGroupData; }

    /** 设置用户自定义数据
    */
    void SetGroupData(size_t nGroupData) { m_nGroupData = nGroupData; }

    /** 获取属性名称的显示控件
    */
    LabelBox* GetLabelBox() const { return m_pLabelBox.get(); }

    /** 获取分组下的所有属性
    * @param [out] properties 返回当前所有的分组列表
    */
    void GetProperties(std::vector<PropertyGridProperty*>& properties) const;

    /** 删除该分组下的属性
    * @param [in] pProperty 需要删除的属性接口
    */
    bool RemoveProperty(PropertyGridProperty* pProperty);

    /** 删除该分组下的所有属性
    */
    void RemoveAllProperties();

    /** 设置关联的属性页接口
    */
    void SetPropertyGrid(PropertyGrid* pPropertyGrid);

    /** 获取关联的属性页接口
    */
    PropertyGrid* GetPropertyGrid() const;

protected:
    /** 初始化函数
     */
    virtual void OnInit() override;

private:
    /** 关联的属性页接口
    */
    ControlPtrT<PropertyGrid> m_pPropertyGrid;

    /** 分组的名称
    */
    UiString m_groupName;

    /** 分组名称的多语版ID
    */
    UiString m_groupNameId;

    /** 组的描述信息
    */
    UiString m_description;

    /** 组的描述信息的多语版ID
    */
    UiString m_descriptionId;

    /** 用户自定义数据
    */
    size_t m_nGroupData;

    /** 属性名称的显示控件
    */
    ControlPtrT<LabelBox> m_pLabelBox;
};

/** 属性的类型
*/
enum class PropertyGridPropertyType
{
    kNone,        //无具体类型，基类
    kText,        //普通文本
    kCombo,        //下拉框
    kFont,        //字体名称
    kFontSize,  //字体大小
    kColor,        //颜色
    kDateTime,    //日期时间
    kIPAddress,    //IP地址
    kHotKey,    //热键
    kFile,        //文件路径
    kDirectory,    //文件夹
    kCustom        //用户自定义的类型，比如自己实现一个子类
};

/** 属性表的属性, 基本结构
*   <PropertyGridProperty>
*        <HBox>
*            <LabelBox/>
*            <LabelBox/>
*        </HBox>
*   </PropertyGridProperty>
*/
class DUILIB_API PropertyGridProperty: public TreeNode
{
    typedef TreeNode BaseClass;
public:
    /** 构造一个属性
    * @param [in] pWindow 关联的窗口
    */
    explicit PropertyGridProperty(Window* pWindow);

    /** 构造一个属性(不支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridProperty(Window* pWindow, 
                         const DString& propertyName,
                         const DString& propertyValue,
                         const DString& description = _T(""),
                         size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] bPropertyNameId true表示propertyName为文本ID, false表示为普通文本
    * @param [in] propertyName 属性的名称
    * @param [in] bPropertyValueId true表示propertyValue为文本ID, false表示为普通文本
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] bDescriptionId true表示description为文本ID, false表示为普通文本
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridProperty(Window* pWindow,
                         bool bPropertyNameId,
                         const DString& propertyName,
                         bool bPropertyValueId,
                         const DString& propertyValue,
                         bool bDescriptionId,
                         const DString& description,
                         size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] param 关联的参数
    */
    PropertyGridProperty(Window* pWindow, const PropertyGridParam& param);

public:
    /** 获取属性类型
    */
    virtual PropertyGridPropertyType GetPropertyType() const 
    { 
        return PropertyGridPropertyType::kNone; 
    }

    /** 获取属性名称
    */
    DString GetPropertyName() const;

    /** 设置属性名称 
    */
    void SetPropertyName(const DString& propertyName);

    /** 获取属性名称ID(支持多语言版)
    */
    DString GetPropertyNameId() const;

    /** 设置属性名称ID(支持多语言版)
    */
    void SetPropertyNameId(const DString& propertyNameId);

    /** 获取属性值(原值)
    */
    DString GetPropertyValue() const;

    /** 设置属性值(原值)
    */
    void SetPropertyValue(const DString& propertyValue);

    /** 获取属性值ID(原值)(支持多语言版)
    */
    DString GetPropertyValueId() const;

    /** 设置属性值ID(原值)(支持多语言版)
    */
    void SetPropertyValueId(const DString& propertyValueId);

    /** 当前是否正在使用属性值ID
    */
    bool UsingPropertyValueId() const;

    /** 获取属性的描述信息
    */
    DString GetDescription() const;

    /** 设置属性的描述信息
    */
    void SetDescription(const DString& description);

    /** 获取属性的描述信息ID(支持多语言版)
    */
    DString GetDescriptionId() const;

    /** 设置属性的描述信息ID(支持多语言版)
    */
    void SetDescriptionId(const DString& descriptionId);

    /** 获取用户自定义数据
    */
    size_t GetPropertyData() const { return m_nPropertyData; }

    /** 设置用户自定义数据
    */
    void SetPropertyData(size_t nPropertyData) { m_nPropertyData = nPropertyData; }

    /** 获取属性名称和属性值所在容器控件，可用于设置背景色等
    */
    HBox* GetHBox() const { return m_pHBox.get(); }

    /** 获取属性名称的显示控件, 父控件是GetHBox()
    */
    LabelBox* GetLabelBoxLeft() const { return m_pLabelBoxLeft.get(); }

    /** 获取属性值的显示控件, 父控件是GetHBox()
    */
    LabelBox* GetLabelBoxRight() const { return m_pLabelBoxRight.get(); }

    /** 设置只读模式
    */
    void SetReadOnly(bool bReadOnly);

    /** 是否为只读模式
    */
    bool IsReadOnly() const { return m_bReadOnly; }

    /** 获取新的属性值（修改后的属性值, 如果无修改则返回原值）
    */
    virtual DString GetPropertyNewValue() const;

    /** 设置关联的属性页接口
    */
    void SetPropertyGrid(PropertyGrid* pPropertyGrid);

    /** 获取关联的属性页接口
    */
    PropertyGrid* GetPropertyGrid() const;

    /** 监听属性值变化事件（以文本形式通知属性值的变化）
    * @param [in] callback 要绑定的回调函数，wParam是(WPARAM)&oldText，lParam是(LPARAM)&newText，oldText和newText变量类型都是DString
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachValueChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventValueChanged, callback, callbackID); }

protected:
    /** 初始化函数
     */
    virtual void OnInit() override;

    /** 设置是否允许存在编辑框控件
    * @param [in] bEnable true表示允许存在编辑框控件，false表示不允许存在编辑框控件
    */
    virtual void EnableEditControl(bool /*bEnable*/) {}

    /** 显示或者隐藏编辑框控件
    * @param [in] bShow 表示显示编辑控件，false表示隐藏编辑控件
    * @param [in] bCancel true表示取消编辑，false表示应用编辑
    * @return 返回编辑控件的接口
    */
    virtual Control* ShowEditControl(bool /*bShow*/, bool /*bCancel*/) { return nullptr; }

    /** 旧的属性值发生了变化
    */
    virtual void OnPropertyValueChanged() {}

    /** 属性值发生了变化（编辑触发）
    * @param [in] oldText 旧的文本内容
    * @param [in] newText 新的文本内容
    * @return 返回true表示继续触发属性值变化事件，返回false表示拦截属性值变化事件
    */
    virtual bool OnPropertyTextChanged(const DString& /*oldText*/, const DString& /*newText*/) { return true; }

    /** 滚动条发生了滚动(用于处理弹出式子窗口的位置问题)
    */
    virtual void OnScrollPosChanged() {}

    /** 获取编辑控件的Margin.right（避免滚动条遮挡编辑控件）
    */
    int32_t GetEditControlMarginRight() const;

protected:
    /** 设置属性值的文本(显示控件)，用于编辑属性后更新显示控件的文本内容
    * @param [in] text 文本内容
    * @param [in] bChanged 是否标记为变化
    * @param [in] bTriggerEvent 是否触发变化事件（回调函数）
    */
    void SetPropertyText(const DString& text, bool bChanged, bool bTriggerEvent = true);

    /** 获取属性值文本(显示控件)
    */
    DString GetPropertyText() const;

    /** 设置属性值的文字颜色(显示控件)
    * @param [in] text 文本内容
    */
    void SetPropertyTextColor(const DString& textColor);

    /** 将焦点设置到属性值文本显示控件
    */
    void SetPropertyFocus();

    /** 在属性值的LabelBox中添加控件
    */
    bool AddPropertySubItem(Control* pControl);

    /** 在属性值的LabelBox中移除控件
    */
    bool RemovePropertySubItem(Control* pControl);

    /** 判断属性值的LabelBox中是否包含控件
    */
    bool HasPropertySubItem(Control* pControl) const;

private:
    /** 关联的属性页接口
    */
    ControlPtrT<PropertyGrid> m_pPropertyGrid;

    /** 属性的名称
    */
    UiString m_propertyName;

    /** 属性的名称ID
    */
    UiString m_propertyNameId;

    /** 属性的值
    */
    UiString m_propertyValue;

    /** 属性的值ID
    */
    UiString m_propertyValueId;

    /** 属性的描述信息
    */
    UiString m_description;

    /** 属性的描述信息ID
    */
    UiString m_descriptionId;

    /** 用户自定义数据
    */
    size_t m_nPropertyData;

    /** 属性名称和属性值所在容器控件
    */
    ControlPtrT<HBox> m_pHBox;

    /** 属性名称的显示控件
    */
    ControlPtrT<LabelBox> m_pLabelBoxLeft;

    /** 属性值的显示控件
    */
    ControlPtrT<LabelBox> m_pLabelBoxRight;

    /** 只读模式
    */
    bool m_bReadOnly;
};

/** 文本类型的属性：使用RichEdit编辑
*/
class DUILIB_API PropertyGridTextProperty : public PropertyGridProperty
{
    typedef PropertyGridProperty BaseClass;
public:
    /** 构造一个属性
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridTextProperty(Window* pWindow,
                             const DString& propertyName,
                             const DString& propertyValue,
                             const DString& description = _T(""),
                             size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] bPropertyNameId true表示propertyName为文本ID, false表示为普通文本
    * @param [in] propertyName 属性的名称
    * @param [in] bPropertyValueId true表示propertyValue为文本ID, false表示为普通文本
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] bDescriptionId true表示description为文本ID, false表示为普通文本
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridTextProperty(Window* pWindow,
                             bool bPropertyNameId,
                             const DString& propertyName,
                             bool bPropertyValueId,
                             const DString& propertyValue,
                             bool bDescriptionId,
                             const DString& description,
                             size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] param 关联的参数
    */
    PropertyGridTextProperty(Window* pWindow, const PropertyGridParam& param);

public:
    /** 获取属性类型
    */
    virtual PropertyGridPropertyType GetPropertyType() const override
    {
        return PropertyGridPropertyType::kText;
    }

    /** 获取新的属性值（修改后的属性值, 如果无修改则返回原值）
    */
    virtual DString GetPropertyNewValue() const override;

    /** 获取编辑框控件
    */
    RichEdit* GetRichEdit() const { return m_pRichEdit.get(); }

    /** 设置新的文本
    * @param [in] newText 新的文本内容
    */
    void SetNewTextValue(const DString& newText);

    /** 设置密码模式（显示 ***）
     * @param[in] bPasswordMode 设置为 true 让控件显示内容为 ***，false 为显示正常内容
     */
    void SetPasswordMode(bool bPasswordMode);

    /** 是否为密码模式
    */
    bool IsPasswordMode() const { return m_bPasswordMode; }

    /** 设置是否支持Spin控件
    * @param [in] bEnable true表示支持Spin控件，false表示不支持Spin控件
    * @param [in] nMin 表示设置数字的最小值
    * @param [in] nMax 表示设置数字的最大值，如果 nMin和nMax同时为0, 表示不设置数字的最小值和最大值
    */
    void SetEnableSpin(bool bEnable, int32_t nMin = 0, int32_t nMax = 0);

protected:
    /** 设置是否允许存在编辑框控件
    * @param [in] bEnable true表示允许存在编辑框控件，false表示不允许存在编辑框控件
    */
    virtual void EnableEditControl(bool bEnable) override;

    /** 显示或者隐藏编辑框控件
    * @param [in] bShow 表示显示编辑控件，false表示隐藏编辑控件
    * @param [in] bCancel true表示取消编辑，false表示应用编辑
    * @return 返回编辑控件的接口
    */
    virtual Control* ShowEditControl(bool bShow, bool bCancel) override;

    /** 语言发生变化，刷新界面文字显示相关的内容
    * @param [in] bRedraw true表示需要内部实现重绘，否则控件内部不需要重绘，由外部调用重绘
    */
    virtual void OnLanguageChanged(bool bRedraw) override;

    /** 旧的属性值发生了变化
    */
    virtual void OnPropertyValueChanged() override;

private:
    /** 编辑框控件(用于修改属性)
    */
    ControlPtrT<RichEdit> m_pRichEdit;

    /** 编辑前的旧值
    */
    UiString m_oldText;

    /** 密码模式
    */
    bool m_bPasswordMode;

    /** 是否有编辑过
    */
    bool m_bTextEdited;
};

/** 下拉框类型的属性：使用Combo编辑
*/
class DUILIB_API PropertyGridComboProperty : public PropertyGridProperty
{
    typedef PropertyGridProperty BaseClass;
public:
    /** 构造一个属性
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridComboProperty(Window* pWindow, 
                              const DString& propertyName,
                              const DString& propertyValue,
                              const DString& description = _T(""),
                              size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] bPropertyNameId true表示propertyName为文本ID, false表示为普通文本
    * @param [in] propertyName 属性的名称
    * @param [in] bPropertyValueId true表示propertyValue为文本ID, false表示为普通文本
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] bDescriptionId true表示description为文本ID, false表示为普通文本
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridComboProperty(Window* pWindow,
                              bool bPropertyNameId,
                              const DString& propertyName,
                              bool bPropertyValueId,
                              const DString& propertyValue,
                              bool bDescriptionId,
                              const DString& description,
                              size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] param 关联的参数
    */
    PropertyGridComboProperty(Window* pWindow, const PropertyGridParam& param);

public:
    /** 获取属性类型
    */
    virtual PropertyGridPropertyType GetPropertyType() const override
    {
        return PropertyGridPropertyType::kCombo;
    }

    /** 获取新的属性值（修改后的属性值, 如果无修改则返回原值）
    */
    virtual DString GetPropertyNewValue() const override;

    /** 增加一个下拉框选项
    * @param [in] optionText 下拉框列表项的内容
    * @return 返回该子项的下标值
    */
    size_t AddOption(const DString& optionText);

    /** 增加一个下拉框选项(支持多语言版)
    * @param [in] optionTextId 下拉框列表项的内容ID
    * @return 返回该子项的下标值
    */
    size_t AddOptionById(const DString& optionTextId);

    /** 获取下拉框选项的格式
    */
    size_t GetOptionCount() const;

    /** 获取下拉表子项的文本
    * @param [in] nIndex 子项的下标值，有效范围：[0, GetOptionCount())
    */
    DString GetOption(size_t nIndex) const;

    /** 设置子项关联的数据
    * @param [in] nIndex 子项的下标值，有效范围：[0, GetOptionCount())
    * @param [in] nOptionData 关联数据
    */
    void SetOptionData(size_t nIndex, size_t nOptionData);

    /** 获取子项关联的数据
    * @param [in] nIndex 子项的下标值，有效范围：[0, GetOptionCount())
    */
    size_t GetOptionData(size_t nIndex) const;

    /** 获取是否包含子项关联的数据
    * @param [in] nIndex 子项的下标值，有效范围：[0, GetOptionCount())
    */
    bool HasOptionData(size_t nIndex) const;

    /** 删除指定的子项
    * @param [in] nIndex 子项的下标值，有效范围：[0, GetOptionCount())
    */
    bool RemoveOption(size_t nIndex);

    /** 删除所有子项
    */
    void RemoveAllOptions();

    /** 获取当前选择项索引
     * @return 返回当前选择项索引, (如果无有效索引，则返回Box::InvalidIndex)
     */
    size_t GetCurSel() const;

    /** 选择一个子项, 触发选择事件
     * @param [in] nIndex 要选择的子项索引，有效范围：[0, GetOptionCount())
     * @param [in] bTriggerEvent true表示触发变化事件，false表示不触发变化事件
     * @return 返回 true 表示成功，否则为 false
     */
    bool SetCurSel(size_t nIndex, bool bTriggerEvent = true);

    /** 设置为列表模式
    * @param [in] bListMode true表示不支持编辑文本，只能从下拉表中选择；false表示允许编辑，允许选择
    */
    void SetComboListMode(bool bListMode);

    /** 同步列表选项内容到显示文本（保持选择与显示一致）
    * @param [in] bTriggerEvent 当属性值相比原值变化时，是否触发属性值变化事件
    */
    void UpdateEditText(bool bTriggerEvent);

    /** 当前是否正在编辑状态
    */
    bool IsComboEditing() const;

    /** 获取下拉框接口
    */
    Combo* GetCombo() const { return m_pCombo.get(); }

protected:
    /** 设置是否允许存在编辑框控件
    * @param [in] bEnable true表示允许存在编辑框控件，false表示不允许存在编辑框控件
    */
    virtual void EnableEditControl(bool bEnable) override;

    /** 显示或者隐藏编辑框控件
    * @param [in] bShow 表示显示编辑控件，false表示隐藏编辑控件
    * @param [in] bCancel true表示取消编辑，false表示应用编辑
    * @return 返回编辑控件的接口
    */
    virtual Control* ShowEditControl(bool bShow, bool bCancel) override;

    /** 滚动条发生了滚动(用于处理弹出式子窗口的位置问题)
    */
    virtual void OnScrollPosChanged() override;

    /** 语言发生变化，刷新界面文字显示相关的内容
    * @param [in] bRedraw true表示需要内部实现重绘，否则控件内部不需要重绘，由外部调用重绘
    */
    virtual void OnLanguageChanged(bool bRedraw) override;

    /** 旧的属性值发生了变化
    */
    virtual void OnPropertyValueChanged() override;

private:
    /** 下拉框接口
    */
    ControlPtrT<Combo> m_pCombo;

    /** 编辑前，原来的显示文本（以支持取消编辑）
    */
    UiString m_oldText;

    /** 编辑前，原来选择的列表项
    */
    size_t m_oldSelItem;

    /** 是否触发编辑操作
    */
    bool m_bComboEdited;

    /** 当前是否正在编辑状态
    */
    bool m_bComboEditing;
};

/** 设置字体名称的属性
*/
class DUILIB_API PropertyGridFontProperty : public PropertyGridComboProperty
{
    typedef PropertyGridComboProperty BaseClass;
public:
    /** 构造一个属性
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值(原字体名称)
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridFontProperty(Window* pWindow,
                             const DString& propertyName,
                             const DString& propertyValue,
                             const DString& description = _T(""),
                             size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] bPropertyNameId true表示propertyName为文本ID, false表示为普通文本
    * @param [in] propertyName 属性的名称
    * @param [in] bPropertyValueId true表示propertyValue为文本ID, false表示为普通文本
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] bDescriptionId true表示description为文本ID, false表示为普通文本
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridFontProperty(Window* pWindow,
                             bool bPropertyNameId,
                             const DString& propertyName,
                             bool bPropertyValueId,
                             const DString& propertyValue,
                             bool bDescriptionId,
                             const DString& description,
                             size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] param 关联的参数
    */
    PropertyGridFontProperty(Window* pWindow, const PropertyGridParam& param);

public:
    /** 获取属性类型
    */
    virtual PropertyGridPropertyType GetPropertyType() const override
    {
        return PropertyGridPropertyType::kFont;
    }

    /** 获取新的字体值（修改后的属性值, 如果无修改则返回原值）
    */
    virtual DString GetPropertyNewValue() const override;

    /** 是否校验字体名称是否合理（在列表中的字体名称为合法值，其他为非法值）
    * @param [in] bValidation true表示检查新值是否合理（默认值），false表示不检查
    */
    void SetFontNameValidation(bool bValidation);

protected:

    /** 初始化函数
     */
    virtual void OnInit() override;

    /** 属性值发生了变化（编辑触发）
    * @param [in] oldText 旧的文本内容
    * @param [in] newText 新的文本内容
    * @return 返回true表示继续触发属性值变化事件，返回false表示拦截属性值变化事件
    */
    virtual bool OnPropertyTextChanged(const DString& oldText, const DString& newText) override;

private:
    /** 字体列表
    */
    std::vector<DString> m_fontNameList;

    /** 当前是否正在校验属性值的有效性
    */
    bool m_bCheckingNewValue;

    /** 是否检查字体名称是否合法
    */
    bool m_bFontNameValidation;
};

/** 设置字体大小的属性
*/
class DUILIB_API PropertyGridFontSizeProperty : public PropertyGridComboProperty
{
    typedef PropertyGridComboProperty BaseClass;
public:
    /** 构造一个属性
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值(原字体名称)
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridFontSizeProperty(Window* pWindow, 
                                 const DString& propertyName,
                                 const DString& propertyValue,
                                 const DString& description = _T(""),
                                 size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] bPropertyNameId true表示propertyName为文本ID, false表示为普通文本
    * @param [in] propertyName 属性的名称
    * @param [in] bPropertyValueId true表示propertyValue为文本ID, false表示为普通文本
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] bDescriptionId true表示description为文本ID, false表示为普通文本
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridFontSizeProperty(Window* pWindow,
                                 bool bPropertyNameId,
                                 const DString& propertyName,
                                 bool bPropertyValueId,
                                 const DString& propertyValue,
                                 bool bDescriptionId,
                                 const DString& description,
                                 size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] param 关联的参数
    */
    PropertyGridFontSizeProperty(Window* pWindow, const PropertyGridParam& param);

public:
    /** 获取属性类型
    */
    virtual PropertyGridPropertyType GetPropertyType() const override
    {
        return PropertyGridPropertyType::kFontSize;
    }

    /** 获取新的字体大小值, 显示值（修改后的属性值, 如果无修改则返回原值）
    */
    virtual DString GetPropertyNewValue() const override;

public:
    /** 设置字体大小列表（可以覆盖内置的默认列表），字体大小值未进行DPI缩放
    * @param [in] fontSizeList 字体大小列表
    */
    void SetFontSizeList(const std::vector<FontSizeInfo>& fontSizeList);

    /** 获取字体大小列表，字体大小值未进行DPI缩放
    * @param [out] fontSizeList 字体大小列表
    */
    void GetFontSizeList(std::vector<FontSizeInfo>& fontSizeList) const;

    /** 获取字体大小列表，字体大小值已经完成进行DPI缩放
    * @param [out] fontSizeList 字体大小列表
    */
    void GetDpiFontSizeList(std::vector<FontSizeInfo>& dpiFontSizeList) const;

    /** 重新填充字体列表（删除原有的值，填充新的值）
    */
    void UpdateFontSizeOptionList();

    /** 是否校验字体大小是否合理（在列表中的字体大小为合法值，其他为非法值）
    * @param [in] bValidation true表示检查新值是否合理（默认值），false表示不检查
    */
    void SetFontSizeValidation(bool bValidation);

public:
    /** 获取字体大小值，浮点数，未做DPI自适应值
    * @return 如果从列表中选择，返回值为非空；如果未能从列表中选择，则返回空
    */
    DString GetFontSize() const;

    /** 获取字体大小值，浮点数，已做DPI自适应值
    * @return 如果从列表中选择，返回值为非空；如果未能从列表中选择，则返回空
    */
    DString GetDpiFontSize() const;

    /** 获取字体大小显示名称对应的字体大小值，浮点数，未做DPI自适应值
    * @param [in] fontSizeName 比如："五号"
    */
    DString GetFontSize(const DString& fontSizeName) const;

    /** 获取字体大小显示名称对应的字体大小值，浮点数，未做DPI自适应值
    * @param [in] fontSizeNameId 字体名称的语言ID
    */
    DString GetFontSizeById(const DString& fontSizeNameId) const;

    /** 获取字体大小显示名称对应的字体大小值，浮点数，已做DPI自适应值
    * @param [in] fontSizeName 比如："五号"
    */
    DString GetDpiFontSize(const DString& fontSizeName) const;

    /** 获取字体大小显示名称对应的字体大小值，浮点数，已做DPI自适应值
    * @param [in] fontSizeNameId 字体名称的语言ID
    */
    DString GetDpiFontSizeById(const DString& fontSizeNameId) const;

protected:

    /** 初始化函数
     */
    virtual void OnInit() override;

    /** 语言发生变化，刷新界面文字显示相关的内容
    * @param [in] bRedraw true表示需要内部实现重绘，否则控件内部不需要重绘，由外部调用重绘
    */
    virtual void OnLanguageChanged(bool bRedraw) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 属性值发生了变化（编辑触发）
    * @param [in] oldText 旧的文本内容
    * @param [in] newText 新的文本内容
    * @return 返回true表示继续触发属性值变化事件，返回false表示拦截属性值变化事件
    */
    virtual bool OnPropertyTextChanged(const DString& oldText, const DString& newText) override;

private:
    /** 字体大小(填充到List)
    */
    std::vector<FontSizeInfo> m_fillFontSizeList;

    /** 字体大小(外部设置)
    */
    std::vector<FontSizeInfo> m_externfontSizeList;

    /** 当前是否正在校验属性值的有效性
    */
    bool m_bCheckingNewValue;

    /** 是否检查字体大小是否合法
    */
    bool m_bFontSizeValidation;
};

/** 设置颜色的属性
*/
class DUILIB_API PropertyGridColorProperty : public PropertyGridProperty
{
public:
    /** 构造一个属性
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值(原字体名称)
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridColorProperty(Window* pWindow, 
                              const DString& propertyName,
                              const DString& propertyValue,
                              const DString& description = _T(""),
                              size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] bPropertyNameId true表示propertyName为文本ID, false表示为普通文本
    * @param [in] propertyName 属性的名称
    * @param [in] bPropertyValueId true表示propertyValue为文本ID, false表示为普通文本
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] bDescriptionId true表示description为文本ID, false表示为普通文本
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridColorProperty(Window* pWindow,
                              bool bPropertyNameId,
                              const DString& propertyName,
                              bool bPropertyValueId,
                              const DString& propertyValue,
                              bool bDescriptionId,
                              const DString& description,
                              size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] param 关联的参数
    */
    PropertyGridColorProperty(Window* pWindow, const PropertyGridParam& param);

public:
    /** 获取属性类型
    */
    virtual PropertyGridPropertyType GetPropertyType() const override
    {
        return PropertyGridPropertyType::kColor;
    }

    /** 获取颜色选择控件接口
    */
    ComboButton* GetComboButton() const { return m_pComboButton.get(); }

protected:
    /** 设置是否允许存在编辑框控件
    * @param [in] bEnable true表示允许存在编辑框控件，false表示不允许存在编辑框控件
    */
    virtual void EnableEditControl(bool bEnable) override;

    /** 显示或者隐藏编辑框控件
    * @param [in] bShow 表示显示编辑控件，false表示隐藏编辑控件
    * @param [in] bCancel true表示取消编辑，false表示应用编辑
    * @return 返回编辑控件的接口
    */
    virtual Control* ShowEditControl(bool bShow, bool bCancel) override;

    /** 滚动条发生了滚动(用于处理弹出式子窗口的位置问题)
    */
    virtual void OnScrollPosChanged() override;

private:
    /** 初始化设置颜色的Combo按钮
    */
    void InitColorCombo();

    /** 显示拾色器窗口
    */
    void ShowColorPicker();

    /** 设置选择颜色
    */
    void OnSelectColor(const DString& color);

private:
    /** 颜色选择控件
    */
    ControlPtrT<ComboButton> m_pComboButton;

    /** 旧值
    */
    UiString m_oldColor;
};

/** 设置日期时间的属性(仅Windows平台提供此功能)
*/
class DUILIB_API PropertyGridDateTimeProperty : public PropertyGridProperty
{
public:
    /** 构造一个属性
    * @param [in] propertyName 属性的名称
    * @param [in] dateTimeValue 日期时间的值
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] editFormat 日期时间的格式
    */
    PropertyGridDateTimeProperty(Window* pWindow, 
                                 const DString& propertyName,
                                 const DString& dateTimeValue,
                                 const DString& description = _T(""),
                                 size_t nPropertyData = 0,
                                 DateTime::EditFormat editFormat = DateTime::EditFormat::kDateCalendar);


    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] bPropertyNameId true表示propertyName为文本ID, false表示为普通文本
    * @param [in] propertyName 属性的名称
    * @param [in] bDateTimeValueId true表示dateTimeValue为文本ID, false表示为普通文本
    * @param [in] dateTimeValue 日期时间的值
    * @param [in] description 属性的描述信息
    * @param [in] bDescriptionId true表示description为文本ID, false表示为普通文本
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] editFormat 日期时间的格式
    */
    PropertyGridDateTimeProperty(Window* pWindow,
                                 bool bPropertyNameId,
                                 const DString& propertyName,
                                 bool bDateTimeValueId,
                                 const DString& dateTimeValue,
                                 bool bDescriptionId,
                                 const DString& description,
                                 size_t nPropertyData = 0,
                                 DateTime::EditFormat editFormat = DateTime::EditFormat::kDateCalendar);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] param 关联的参数
    * @param [in] editFormat 日期时间的格式
    */
    PropertyGridDateTimeProperty(Window* pWindow, const PropertyGridParam& param,
                                 DateTime::EditFormat editFormat = DateTime::EditFormat::kDateCalendar);

public:
    /** 获取属性类型
    */
    virtual PropertyGridPropertyType GetPropertyType() const override
    {
        return PropertyGridPropertyType::kDateTime;
    }

    /** 获取日期时间控件接口
    */
    DateTime* GetDateTime() const { return m_pDateTime.get(); }

protected:
    /** 设置是否允许存在编辑框控件
    * @param [in] bEnable true表示允许存在编辑框控件，false表示不允许存在编辑框控件
    */
    virtual void EnableEditControl(bool bEnable) override;

    /** 显示或者隐藏编辑框控件
    * @param [in] bShow 表示显示编辑控件，false表示隐藏编辑控件
    * @param [in] bCancel true表示取消编辑，false表示应用编辑
    * @return 返回编辑控件的接口
    */
    virtual Control* ShowEditControl(bool bShow, bool bCancel) override;

    /** 滚动条发生了滚动(用于处理弹出式子窗口的位置问题)
    */
    virtual void OnScrollPosChanged() override;

private:
    /** 日期时间控件接口
    */
    ControlPtrT<DateTime> m_pDateTime;

    /** 日期时间的格式
    */
    DateTime::EditFormat m_editFormat;
};

/** 设置IP地址的属性
*/
class DUILIB_API PropertyGridIPAddressProperty : public PropertyGridProperty
{
public:
    /** 构造一个属性
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridIPAddressProperty(Window* pWindow, 
                                  const DString& propertyName,
                                  const DString& propertyValue,
                                  const DString& description = _T(""),
                                  size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] bPropertyNameId true表示propertyName为文本ID, false表示为普通文本
    * @param [in] propertyName 属性的名称
    * @param [in] bPropertyValueId true表示propertyValue为文本ID, false表示为普通文本
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] bDescriptionId true表示description为文本ID, false表示为普通文本
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridIPAddressProperty(Window* pWindow,
                                  bool bPropertyNameId,
                                  const DString& propertyName,
                                  bool bPropertyValueId,
                                  const DString& propertyValue,
                                  bool bDescriptionId,
                                  const DString& description,
                                  size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] param 关联的参数
    */
    PropertyGridIPAddressProperty(Window* pWindow, const PropertyGridParam& param);

public:
    /** 获取属性类型
    */
    virtual PropertyGridPropertyType GetPropertyType() const override
    {
        return PropertyGridPropertyType::kIPAddress;
    }

    /** 获取IP地址控件接口
    */
    IPAddress* GetIPAddress() const { return m_pIPAddress.get(); }

    /** 设置新的IP地址
    * @param [in] newIP 新的IP地址
    */
    void SetNewIPAddressValue(const DString& newIP);

protected:
    /** 设置是否允许存在编辑框控件
    * @param [in] bEnable true表示允许存在编辑框控件，false表示不允许存在编辑框控件
    */
    virtual void EnableEditControl(bool bEnable) override;

    /** 显示或者隐藏编辑框控件
    * @param [in] bShow 表示显示编辑控件，false表示隐藏编辑控件
    * @param [in] bCancel true表示取消编辑，false表示应用编辑
    * @return 返回编辑控件的接口
    */
    virtual Control* ShowEditControl(bool bShow, bool bCancel) override;

private:
    /** IP地址控件
    */
    ControlPtrT<IPAddress> m_pIPAddress;
};

/** 设置热键的属性
*/
class DUILIB_API PropertyGridHotKeyProperty : public PropertyGridProperty
{
public:
    /** 构造一个属性
    @param [in] propertyName 属性的名称
    @param [in] propertyValue 属性的值
    @param [in] description 属性的描述信息
    @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridHotKeyProperty(Window* pWindow, 
                               const DString& propertyName,
                               const DString& propertyValue,
                               const DString& description = _T(""),
                               size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] bPropertyNameId true表示propertyName为文本ID, false表示为普通文本
    * @param [in] propertyName 属性的名称
    * @param [in] bPropertyValueId true表示propertyValue为文本ID, false表示为普通文本
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] bDescriptionId true表示description为文本ID, false表示为普通文本
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridHotKeyProperty(Window* pWindow,
                               bool bPropertyNameId,
                               const DString& propertyName,
                               bool bPropertyValueId,
                               const DString& propertyValue,
                               bool bDescriptionId,
                               const DString& description,
                               size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] param 关联的参数
    */
    PropertyGridHotKeyProperty(Window* pWindow, const PropertyGridParam& param);

public:
    /** 获取属性类型
    */
    virtual PropertyGridPropertyType GetPropertyType() const override
    {
        return PropertyGridPropertyType::kHotKey;
    }

    /** 获取热键控件接口
    */
    HotKey* GetHotKey() const { return m_pHotKey.get(); }

protected:
    /** 设置是否允许存在编辑框控件
    * @param [in] bEnable true表示允许存在编辑框控件，false表示不允许存在编辑框控件
    */
    virtual void EnableEditControl(bool bEnable) override;

    /** 显示或者隐藏编辑框控件
    * @param [in] bShow 表示显示编辑控件，false表示隐藏编辑控件
    * @param [in] bCancel true表示取消编辑，false表示应用编辑
    * @return 返回编辑控件的接口
    */
    virtual Control* ShowEditControl(bool bShow, bool bCancel) override;

private:
    /** 热键控件
    */
    ControlPtrT<HotKey> m_pHotKey;
};

/** 设置文件路径属性
*/
class DUILIB_API PropertyGridFileProperty : public PropertyGridTextProperty
{
    typedef PropertyGridTextProperty BaseClass;
public:
    /** 构造一个属性
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值(文件的路径)
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] bOpenFileDialog true表示打开文件，false表示保存文件
    * @param [in] fileTypes 对话框可以打开或保存的文件类型
    * @param [in] nFileTypeIndex 选择的文件类型，有效范围：[0, fileTypes.size())
    * @param [in] defaultExt 默认的文件类型, 举例："doc;docx"
    */
    PropertyGridFileProperty(Window* pWindow, 
                             const DString& propertyName,
                             const DString& propertyValue,
                             const DString& description = _T(""),
                             size_t nPropertyData = 0,
                             bool bOpenFileDialog = true,
                             const std::vector<FileDialog::FileType>& fileTypes = std::vector<FileDialog::FileType>(),
                             int32_t nFileTypeIndex = -1,
                             const DString& defaultExt = _T(""));


    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] bPropertyNameId true表示propertyName为文本ID, false表示为普通文本
    * @param [in] propertyName 属性的名称
    * @param [in] bPropertyValueId true表示propertyValue为文本ID, false表示为普通文本
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] bDescriptionId true表示description为文本ID, false表示为普通文本
    * @param [in] nPropertyData 用户自定义数据
    * @param [in] bOpenFileDialog true表示打开文件，false表示保存文件
    * @param [in] fileTypes 对话框可以打开或保存的文件类型
    * @param [in] nFileTypeIndex 选择的文件类型，有效范围：[0, fileTypes.size())
    * @param [in] defaultExt 默认的文件类型, 举例："doc;docx"
    */
    PropertyGridFileProperty(Window* pWindow,
                            bool bPropertyNameId,
                            const DString& propertyName,
                            bool bPropertyValueId,
                            const DString& propertyValue,
                            bool bDescriptionId,
                            const DString& description,
                            size_t nPropertyData = 0,
                            bool bOpenFileDialog = true,
                            const std::vector<FileDialog::FileType>& fileTypes = std::vector<FileDialog::FileType>(),
                            int32_t nFileTypeIndex = -1,
                            const DString& defaultExt = _T(""));

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] param 关联的参数
    * @param [in] bOpenFileDialog true表示打开文件，false表示保存文件
    * @param [in] fileTypes 对话框可以打开或保存的文件类型
    * @param [in] nFileTypeIndex 选择的文件类型，有效范围：[0, fileTypes.size())
    * @param [in] defaultExt 默认的文件类型, 举例："doc;docx"
    */
    PropertyGridFileProperty(Window* pWindow, const PropertyGridParam& param,
                             bool bOpenFileDialog = true,
                             const std::vector<FileDialog::FileType>& fileTypes = std::vector<FileDialog::FileType>(),
                             int32_t nFileTypeIndex = -1,
                             const DString& defaultExt = _T(""));

protected:
    /** 获取属性类型
    */
    virtual PropertyGridPropertyType GetPropertyType() const override
    {
        return PropertyGridPropertyType::kFile;
    }

    /** 设置是否允许存在编辑框控件
    * @param [in] bEnable true表示允许存在编辑框控件，false表示不允许存在编辑框控件
    */
    virtual void EnableEditControl(bool bEnable) override;

    /** 点击了浏览按钮
    */
    virtual void OnBrowseButtonClicked();

private:
    /** 浏览按钮
    */
    ControlPtrT<Button> m_pBrowseBtn;

    /** true表示打开文件，false表示保存文件
    */
    bool m_bOpenFileDialog;

    /** 文件类型过滤器
    */
    std::vector<FileDialog::FileType> m_fileTypes;

    /** 选择的文件类型下标值
    */
    int32_t m_nFileTypeIndex;

    /** 默认的文件类型
    */
    DString m_defaultExt;
};

/** 设置文件夹属性
*/
class DUILIB_API PropertyGridDirectoryProperty : public PropertyGridTextProperty
{
    typedef PropertyGridTextProperty BaseClass;
public:
    /** 构造一个属性
    * @param [in] propertyName 属性的名称
    * @param [in] propertyValue 属性的值(文件夹路径)
    * @param [in] description 属性的描述信息
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridDirectoryProperty(Window* pWindow, 
                                  const DString& propertyName,
                                  const DString& propertyValue,
                                  const DString& description = _T(""),
                                  size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] bPropertyNameId true表示propertyName为文本ID, false表示为普通文本
    * @param [in] propertyName 属性的名称
    * @param [in] bPropertyValueId true表示propertyValue为文本ID, false表示为普通文本
    * @param [in] propertyValue 属性的值
    * @param [in] description 属性的描述信息
    * @param [in] bDescriptionId true表示description为文本ID, false表示为普通文本
    * @param [in] nPropertyData 用户自定义数据
    */
    PropertyGridDirectoryProperty(Window* pWindow,
                                  bool bPropertyNameId,
                                  const DString& propertyName,
                                  bool bPropertyValueId,
                                  const DString& propertyValue,
                                  bool bDescriptionId,
                                  const DString& description,
                                  size_t nPropertyData = 0);

    /** 构造一个属性(支持多语言版)
    * @param [in] pWindow 关联的窗口
    * @param [in] param 关联的参数
    */
    PropertyGridDirectoryProperty(Window* pWindow, const PropertyGridParam& param);

protected:
    /** 获取属性类型
    */
    virtual PropertyGridPropertyType GetPropertyType() const override
    {
        return PropertyGridPropertyType::kDirectory;
    }

    /** 设置是否允许存在编辑框控件
    * @param [in] bEnable true表示允许存在编辑框控件，false表示不允许存在编辑框控件
    */
    virtual void EnableEditControl(bool bEnable) override;

    /** 点击了浏览按钮
    */
    virtual void OnBrowseButtonClicked();

private:
    /** 浏览按钮
    */
    ControlPtrT<Button> m_pBrowseBtn;
};

}//namespace ui

#endif //UI_CONTROL_PROPERTY_GRID_H_
