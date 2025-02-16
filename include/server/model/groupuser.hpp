#ifndef GROUPUSER_H
#define GROUPUSER_H
#include <string>
#include "user.hpp"
class GroupUser:public User
{
public:
    void setRole(std::string role) {this->role = role;}
    std::string getRole() {return this->role;}
private:
    std::string role;
};

#endif