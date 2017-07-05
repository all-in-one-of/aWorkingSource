#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/gmp.hpp>

#include <iostream>
#include <string.h>
#include <math.h>
#include <sstream>
#include "string2float.h"

#include <sstream>

template <class T>
inline std::string to_string (const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}


int main()
{
	using boost::multiprecision::cpp_int;
	std::string text = "295232799039604140847618609643520000000";
	//mpz_int N(567014094304930933548155069494723691156768423655208899778686163624192868328194365094673392756508907687565332345345678900976543567890976543565789054335678097654680986564323567890876532456890775646780976543556789054367890765435689876545898876587907876535976565578907654538790878656543687656543467898786565457897675645657689756456578656456768654657898865567689656890795587907654678798765787897865654657897654678965465786867278762795432151914451557727529104757415030674806148138138281214236089749601911974949125689884222023119844272122501649909415937);
	//cpp_int limit = cpp_int(295232799039604140847618609643520000000);
	cpp_int factorial = 2952327990396041408476186096435200000;
	int code = 0;
/*	for(int i = 0;i <= text.size();i++)
	{
		if(text[i])
		std::cout << text[i] << std::endl;
	}
	unsigned t = pow(2,100);
	std::string tt = to_string(pow(2,100));	
	std::cout << text.size() << std::endl;
	std::cout << tt << std::endl;*/
}