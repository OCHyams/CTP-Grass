#include "Shaders.h"
#include<fstream>


bool DXHelper::readBytesFromFile(const char* _fpath, char** _out_bytes, int* _out_length)
{
	if (*_out_bytes != nullptr) 
		return false;

	std::fstream fileStream;
	fileStream.open(_fpath, std::ifstream::in);

	if (fileStream.is_open() == false)
		return false;

	fileStream.seekg(0, std::ios::end);
	*_out_length = (int)fileStream.tellg();
	fileStream.seekg(0, std::ios::beg);

	if (*_out_length <= 0)
		return false;

	*_out_bytes = new char[*_out_length];
	fileStream.read(*_out_bytes, *_out_length);
	return true;
}
