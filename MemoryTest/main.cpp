#include "memorytest.h"
#include <QtWidgets/QApplication>
#include <QDebug>
#include <set>

int newInt1()
{
    for (int i=0; i<10; i++)
    {
        int *p = new int();
        delete p;
    }

    //for (int ii=0; ii<1000000; ii++)
    //{
    //    int n = 10;
    //    int **ppI = new int*[n];
    //    for (int i=0; i<n; i++)
    //    {
    //        ppI[i] = new int;
    //        delete ppI[i];
    //        ppI[i] = nullptr;
    //    }
    //    delete ppI;
    //    void **ptr2 = (void **)ppI;
    //    ppI = nullptr;
    //}
    

    return 0;
}

int newInt2()
{
    for (int i=0; i<20; i++)
    {
        new int();
    }
    return 0;
}

int newDouble1()
{
    for (int i=0; i<10; i++)
    {
        new double();
    }
    return 0;
}

int newDouble2()
{
    for (int i=0; i<20; i++)
    {
        double *p = new double();
        delete p;
    }
    return 0;
}

void testNewFree()
{
    int *p = new int;
    free(p);
    p = nullptr;
}

void testSTD()
{
    std::wstring sName;
    std::vector<int> a;
}

void testManyMemoryFunc()
{
    int *p1 = (int*)malloc(4);
    int *p2 = (int*)calloc(2, 4);

    p1 = (int*)realloc(p1, 1024);
    //free(p1);

    p2 = (int*)_recalloc(p2, 4, 8);
    //free(p2);
}

class BaseClassNotVirtualDestruction
{
public:
    BaseClassNotVirtualDestruction() {}
    //~BaseClassNotVirtualDestruction() {}
};

class ChildClassNotVirtualDestruction : public BaseClassNotVirtualDestruction
{
public:
    ChildClassNotVirtualDestruction()
    {
        vct1.push_back(1);
        vct1.push_back(1);
        vct1.push_back(1);

        vct2.push_back(1);
        vct2.push_back(1);
        vct2.push_back(1);
    }
    ~ChildClassNotVirtualDestruction()
    {

    }

    std::vector<int> vct1;
    QVector<int> vct2;
};

class TestVectorData
{
public:
    ~TestVectorData()
    {
        new int;
    }
};

class testClass
{
public:
    testClass(int len)
    {
        ptr = new char[len];
        ptr2 = new char;
        ptr3 = new char;
        char *p1 = new char[123];
        char *p2 = new char;
        char *p3 = new char[56];
        std::set<int> *set1 = new std::set<int>;
        set1->insert(1);
        delete set1;

        std::vector<int> *vct1 = new std::vector<int>;
        std::vector<TestVectorData> *vct2 = new std::vector<TestVectorData>;
        
        //for (int i=0; i<10000; i++)
        //{
        //    new int;
        //}
        vct1->push_back(1);
        vct1->reserve(100);
        vct2->push_back(TestVectorData());
        vct2->reserve(200);
        for (int i=0; i<100000; i++)
        {
            new int;
        }
        vct1->reserve(10000);
        vct1->reserve(10);
        vct2->reserve(10000);
        vct2->reserve(10);
        //vct1->swap(std::vector<int>());
        //for (int i=0; i<10000000; i++)
        //{
        //    vct1->push_back(i);
        //    vct2->push_back(TestVectorData());
        //}
        //vct1->swap(std::vector<int>());
        //delete vct1;


        //qDebug() << "ptr" << (int)ptr;
        //qDebug() << "ptr2" << (int)ptr2;
        //qDebug() << "ptr3" << (int)ptr3;
        //qDebug() << "p1" << (int)p1;
        //qDebug() << "p2" << (int)p2;
        m_index = len;
        //qDebug() << "testClass()" << m_index;

        //ptr = (char*)1;
        //ch1 = 0;
        //ch2 = 1;
        ////ch3 = 0;
        //ptr2 = (char*)1;
        //ptr3 = 0;
        //m_index = 1;

        qDebug() << "testClass() 1" << (int)&ptr << (int)&ch1 << (int)&ch2 << (int)&ptr2 << (int)&ptr3 << (int)&m_index;
    }

    void setPtr(int len)
    {
        ptr = new char(len);
    }

    ~testClass()
    {
        //delete ptr;
        //ptr = nullptr;
        qDebug() << "~testClass()" << m_index;
    }

    char *ptr;
    char *ch1;
    char *ch2;
    char *ch3;
    //char ch4;
    //char ch5;
    char *ptr2;
    char *ptr3;
    int m_index;
    std::set<int> m_set1;
};

//≤‚ ‘À≥–Ú
//testClass g_test1(1);
//testClass *g_test2 = new testClass(2);
//
//testClass *g_test3 = new testClass(3);
//testClass g_test4(4);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    //MemoryTest w;
    //w.show();

    system("pause");

    {
        BaseClassNotVirtualDestruction *baseValue1 = new ChildClassNotVirtualDestruction();
        delete baseValue1;
        baseValue1 = nullptr;
    }

    //system("pause");

    //testManyMemoryFunc();

    system("pause");

    //static testClass s_test5(5);

    void * testp = new testClass(100);
    //qDebug() << "testp" << (int)testp;
    //int *p00 = new int;

    system("pause");
    testClass(1024);
    int *p0 = new int;
    system("pause");
    int *p5 = new int();
    double *p6 = new double();
    //newInt1();
    //newInt2();
    //newDouble1();
    //newDouble2();
    //testNewFree();
    //testSTD(); 
    //delete p0;
    //p0 = nullptr;
    system("pause");

    return 0/*a.exec()*/;
}
