#pragma once

class Convert()
{
public:
	TCHAR* StringToTCHAR(std::string Argument);
}
	
TCHAR* Convert::StringToTCHAR(std::string Argument)
{
	std::string Argument_string = Argument;
	/** string to char */
	char Argument_char[MAX_PATH];
	int i;
	for (i = 0; i<Argument_string.length(); i++)
		Argument_char[i] = Argument_string[i];
	Argument_char[i] = '\0';
	/** char to TCHAR */
	TCHAR* Argument_TCHAR = new TCHAR[MAX_PATH];
	char Argument_char2[MAX_PATH];
	sprintf_s(Argument_char2, "%s", Argument_char);
	int iLength = MultiByteToWideChar(CP_ACP, 0, Argument_char2, strlen(Argument_char2) + 1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, Argument_char2, strlen(Argument_char2) + 1, Argument_TCHAR, iLength);
	TCHAR* Result = Argument_TCHAR;
	return Result;
}