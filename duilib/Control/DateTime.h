#ifndef _UI_CONTROL_DATETIME_H_
#define _UI_CONTROL_DATETIME_H_

#include "duilib/Control/Label.h"
#include "duilib/Box/HBox.h"
#include <ctime>

namespace ui
{
/** 日期时间选择控件
*/
class DateTimeWnd;
class UILIB_API DateTime : public LabelTemplate<HBox>
{
    typedef LabelTemplate<HBox> BaseClass;
    friend class DateTimeWnd;
public:
    explicit DateTime(Window* pWindow);
    DateTime(const DateTime& r) = delete;
    DateTime& operator=(const DateTime& r) = delete;
    virtual ~DateTime() override;

public:
    /** 以本地时间初始化
    */
    void InitLocalTime();

    /** 清除日期时间值
    */
    void ClearTime();

    /** 获取日期时间值
    */
    const struct tm& GetDateTime() const;

    /** 设置日期时间值
    */
    void SetDateTime(const struct tm& dateTime);

    /** 当前日期时间值是否为有效的日期时间
    */
    bool IsValidDateTime() const;

    /** 获取日期时间字符串, 日期的格式可以通过SetStringFormat()函数设置
    */
    DString GetDateTimeString() const;

    /** 设置日期时间字符串, 按照GetStringFormat()指定的格式, 更新日期时间值
    */
    bool SetDateTimeString(const DString& dateTime);

    /** 获取日期的字符串格式
    */
    DString GetStringFormat() const;
    
    /** 设置日期格式，默认值取决于EditFormat的值
    * @param [in] sFormat 日期的格式，具体可参考：std::put_time 函数的说明：
        %a 星期几的简写
        %A 星期几的全称
        %b 月分的简写
        %B 月份的全称
        %c 标准的日期的时间串
        %C 年份的后两位数字
        %d 十进制表示的每月的第几天
        %D 月/天/年
        %e 在两字符域中，十进制表示的每月的第几天
        %F 年-月-日
        %g 年份的后两位数字，使用基于周的年
        %G 年分，使用基于周的年
        %h 简写的月份名
        %H 24小时制的小时
        %I 12小时制的小时
        %j 十进制表示的每年的第几天
        %m 十进制表示的月份
        %M 十时制表示的分钟数
        %n 新行符
        %p 本地的AM或PM的等价显示
        %r 12小时的时间
        %R 显示小时和分钟：hh:mm
        %S 十进制的秒数
        %t 水平制表符
        %T 显示时分秒：hh:mm:ss
        %u 每周的第几天，星期一为第一天 （值从0到6，星期一为0）
        %U 第年的第几周，把星期日做为第一天（值从0到53）
        %V 每年的第几周，使用基于周的年
        %w 十进制表示的星期几（值从0到6，星期天为0）
        %W 每年的第几周，把星期一做为第一天（值从0到53）
        %x 标准的日期串
        %X 标准的时间串
        %y 不带世纪的十进制年份（值从0到99）
        %Y 带世纪部分的十进制年份
        %z，%Z 时区名称，如果不能得到时区名称则返回空字符。
        %% 百分号
    */
    void SetStringFormat(const DString& sFormat);

    /** 日期时间的编辑格式
    */
    enum class EditFormat
    {
        kDateCalendar,      //编辑时显示：年-月-日，通过下拉框展示月日历的方式来修改日期，不支持选择时间(SDL实现时，同kDateUpDown)
        kDateUpDown,        //编辑时显示：年-月-日，通过控件的右侧放置一个向上-向下的控件以修改日期，不支持选择时间
        kDateTimeUpDown,    //编辑时显示：年-月-日 时:分:秒，通过控件的右侧放置一个向上-向下的控件以修改日期和时间
        kDateMinuteUpDown,  //编辑时显示：年-月-日 时:分，通过控件的右侧放置一个向上-向下的控件以修改日期和时间
        kTimeUpDown,        //编辑时显示：时:分:秒，通过控件的右侧放置一个向上-向下的控件以修改时间，不支持修改日期
        kMinuteUpDown,      //编辑时显示：时:分，通过控件的右侧放置一个向上-向下的控件以修改时间，不支持修改日期
    };

    /** 设置编辑格式
    */
    void SetEditFormat(EditFormat editFormat);

    /** 获取编辑模式
    */
    EditFormat GetEditFormat() const;

    /** 获取年月日的分隔符
    */
    DString::value_type GetDateSeparator() const;

    /** 更新下编辑窗口的位置
    */
    void UpdateEditWndPos();

    /** 设置Spin功能的Class名称
    */
    void SetSpinClass(const DString& spinClass);

    /** 获取Spin功能的Class名称
    */
    DString GetSpinClass() const;

    /** 添加日期时间值变化监听事件
    */
    void AttachTimeChange(const EventCallback& callback) { AttachEvent(kEventValueChange, callback); }

public:
    //基类的虚函数
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void HandleEvent(const EventArgs& msg) override;

    //用于初始化xml属性
    virtual void OnInit() override;

    /** 将消息派发到消息处理函数
     * @param[in] msg 消息内容
     */
    virtual void SendEventMsg(const EventArgs& msg) override;

private:
    /** 比较两个时间是否相同
    */
    bool IsEqual(const struct tm& a, const struct tm& b) const;

    /** 结束编辑
    */
    void EndEditDateTime();

private:
    /** 当前的日期时间值
    */
    struct tm m_dateTime;

    /* 日期时间显示格式
    */
    UiString m_sFormat;

    /** 日期时间的编辑格式
    */
    EditFormat m_editFormat;

    /** 年月日的分隔符
    */
    DString::value_type m_dateSeparator;

    /** 设置日期控件窗口接口
    */
    DateTimeWnd* m_pDateWindow;

    /** Spin功能的Class名称
    */
    UiString m_spinClass;
};

}//namespace ui

#endif // _UI_CONTROL_DATETIME_H_
