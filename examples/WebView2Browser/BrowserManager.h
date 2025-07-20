#ifndef EXAMPLES_BROWSER_MANAGER_H_
#define EXAMPLES_BROWSER_MANAGER_H_

#include "BrowserBox.h"

/** 多标签浏览器管理类
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
    * @param[in] pBrowserForm 浏览器窗口，如果为nullptr则创建一个新的窗口，否则在这个窗口里新建浏览器盒子
    * @param[in] browserId 某个浏览器盒子的唯一标识(可以为空)
    * @param[in] url 初始化URL
    * @return BorwserBox*  浏览器盒子指针
    */
    BrowserBox* CreateBorwserBox(BrowserForm* pBrowserForm, std::string browserId, const DString& url);

    /** 判断某个浏览器盒子是否处于激活状态
    * @param[in] browserId 某个浏览器盒子的唯一标识
    * @return bool true 激活，false 非激活
    */
    bool IsBorwserBoxActive(const std::string& browserId);

    /** 根据会话id，找回浏览器盒子
    * @param [in] browserId 某个浏览器盒子的唯一标识
    * @return BorwserBox* 返回浏览器盒子的指针
    */
    BrowserBox* FindBorwserBox(const std::string& browserId);

    /** 根据会话id，移除本类中浏览器盒子信息
    * @param [in] browserId 某个浏览器盒子的唯一标识
    * @param [in] box id对应的浏览器盒子指针
    */
    void RemoveBorwserBox(const std::string& browserId, const BrowserBox* box = nullptr);

    /** 找到一个活动的窗口
    */
    BrowserForm* GetLastActiveBrowserForm() const;

    /** 创建Browser ID
    */
    std::string CreateBrowserID() const;

private:
    /** 所有的浏览器盒子按ID保持到该容器
    */
    std::map<std::string, BrowserBox*> m_boxMap;
};
#endif //EXAMPLES_BROWSER_MANAGER_H_
