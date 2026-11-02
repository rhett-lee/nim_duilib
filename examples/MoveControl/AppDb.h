#ifndef EXAMPLES_APP_DB_H_
#define EXAMPLES_APP_DB_H_

#include "AppItemUi.h"

//与app相关的Db操作
//这个类最好是单实例类
class AppDb
{
public:
    static AppDb& GetInstance()
    {
        static AppDb db;
        return db;
    }

    AppDb();
    ~AppDb();

    /** 加载应用列表
    * @param [out] app_list 返回当前的应用列表（包含各应用是否属于"常用应用"）
    */
    bool LoadFromDb(std::vector<AppItem>& app_list);

    /** 保存应用的状态（本示例用内存模拟数据库，真实项目中可换成持久化存储）
    * @param [in] item 要保存的应用
    */
    bool SaveToDb(const AppItem& item);

private:
    /** 首次加载时初始化演示数据
    */
    void InitDefaultData();

private:
    /** 内存中的"数据库"：保存应用列表以及各应用的"常用"状态
    */
    std::vector<AppItem> m_appList;

    /** 演示数据是否已初始化
    */
    bool m_bInited = false;
};

#endif //EXAMPLES_APP_DB_H_
