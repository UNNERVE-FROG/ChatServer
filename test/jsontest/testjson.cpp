#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;

//json序列化1
string func1() {
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhangsan";
    js["to"] = "li si";
    js["msg"] = "hello world!";

    string buffer = js.dump();

    cout << buffer <<endl;
    return buffer;
}

string func2(){
    json js;
    js["id"] = {1,2,3,4,5};
    // 添加key-value
    js["name"] = "zhang san";
    // 添加对象
    js["msg"]["zhang san"] = "hello world";
    js["msg"]["liu shuo"] = "hello china";
    // 上面等同于下面这句一次性添加数组对象
    js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}};
    cout << js <<endl;
    return js.dump();
}

//json 序列化容器
string func3() {
    json j;
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    j["list"] = vec;

    map<int,string> m;
    m.insert({1,"黄山"});
    m.insert({2,"华山"});

    j["map"] = m;

    cout << j<<endl;
    return j.dump();

}

int main() {
    string buffer = func3();
    json js = json::parse(buffer);
    map<int,string> m = js["map"];
    cout << m[1];
    return 0;
}