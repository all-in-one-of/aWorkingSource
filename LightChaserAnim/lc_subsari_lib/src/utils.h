#pragma once

#include <algorithm>
#include <cstring>
#include <cmath>
#include <ai.h>

namespace kt
{// starting namespace

AtVector min(AtVector a,AtVector b)
{
	AtVector result;
	result.x = std::min(a.x,b.x);
	result.y = std::min(a.y,b.y);
	result.z = std::min(a.z,b.z);
	return	result;	
}

AtVector max(AtVector a, AtVector b)
{
	AtVector result;
	result.x = std::max(a.x,b.x);
	result.y = std::max(a.y,b.y);
	result.z = std::max(a.z,b.z);
	return	result;	
}

template<typename genDType>

genDType mix(genDType x, genDType y, genDType a)
{
	genDType result;
	result = x*(1 - a) + y*a;
	return result;
}

template<typename genUType>

genUType clamp(genUType x, genUType minVal, genUType maxVal)
{
	genUType result;
	result = kt::min(kt::max(x, minVal), maxVal);
	return result;
}

template<typename genMType>

AtRGB AtVector2AtRGB(genMType vec)
{
	AtRGB result;
	result.r = vec.x;
	result.g = vec.y;
	result.b = vec.z;
	return result;
}

}//ending namespace
