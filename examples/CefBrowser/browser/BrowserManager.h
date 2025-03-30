#ifndef EXAMPLES_BROWSER_MANAGER_H_
#define EXAMPLES_BROWSER_MANAGER_H_

#include "BrowserBox.h"

/** 多标签浏览器管理类，用于统一管理多标签浏览器打开、合并、拆分功能
 */
class BrowserManager : public virtual ui::SupportWeakCallback
{
public:
    BrowserManager();
    BrowserManager(const BrowserManager&) = delete;
    BrowserManager& operator=(const BrowserManager&) = delete;

    /** 单例对象
    */
    static BrowserManager* GetInstance();

private:
    ~BrowserManager();

public:
    /** 创建一个Browser窗口
    */
    BrowserForm* CreateBrowserForm();

    /** 创建一个浏览器盒子
    * @param[in] browser_form 浏览器窗口，如果为nullptr则创建一个新的窗口，否则在这个窗口里新建浏览器盒子
    * @param[in] id 某个浏览器盒子的唯一标识
    * @param[in] url 初始化URL
    * @return BorwserBox*    浏览器盒子指针
    */
    BrowserBox* CreateBorwserBox(BrowserForm* browser_form, const std::string& id, const DString& url);

    /** 判断某个浏览器盒子是否处于激活状态
    * @param[in] id 某个浏览器盒子的唯一标识
    * @return bool true 激活，false 非激活
    */
    bool IsBorwserBoxActive(const std::string& id);

    /** 根据会话id，找回浏览器盒子
    * @param [in] id 某个浏览器盒子的唯一标识
    * @return BorwserBox* 返回浏览器盒子的指针
    */
    BrowserBox* FindBorwserBox(const std::string& id);

    /** 根据会话id，移除本类中浏览器盒子信息
    * @param [in] id 某个浏览器盒子的唯一标识
    * @param [in] box id对应的浏览器盒子指针
    */
    void RemoveBorwserBox(const std::string& id, const BrowserBox* box = nullptr);

    /** 找到一个活动的窗口
    */
    BrowserForm* GetLastActiveBrowserForm() const;

    /** 创建Browser ID
    */
    std::string CreateBrowserID() const;

public:
    /** 设置是否开启多标签功能
    * @param[in] enable true:开启多标签功能,自动合并当前所有浏览器盒子（一个窗口中显示多个网页，多标签模式）
    *                   false:关闭多标签功能,自动拆分当前所有浏览器盒子（一个窗口只显示一个网页，单窗口模式）
    */
    void SetEnableMerge(bool enable);

    /** 是否开启多标签功能
    * @return bool true 是，false 否
    */
    bool IsEnableMerge() const;

private:
    /** 是否开启多标签功能
    */
    bool m_bEnableMerge;

    /** 所有的浏览器盒子按ID保持到该容器
    */
    std::map<std::string, BrowserBox*> m_boxMap;
};
#endif //EXAMPLES_BROWSER_MANAGER_H_
