#pragma once

#include <string>
#include <vector>

struct alpha
{
	std::string value1,value2;
	int index;
	alpha() : value1(""),value2(""),index(0){ }
	alpha(std::string inValue1,std::string inValue2,int inIndex) : value1(inValue1),value2(inValue2),index(inIndex){ }
};

// /std::vector<std::string> bet = {"0","1","2","3","4","5","6","7","8","9"};
class bet
{
public:
	bet()
	{
		numberbet[3] = (1,2,3);
	};
	~bet();
private:
	static std::string numberbet[3];
	static std::string alphabet[3];
};


