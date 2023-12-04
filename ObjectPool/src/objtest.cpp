#include<HCNSObjectPool.hpp>

class Example_ClassA:public HCNSObjectPool<Example_ClassA>
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

int main()
{
    //C *p(HCNSObjectPool<C,10>::createObject(10,100,98.98,std::string("hello world")));
    Example_ClassA *p(
        Example_ClassA::createObject(100,98.98,std::string("hello world"))
    );
    return 0;
}