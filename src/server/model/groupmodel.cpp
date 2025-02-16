#include "model/groupmodel.hpp"
#include "db/db.hpp"
using namespace std;

//创建群组
bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql,"insert into allgroup(groupname,groupdesc) values('%s','%s')",group.getName().c_str(),group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect()) {
        if(mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}


void GroupModel::addGroup(int userid,int grooupid,string role)
{
    char sql[1024] = {0};
    sprintf(sql,"insert into groupuser values(%d,%d,'%s')",userid,grooupid,role.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[1024] = {0};
    sprintf(sql,"select a.id,a.groupname,a.groupdesc from allgroup a inner join groupuser b on a.id = b.groupid where b.userid=%d",userid);
    MySQL mysql;
    vector<Group> groupVec;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row=mysql_fetch_row(res))!=nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    for (auto &group : groupVec)
    {
        sprintf(sql,"select a.id,a.name,a.state,b.grouprole from user a inner join groupuser b on a.id = b.userid where b.groupid=%d",group.getId());
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row=mysql_fetch_row(res))!=nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);

            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}

//查询出了userid之外其他成员的id
vector<int> GroupModel::queryGroupUsers(int userid,int groupid)
{
    char sql[1024] = {0};
    sprintf(sql,"select userid from groupuser where groupid=%d and userid!=%d",groupid,userid);
    MySQL mysql;
    vector<int> idVec;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row=mysql_fetch_row(res))!=nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}

