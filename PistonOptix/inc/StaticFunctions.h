#pragma once

#ifndef STATIC_FUNCTIONS_H
#define STATIC_FUNCTIONS_H

#include <string>

using namespace std;

namespace POptix 
{
	static std::string getFileName(const std::string& filePath)
	{
		char sep = '/';

#ifdef _WIN32
		sep = '\\';
#endif

		size_t i = filePath.rfind(sep, filePath.length());
		if (i != std::string::npos)
		{
			return(filePath.substr(i + 1, filePath.length() - i));
		}

		return("");
	}

	static std::string getDirectoryPath(const std::string& filename)
	{
		char sep = '/';

#ifdef _WIN32
		sep = '\\';
#endif

		const size_t last_slash_idx = filename.rfind(sep);
		if (std::string::npos != last_slash_idx)
		{
			return filename.substr(0, last_slash_idx);
		}

		return("");
	}

	static std::string getFileExtension(const std::string& filename)
	{
		char sep = '.';

		const size_t last_dot_idx = filename.rfind(sep);
		if (last_dot_idx != std::string::npos)
		{
			return(filename.substr(last_dot_idx + 1, filename.length() - last_dot_idx));
		}

		return("");
	}

}

#endif // STATIC_FUNCTIONS_H
