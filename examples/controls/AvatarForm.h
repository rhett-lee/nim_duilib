#ifndef EXAMPLES_CONTROLS_AVATAR_FORM_H_
#define EXAMPLES_CONTROLS_AVATAR_FORM_H_

#include "duilib/duilib.h"

/** Avatar 头像演示窗口
 */
class AvatarForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    AvatarForm();
    virtual ~AvatarForm() override;

    /** 加载资源文件的目录，位于 bin/resources/ 目录下
     */
    virtual DString GetSkinFolder() override;

    /** 窗口使用的 XML 布局文件
     */
    virtual DString GetSkinFile() override;

    /** 窗口初始化
     */
    virtual void OnInitWindow() override;

private:
    /** 绑定"动态修改"演示区的按钮
     */
    void BindDynamicDemo();

    /** 切换尺寸：24 -> 32 -> 40 -> 64 -> 24
     */
    void OnChangeSize();

    /** 切换形状：圆形 <-> 圆角方形
     */
    void OnChangeShape();

    /** 切换文字内容
     */
    void OnChangeText();

    /** 切换图片内容（有 image 时优先于文字显示）
     */
    void OnChangeImage();

    /** 清空全部内容（图片 / 图标 / 文字），只保留底色
     */
    void OnClearContent();

    /** 开关自动调色板
     */
    void OnTogglePalette();

    /** 把动态演示头像的当前状态回显到标签
     */
    void UpdateStateText();

    /** 读取当前语言下的文本
     * @param [in] strId 语言 ID
     */
    DString GetLangText(const DString& strId) const;

private:
    /** 动态演示的头像控件（生命周期与窗口一致，不需要额外持有）
     */
    ui::Avatar* m_pDynamic = nullptr;

    /** 尺寸循环的当前索引（对应 24 / 32 / 40 / 64）
     */
    size_t m_nSizeIndex = 2;

    /** 文字循环的当前索引
     */
    size_t m_nTextIndex = 0;

    /** 动态演示当前是否显示图片
     */
    bool m_bShowImage = false;
};

#endif // EXAMPLES_CONTROLS_AVATAR_FORM_H_
