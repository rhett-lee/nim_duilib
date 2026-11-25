// PropertyGrid 控件的单元测试
//
// 测试对象：ui::PropertyGridProperty / PropertyGridGroup 的数据模型部分。
// 本测试不创建窗口、不加载主题资源（不调用 OnInit），因此只验证与界面无关的逻辑：
//  1. 属性参数（PropertyGridParam）与多语言ID相关的构造与读取；
//  2. 属性的默认值、重置（ResetPropertyValue）、是否已修改（IsModified）；
//  3. 属性值校验（SetValueValidator/GetLastErrorText）与 SetPropertyText 的拦截行为；
//  4. SetPropertyNewValue（只读状态拦截、值写入、校验拦截后的回滚）；
//  5. 分组的名称/描述/自定义数据与分组级只读状态；
//  6. 新增属性类型（布尔/数值/多行文本）的类型标识与默认取值。

#if defined(_WIN32)
#include <crtdbg.h>
#endif

#include "duilib/Control/PropertyGrid.h"

#include <functional>
#include <iostream>
#include <string>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;

void Check(bool condition, const std::function<std::string()>& message)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << message() << std::endl;
    }
}

void Check(bool condition, const char* message)
{
    Check(condition, [message]() { return std::string(message); });
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })

/** 用于测试受保护接口的属性子类 */
class TestProperty : public ui::PropertyGridProperty
{
public:
    explicit TestProperty(ui::Window* pWindow) :
        ui::PropertyGridProperty(pWindow)
    {
    }

    TestProperty(ui::Window* pWindow, const DString& propertyName, const DString& propertyValue) :
        ui::PropertyGridProperty(pWindow, propertyName, propertyValue)
    {
    }

    using ui::PropertyGridProperty::SetPropertyText;
};

/** 属性参数与多语言ID */
void TestParam()
{
    ui::PropertyGridParam param;
    param.m_propertyName = _T("Name");
    param.m_propertyValue = _T("Value");
    param.m_description = _T("Description");
    param.m_nPropertyData = 100;

    ui::PropertyGridProperty property(nullptr, param);
    CHECK(property.GetPropertyName() == _T("Name"));
    CHECK(property.GetPropertyValue() == _T("Value"));
    CHECK(property.GetDescription() == _T("Description"));
    CHECK(property.GetPropertyData() == 100);
    CHECK(!property.UsingPropertyValueId());

    //值使用文本ID的形式
    ui::PropertyGridParam valueIdParam;
    valueIdParam.m_propertyNameId = _T("STRID_PROPERTY_NAME");
    valueIdParam.m_propertyValueId = _T("STRID_PROPERTY_VALUE");
    ui::PropertyGridProperty valueIdProperty(nullptr, valueIdParam);
    CHECK(valueIdProperty.GetPropertyNameId() == _T("STRID_PROPERTY_NAME"));
    CHECK(valueIdProperty.GetPropertyValueId() == _T("STRID_PROPERTY_VALUE"));
    CHECK(valueIdProperty.UsingPropertyValueId());
}

/** 默认值与重置 */
void TestDefaultValueAndReset()
{
    TestProperty property(nullptr, _T("Name"), _T("Original"));
    CHECK(property.GetDefaultValue().empty());
    CHECK(!property.IsModified());

    property.SetDefaultValue(_T("Default"));
    CHECK(property.GetDefaultValue() == _T("Default"));

    //重置为默认值
    property.ResetPropertyValue();
    CHECK(property.GetPropertyValue() == _T("Default"));
    CHECK(!property.IsModified());

    //未设置默认值时，重置为原始值
    TestProperty property2(nullptr, _T("Name"), _T("Original"));
    property2.ResetPropertyValue();
    CHECK(property2.GetPropertyValue() == _T("Original"));
    CHECK(!property2.IsModified());
}

/** 属性值校验 */
void TestValueValidator()
{
    TestProperty property(nullptr, _T("Name"), _T("Original"));
    bool bValidatorCalled = false;
    property.SetValueValidator([&bValidatorCalled](const DString& newValue) -> DString {
        bValidatorCalled = true;
        if (newValue == _T("Invalid")) {
            return _T("Invalid value");
        }
        return DString();
    });

    //校验通过
    CHECK(property.SetPropertyText(_T("Valid"), true));
    CHECK(bValidatorCalled);
    CHECK(property.GetLastErrorText().empty());

    //校验失败：返回false，并记录错误信息
    bValidatorCalled = false;
    CHECK(!property.SetPropertyText(_T("Invalid"), true));
    CHECK(bValidatorCalled);
    CHECK(property.GetLastErrorText() == _T("Invalid value"));

    //未变化的值不触发校验
    bValidatorCalled = false;
    CHECK(property.SetPropertyText(_T("Valid"), true));
    CHECK(bValidatorCalled);
}

/** 设置属性当前值 */
void TestSetPropertyNewValue()
{
    TestProperty property(nullptr, _T("Name"), _T("Original"));
    CHECK(property.SetPropertyNewValue(_T("NewValue")));
    CHECK(property.GetLastErrorText().empty());

    //只读状态：拒绝写入
    property.SetReadOnly(true);
    CHECK(!property.SetPropertyNewValue(_T("Another")));
    property.SetReadOnly(false);
    CHECK(property.SetPropertyNewValue(_T("Another")));
}

/** 校验函数拒绝 SetPropertyNewValue 时，保持原值并记录错误信息 */
void TestSetPropertyNewValueValidation()
{
    TestProperty property(nullptr, _T("Name"), _T("Origin"));
    property.SetValueValidator([](const DString& newValue) -> DString {
        if (newValue == _T("Bad")) {
            return _T("bad value");
        }
        return DString();
    });

    //校验失败：返回false，值不变，错误信息被记录
    CHECK(!property.SetPropertyNewValue(_T("Bad")));
    CHECK(property.GetLastErrorText() == _T("bad value"));
    CHECK(property.GetPropertyNewValue() == _T("Origin"));
    CHECK(!property.IsModified());

    //校验通过：清除错误信息
    CHECK(property.SetPropertyNewValue(_T("Good")));
    CHECK(property.GetLastErrorText().empty());
}

/** 分组的基本信息与只读状态 */
void TestGroup()
{
    ui::PropertyGridGroup group(nullptr);
    group.SetGroupName(_T("Group1"));
    group.SetDescription(_T("GroupDescription"));
    group.SetGroupData(55);
    CHECK(group.GetGroupName() == _T("Group1"));
    CHECK(group.GetDescription() == _T("GroupDescription"));
    CHECK(group.GetGroupData() == 55);
    CHECK(!group.IsReadOnly());

    group.SetReadOnly(true);
    CHECK(group.IsReadOnly());

    CHECK(group.IsPropertiesEnabled());
    group.SetEnableProperties(false);
    CHECK(!group.IsPropertiesEnabled());
    group.SetEnableProperties(true);
    CHECK(group.IsPropertiesEnabled());

    ui::PropertyGridGroup groupById(nullptr, true, _T("STRID_GROUP_NAME"), true, _T("STRID_GROUP_DESC"), 0);
    CHECK(groupById.GetGroupNameId() == _T("STRID_GROUP_NAME"));
    CHECK(groupById.GetDescriptionId() == _T("STRID_GROUP_DESC"));
}

/** 新增属性类型（布尔/数值/多行文本） */
void TestNewPropertyTypes()
{
    ui::PropertyGridBoolProperty boolProperty(nullptr, _T("Bool"), _T("true"));
    CHECK(boolProperty.GetPropertyType() == ui::PropertyGridPropertyType::kBool);
    CHECK(boolProperty.GetPropertyValue() == _T("true"));

    ui::PropertyGridNumberProperty numberProperty(nullptr, _T("Number"), _T("10"));
    CHECK(numberProperty.GetPropertyType() == ui::PropertyGridPropertyType::kNumber);
    CHECK(numberProperty.IsInteger());
    numberProperty.SetNumberRange(false, -10.0, 10.0, 2, 0.5);
    CHECK(!numberProperty.IsInteger());
    numberProperty.SetNumberRange(true, -10.0, 10.0, 0, 2.0);
    CHECK(numberProperty.IsInteger());

    ui::PropertyGridMultiLineTextProperty multiLineProperty(nullptr, _T("MultiLine"), _T("Line1\nLine2"));
    CHECK(multiLineProperty.GetPropertyType() == ui::PropertyGridPropertyType::kMultiLineText);
    CHECK(multiLineProperty.GetPropertyValue() == _T("Line1\nLine2"));
}

/** 文本属性的密码模式、修改状态与重置 */
void TestTextPropertyState()
{
    ui::PropertyGridTextProperty property(nullptr, _T("Password"), _T("123456"));
    CHECK(!property.IsPasswordMode());
    CHECK(!property.IsModified());

    //密码模式下，内部保存的是真实文本（不含掩码）
    property.SetPasswordMode(true);
    CHECK(property.IsPasswordMode());

    CHECK(property.SetNewTextValue(_T("abcdef")));
    CHECK(property.GetPropertyNewValue() == _T("abcdef"));
    CHECK(property.IsModified());

    //重置后恢复为原始值，且不再处于"已修改"状态
    property.ResetPropertyValue();
    CHECK(property.GetPropertyNewValue() == _T("123456"));
    CHECK(!property.IsModified());

    //只读时不允许设置值
    property.SetReadOnly(true);
    CHECK(!property.SetNewTextValue(_T("xyz")));
    property.SetReadOnly(false);
    CHECK(property.SetNewTextValue(_T("xyz")));
}

/** 布尔属性的默认值与重置 */
void TestBoolDefaultAndReset()
{
    ui::PropertyGridBoolProperty property(nullptr, _T("Bool"), _T("true"));
    property.SetDefaultValue(_T("false"));

    property.ResetPropertyValue();
    CHECK(property.GetPropertyValue() == _T("false"));

    //只读时拒绝设置值
    property.SetReadOnly(true);
    CHECK(!property.SetPropertyNewValue(_T("true")));
    property.SetReadOnly(false);
    CHECK(property.SetPropertyNewValue(_T("true")));
}

/** 数值属性的取值规范化（范围截断与小数精度） */
void TestNumberFormat()
{
    //整数类型：超出范围时按范围截断
    ui::PropertyGridNumberProperty intProperty(nullptr, _T("Int"), _T("0"));
    intProperty.SetPasswordMode(true); //借助密码模式读取内部值（无界面环境下）
    intProperty.SetNumberRange(true, 0, 100, 0, 1);
    CHECK(intProperty.SetPropertyNewValue(_T("150")));
    CHECK(intProperty.GetPropertyNewValue() == _T("100"));
    CHECK(intProperty.SetPropertyNewValue(_T("-5")));
    CHECK(intProperty.GetPropertyNewValue() == _T("0"));

    //浮点类型：按小数位数格式化
    ui::PropertyGridNumberProperty floatProperty(nullptr, _T("Float"), _T("0"));
    floatProperty.SetPasswordMode(true);
    floatProperty.SetNumberRange(false, -100.0, 100.0, 2, 1.0);
    CHECK(floatProperty.SetPropertyNewValue(_T("3.14159")));
    CHECK(floatProperty.GetPropertyNewValue() == _T("3.14"));

    //未设置取值范围时，不限制取值范围
    ui::PropertyGridNumberProperty freeProperty(nullptr, _T("Free"), _T("0"));
    freeProperty.SetPasswordMode(true);
    CHECK(freeProperty.SetPropertyNewValue(_T("999999")));
    CHECK(freeProperty.GetPropertyNewValue() == _T("999999"));
}

/** 多行文本的单行预览转换 */
class TestMultiLineProperty : public ui::PropertyGridMultiLineTextProperty
{
public:
    explicit TestMultiLineProperty(ui::Window* pWindow) :
        ui::PropertyGridMultiLineTextProperty(pWindow, _T("MultiLine"), _T(""))
    {
    }

    DString ToDisplayText(const DString& text) const
    {
        return ConvertToDisplayText(text);
    }
};

void TestMultiLineDisplayText()
{
    TestMultiLineProperty property(nullptr);
    //非编辑状态也按多行显示：仅统一换行符，不再压缩为单行
    CHECK(property.ToDisplayText(_T("Line1\nLine2")) == _T("Line1\nLine2"));
    CHECK(property.ToDisplayText(_T("Line1\r\nLine2")) == _T("Line1\nLine2"));
    CHECK(property.ToDisplayText(_T("Line1\rLine2")) == _T("Line1\nLine2"));
    CHECK(property.ToDisplayText(_T("SingleLine")) == _T("SingleLine"));
}

/** 属性类型的标识值 */
void TestPropertyTypes()
{
    CHECK(ui::PropertyGridPropertyType::kText != ui::PropertyGridPropertyType::kCombo);
    CHECK(ui::PropertyGridPropertyType::kBool != ui::PropertyGridPropertyType::kNumber);
    CHECK(ui::PropertyGridPropertyType::kNumber != ui::PropertyGridPropertyType::kMultiLineText);
    CHECK(ui::PropertyGridPropertyType::kCustom != ui::PropertyGridPropertyType::kNone);
}

} // namespace

int main()
{
#if defined(_WIN32) && defined(_DEBUG)
    // 本测试不启动全局资源（GlobalManager::Startup），部分内部断言仅用于提示
    // "未初始化/非UI线程"，这里让断言只输出到调试器，避免弹出对话框导致测试挂起
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

    std::cout << "[run] TestParam" << std::endl;
    TestParam();
    std::cout << "[run] TestDefaultValueAndReset" << std::endl;
    TestDefaultValueAndReset();
    std::cout << "[run] TestValueValidator" << std::endl;
    TestValueValidator();
    std::cout << "[run] TestSetPropertyNewValue" << std::endl;
    TestSetPropertyNewValue();
    std::cout << "[run] TestSetPropertyNewValueValidation" << std::endl;
    TestSetPropertyNewValueValidation();
    std::cout << "[run] TestGroup" << std::endl;
    TestGroup();
    std::cout << "[run] TestNewPropertyTypes" << std::endl;
    TestNewPropertyTypes();
    std::cout << "[run] TestTextPropertyState" << std::endl;
    TestTextPropertyState();
    std::cout << "[run] TestBoolDefaultAndReset" << std::endl;
    TestBoolDefaultAndReset();
    std::cout << "[run] TestNumberFormat" << std::endl;
    TestNumberFormat();
    std::cout << "[run] TestMultiLineDisplayText" << std::endl;
    TestMultiLineDisplayText();
    std::cout << "[run] TestPropertyTypes" << std::endl;
    TestPropertyTypes();

    std::cout << "property_grid tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
