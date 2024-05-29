#ifndef EXAMPLES_APP_DB_H_
#define EXAMPLES_APP_DB_H_

#include "app_item.h"

//与app相关的Db操作
//这个类最好是单实例类
class CAppDb
{
public:
    static CAppDb& GetInstance()
    {
        static CAppDb db;
        return db;
    }

    CAppDb();
    ~CAppDb();

    bool LoadFromDb(std::vector<AppItem>& app_list);
    bool SaveToDb(const AppItem& item);

private:

};

#endif //EXAMPLES_APP_DB_H_
