#ifndef _UI_CONTROL_DATETIME_H_
#define _UI_CONTROL_DATETIME_H_
#pragma once
#include "duilib/Control/Label.h"

namespace ui
{
/** 日期时间控件
*/
class DateTimeWnd;
class UILIB_API DateTime : public Label
{
    friend class DateTimeWnd;
public:
    DateTime();
    DateTime(const DateTime& r) = delete;
    DateTime& operator=(const DateTime& r) = delete;
    virtual ~DateTime();

public:
    /** 获取日期格式
    */
    const std::wstring& GetFormat() const;
    
    /** 设置日期格式
    */
    void SetFormat(const std::wstring& val);

    /** 以本地时间初始化
    */
    void InitLocalTime();

    /** 清除日期时间值
    */
    void ClearTime();

    /** 获取当前日期时间字符串
    */
    std::wstring GetDateTime() const;

    /** 获取时间值
    */
    const SYSTEMTIME& GetSystemTime() const;

    /** 设置时间值
    */
    void SetTime(const SYSTEMTIME& systemTime);

    /** 当前时间值是否为有效的日期时间
    */
    bool IsValidTime() const;

    /** 设置只读模式
    */
    void SetReadOnly(bool bReadOnly);

    /** 是否为只读模式
    */
    bool IsReadOnly() const;

    /** 添加日期时间值变化监听事件
    */
    void AttachTimeChange(const EventCallback& callback) { AttachEvent(kEventValueChange, callback); }

public:
    //基类的虚函数
    virtual std::wstring GetType() const override;
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
    virtual void HandleEvent(const EventArgs& event) override;

private:
    //比较两个时间是否相同
    bool IsEqual(const SYSTEMTIME& st) const;

private:
    //系统时间值
    SYSTEMTIME m_sysTime;

    //只读模式
    bool m_bReadOnly;

    /* 日期时间格式
    
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
    std::wstring m_sFormat;

    //设置日期控件窗口接口
    DateTimeWnd* m_pDateWindow;
};

}//namespace ui

#endif // _UI_CONTROL_DATETIME_H_
