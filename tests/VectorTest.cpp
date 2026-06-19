#include <iostream>
#include <exception>
#include "Vector.h"

using namespace std;

int testsPassed = 0;
int testsFailed = 0;

void pass(const string& testName){
    cout << "[PASS] " << testName << endl;
    testsPassed++;
}

void fail(const string& testName){
    cout << "[FAIL] " << testName << endl;
    testsFailed++;
}

void printSummary(){
    cout << "\n=====================================\n";
    cout << "Testing Summary\n";
    cout << "=====================================\n";
    cout << "Passed : " << testsPassed << endl;
    cout << "Failed : " << testsFailed << endl;
    cout << "Total  : " << testsPassed + testsFailed << endl;
    cout << "=====================================\n";
}

void testDefaultConstructor(){
    Vector<int> v;

    if(v.size()==0 &&
       v.capacity()==1 &&
       v.empty())
        pass("Default Constructor");
    else
        fail("Default Constructor");
}

void testInitializerList(){
    Vector<int> v={1,2,3};

    if(v.size()==3 &&
       v[0]==1 &&
       v[1]==2 &&
       v[2]==3)
        pass("Initializer List");
    else
        fail("Initializer List");
}

void testCopyConstructor(){
    Vector<int> a={10,20,30};

    Vector<int> b(a);

    if(b.size()==3 &&
       b[0]==10 &&
       b[1]==20 &&
       b[2]==30)
        pass("Copy Constructor");
    else
        fail("Copy Constructor");
}

void testAssignmentOperator(){
    Vector<int> a={5,6,7};

    Vector<int> b;

    b=a;

    if(b.size()==3 &&
       b[0]==5 &&
       b[1]==6 &&
       b[2]==7)
        pass("Assignment Operator");
    else
        fail("Assignment Operator");
}


void testPushBackOne(){
    Vector<int> v;

    v.push_back(10);

    if(v.size()==1 &&
       v[0]==10)
        pass("Push Back One Element");
    else
        fail("Push Back One Element");
}

void testPushBackMultiple(){
    Vector<int> v;

    for(int i=0;i<10;i++)
        v.push_back(i);

    bool ok=true;

    for(int i=0;i<10;i++){
        if(v[i]!=i)
            ok=false;
    }

    if(ok)
        pass("Push Back Multiple");
    else
        fail("Push Back Multiple");
}


void testPopBack(){
    Vector<int> v={1,2,3};

    v.pop_back();

    if(v.size()==2 &&
       v.back()==2)
        pass("Pop Back");
    else
        fail("Pop Back");
}

void testPopBackEmpty(){
    Vector<int> v;

    try{
        v.pop_back();
        fail("Pop Empty Vector");
    }
    catch(...){
        pass("Pop Empty Vector");
    }
}

void testInsertBeginning(){
    Vector<int> v={2,3};

    v.insert(0,1);

    if(v[0]==1 &&
       v[1]==2 &&
       v[2]==3)
        pass("Insert Beginning");
    else
        fail("Insert Beginning");
}

void testInsertMiddle(){
    Vector<int> v={1,3};

    v.insert(1,2);

    if(v[0]==1 &&
       v[1]==2 &&
       v[2]==3)
        pass("Insert Middle");
    else
        fail("Insert Middle");
}

void testInsertEnd(){
    Vector<int> v={1,2};

    v.insert(2,3);

    if(v.back()==3)
        pass("Insert End");
    else
        fail("Insert End");
}


void testRemoveBeginning(){
    Vector<int> v={1,2,3};

    v.remove(0);

    if(v[0]==2 &&
       v.size()==2)
        pass("Remove Beginning");
    else
        fail("Remove Beginning");
}

void testRemoveMiddle(){
    Vector<int> v={1,2,3};

    v.remove(1);

    if(v[1]==3)
        pass("Remove Middle");
    else
        fail("Remove Middle");
}

void testRemoveEnd(){
    Vector<int> v={1,2,3};

    v.remove(2);

    if(v.back()==2)
        pass("Remove End");
    else
        fail("Remove End");
}


void testFront(){
    Vector<int> v={5,6,7};

    if(v.front()==5)
        pass("Front");
    else
        fail("Front");
}

void testBack(){
    Vector<int> v={5,6,7};

    if(v.back()==7)
        pass("Back");
    else
        fail("Back");
}

void testAt(){
    Vector<int> v={5,6,7};

    if(v.at(1)==6)
        pass("At");
    else
        fail("At");
}

void testEmpty(){
    Vector<int> v;

    if(v.empty())
        pass("Empty");
    else
        fail("Empty");
}

void testSize(){
    Vector<int> v={1,2,3};

    if(v.size()==3)
        pass("Size");
    else
        fail("Size");
}

void testCapacity(){
    Vector<int> v;

    if(v.capacity()==1)
        pass("Capacity");
    else
        fail("Capacity");
}

int main(){
    cout<<"========== VECTOR TESTS ==========\n\n";

    testDefaultConstructor();
    testInitializerList();
    testCopyConstructor();
    testAssignmentOperator();

    testPushBackOne();
    testPushBackMultiple();

    testPopBack();
    testPopBackEmpty();

    testInsertBeginning();
    testInsertMiddle();
    testInsertEnd();

    testRemoveBeginning();
    testRemoveMiddle();
    testRemoveEnd();

    testFront();
    testBack();
    testAt();

    testEmpty();
    testSize();
    testCapacity();

    printSummary();

    return 0;
}