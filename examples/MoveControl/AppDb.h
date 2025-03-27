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

    bool LoadFromDb(std::vector<AppItem>& app_list);
    bool SaveToDb(const AppItem& item);

private:

};

#endif //EXAMPLES_APP_DB_H_
