#include <iostream>
#include <vector>
double add(double a,double b)
{
	double result;
	result = a + b ;
	return result;
}

int main(int argc, char *argv[])
{
	int* a = new int[100];
	int b = 1;
	int* c = &b;
	for(int i = 0;i<=100;i++)
	{
		a[i] = i;
	}
	std::cout << argc << "\n"<< argv[1] << std::endl;
}
