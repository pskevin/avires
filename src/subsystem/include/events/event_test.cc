#include <map>
#include <string>
#include <iostream>

using std::cout;
using std::endl;

class A
{
public:
    A() {}
    virtual void f() { cout << "A::f" << endl; }
};

class B : public A
{
public:
    B() {}
    void f() { cout << "B::f" << endl; }
};

int main()
{
    std::map<std::string, void (A::*)()> fMap;


    A a;
    fMap["a"] = &A::f;

    B b;
    fMap["b"] = &B::f;

    // call via pointer to object
    A *aptr = &a;
    (aptr->*fMap["a"])();    

    B *bptr = &b;
    (bptr->*fMap["b"])();    
    return 0;
}