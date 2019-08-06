#include <iostream>

void addOne(int val)
{
    // val++;
    val = 777;
}

void addOneP(int* val)
{
    // *val++;
    *val += 1;
    // *val = 888;
}

int main()
{
    int first = 123;
    int second = 456;

    int* third;
    *third = 789;

    // int* fourth;
    // *fourth = 333;

    std::cout << "\nfirst: " << first << std::endl;
    std::cout << "first [ref]: " << &first << std::endl;
    addOne(first);
    std::cout << "first [addOne]: " << first << std::endl;

    std::cout << "\nsecond: " << second << std::endl;
    std::cout << "second [ref]: " << &second << std::endl;
    addOneP(&second);
    std::cout << "second [addOne]: " << second << std::endl;

    std::cout << "\nthird: " << *third << std::endl;
    std::cout << "third [pnt]: " << third << std::endl;
    std::cout << "third [ref]: " << &third << std::endl;
    addOneP(third);
    std::cout << "third [addOne]: " << *third << std::endl;

    int& fourth = second;

    std::cout << "\nfourth: " << fourth << std::endl;
    std::cout << "fourth [ref]: " << &fourth << std::endl;
    addOneP(&second);
    // &fourth = &fourth - 4;
    std::cout << "fourth [val]: " << fourth << std::endl;

    // std::cout << "\nfourth: " << *fourth << std::endl;
    // std::cout << "fourth [pnt]: " << fourth << std::endl;
    // std::cout << "fourth [ref]: " << &fourth << std::endl;
    // addOneP(fourth);
    // std::cout << "fourth [addOne]: " << *fourth << std::endl;
}
