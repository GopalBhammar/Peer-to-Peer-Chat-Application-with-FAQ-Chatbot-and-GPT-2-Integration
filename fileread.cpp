#include <iostream>
#include <fstream>
#include <sstream>
// #include<bits/stdc++.h>
using namespace std;

int main()
{
    ifstream myfile("FAQs.txt");
    string temp;
    while(getline(myfile,temp))
    {
        stringstream tok(temp);
        string inte;
        while(getline(tok,inte,' '))
        {
            cout<<inte<<endl;
        }
    }
}