// #include <algorithm>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "hi there!" << std::endl;

    int numVecs = 0;

    std::vector<int> myVecs;

    std:: cout << "\nEnter # of vectors" << std::endl;

    std::cin >> numVecs;

    for (int i = 0; i < numVecs; i++)
        myVecs.push_back(i);

    std::cout << "\nMy Vecs -> " << std::endl;

    for (int i = 0; i < numVecs; i++)
        std::cout << myVecs[i] << std::endl;

    int sum = 0;

    // std::for_each(myVecs.begin(), myVecs.end(),
    //     [&](int x){ sum += x; });
    for (auto & vecs : myVecs)
        sum += vecs;

    std::cout << "\nTotal sum is: " << sum << std::endl;

    return 0;
}
