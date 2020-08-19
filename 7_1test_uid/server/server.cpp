#include<unistd.h>
#include<iostream>
using namespace std;
int main()
{
    uid_t uid = getuid();
    uid_t euid = geteuid();
    cout<<"Userid is "<<uid<<" and effective userid is "<<euid<<endl;
    return 0;
}