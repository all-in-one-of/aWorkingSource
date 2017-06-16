#include <iostream>
#include <string>

std::string toString(float fltValue)
{
	static const char* hexDigits = "0123456789abcdef";
	unsigned int value = *reinterpret_cast<unsigned int*>(&fltValue);
	std::cout << "unsigned int value:" << value << std::endl;
	char buf[10];
	buf[9] = '\0';
	buf[8] = hexDigits[(value & 0xf)];
	value >>= 4;
	buf[7] = hexDigits[(value & 0xf)];
	value >>= 4;
	buf[6] = hexDigits[(value & 0xf)];
	value >>= 4;
	buf[5] = hexDigits[(value & 0xf)];
	value >>= 4;
	buf[4] = hexDigits[(value & 0xf)];
	value >>= 4;
	buf[3] = hexDigits[(value & 0xf)];
	value >>= 4;
	buf[2] = hexDigits[(value & 0xf)];
	value >>= 4;
	buf[1] = hexDigits[(value & 0xf)];
	buf[0] = 'x';
	return std::string(buf);
}

int main()
{
	float f = 10.0f;
	std::string s = toString(f);
	std::cout << s << std::endl;
}