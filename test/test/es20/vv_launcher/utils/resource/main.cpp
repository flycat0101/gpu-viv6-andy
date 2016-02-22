/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>

#ifdef _WIN32
// for CTime
#include <atltime.h>
#endif


int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << argv[0] << " expects <infile>" << std::endl;
		return -1;
	}

	std::string infile = argv[1];

	/*
	 Because different build systems unfortunately will produce
	 different paths to the original source, the path can't practically
	 be a part of the resource name. (One alternative is to provide
	 the resource name and output file path explicitly.)
	*/

#ifdef _WIN32
	char sep = '\\';
#else
	char sep = '/';
#endif

	size_t pos = infile.rfind(sep);
	std::string name;
	char rc_name[255];
	if (pos != std::string::npos)
	{
		name = infile.substr(pos+1);
	}
	else
	{
		name = infile;
	}

	strcpy(rc_name, "_");
	strcat(rc_name, name.c_str());
	for (size_t i = 0; i < strlen(rc_name) && rc_name[i] != '\0'; i++)
	{
		if (!isdigit(rc_name[i]) && !isalpha(rc_name[i]) )
		{
			rc_name[i] = '_';
		}
	}


	std::string outfile = infile + ".c";

	std::ifstream in;
	in.open(infile.c_str(), std::ios::binary);

	if (!in)
	{
		std::cerr << argv[0] << ": can't open input file" << std::endl;
		perror(infile.c_str());
		return -1;
	}

	std::ofstream out;
	out.open(outfile.c_str());
	if (!out)
	{
		std::cerr << argv[0] << ": can't open output file" << std::endl;
		perror(outfile.c_str());
	}

	int size = 0;

#ifdef _WIN32
	{
		CTime now = CTime::GetCurrentTime();
		CString s = now.Format( "%A, %B %d, %Y at %H:%M:%S" );
		const char* date = s;

		out << "//   Resource: " << name << "\n"
			<< "//       Path: " << outfile << "\n//\n"
			<< "//  Generated: " << date << "\n"
			<< "//         By: " << argv[0] << "\n\n";
	}
#else
	{
		time_t now;
		time(&now);
		const char* date = ctime(&now);

		out << "// Resource file: " << outfile << "\n"
			<< "// Generated at: " << date << "\n"
			<< "// By: " << argv[0] << "\n\n";
	}
#endif

	out << "#include <rc/resource.h>\n\n"
		<< "static unsigned char "
		<< rc_name
		<<"_data[] = {\n";

	char c;
	while (in.get(c))
	{

		unsigned char uc = (unsigned char)(c);

		if (size%10==0) out << "\n\t";
		out << (unsigned int)(uc) << ",";
		++size;
	}

	out << "\n};\n\n";

	out <<"Resource "<<rc_name<<" = \n"
		<<"{\n"
		<<"\t\""<<name<<"\",\n"
		<<"\t"<<rc_name<<"_data,\n"
		<<"\t"<<size<<"\n"
		<<"};\n\n";

	return 0;
}

