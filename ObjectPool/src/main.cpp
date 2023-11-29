#include<HCNSObjectPool.hpp>
#include"HCNSObjectPool.cpp"

class Example_ClassA:public HCNSObjectPool<Example_ClassA,10>
{
public:
    Example_ClassA(int _a,double _b,std::string _c)
        :a(_a),b(_b),c(_c){}

    virtual ~Example_ClassA(){}

private:
    int a;
    double b;
    std::string c;
};

class Example_ClassB:public HCNSObjectPool<Example_ClassB,1>
{
public:
    Example_ClassB(){
        
    }
    virtual ~Example_ClassB(){}
};

int main()
{
    Example_ClassA *create_a = Example_ClassA::createObject(100,98.98,std::string("hello world"));
    Example_ClassA::deleteObject(create_a);
    return 0;
}