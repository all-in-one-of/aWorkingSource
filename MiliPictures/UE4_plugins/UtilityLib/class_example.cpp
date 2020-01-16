//C++ class example
//header.h
#ifndef HEADER_H
#define HEADER_H
#include <iostream>
#include <string>
#include <vector>
namespace ChenhuiPan
{
	namespace KaiXu
	{
		class myClass
		{
		public:
			myClass(int &argId,int &argNum);
			~myClass();
			static std::string check;
			std::string show()
			{

				std::cout << result <<std::endl;
				return "Great";
			}
		private:
			double id;
			double num;
			double result;
			double add();

		};
		std::string myClass::check = "Yohoo";
		myClass::myClass(int &argId,int &argNum)
		{
			id = argId;
			num = argNum;
			result = add();
		}
		myClass::~myClass()
		{
			std::cout << "Clear it!" << std::endl;
		}
		double myClass::add()
		{
			double result;
			result = id + num;
			return result;
		}

	}

}
#endif
//main.cpp
#include <iostream>
#include <string>
#include <vector>
#include "header.h"
using namespace std;
struct data_base
{
	string name;
	int id;
	string course;
	int score;

};
int main()
{   
	int a=1,b=2;
	ChenhuiPan::KaiXu::myClass data(a,b);
	ChenhuiPan::KaiXu::myClass *point = &data;
	cout << point->check << endl;
	point->show();
	vector<data_base>map;	
	system("pause");
}
//example end
