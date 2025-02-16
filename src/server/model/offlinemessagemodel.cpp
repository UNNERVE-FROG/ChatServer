#include "model/offlinemessagemodel.hpp"
#include "db/db.hpp"
using namespace std;

void OfflineMessageModel::insert(int userid,std::string msg)
{
    char sql[1024] = {0};
    sprintf(sql,"insert into offlinemessage(userid,message) values('%d','%s')",userid,msg.c_str());

    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }

}
void OfflineMessageModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql,"delete from offlinemessage where userid='%d'",userid);

    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}


vector<string> OfflineMessageModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql,"select message from offlinemessage where userid='%d'",userid);

    MySQL mysql;
    vector<string> vec;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr) 
        {
            MYSQL_ROW row;
            while ((row=mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}
