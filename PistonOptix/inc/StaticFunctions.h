#pragma once

#ifndef STATIC_FUNCTIONS_H
#define STATIC_FUNCTIONS_H

#include <string>

using namespace std;

namespace POptix 
{
	static std::string getFileName(const std::string& s)
	{
		char sep = '/';

#ifdef _WIN32
		sep = '\\';
#endif

		size_t i = s.rfind(sep, s.length());
		if (i != std::string::npos)
		{
			return(s.substr(i + 1, s.length() - i));
		}

		return("");
	}
}

#endif // STATIC_FUNCTIONS_H
