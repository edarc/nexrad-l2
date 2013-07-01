#include <iostream>

using namespace std;

int main(void)
{
    union {
        float f;
        unsigned long l;
    } a;
    
    a.f = 0.5;
    cout << "0.5 -> " <<hex << a.l << endl;

    unsigned int b = 3;
    cout << "3 -> " << static_cast<float>(b) << endl;
}
