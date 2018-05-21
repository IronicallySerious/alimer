//
// Copyright (c) 2018 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../Util/Util.h"
using namespace std;

namespace Util
{
	vector<string> Split(
		const string &str,
		const char *delim,
		bool allowEmpty)
	{
		if (str.empty())
			return {};
		vector<std::string> ret;

		size_t start_index = 0;
		size_t index = 0;
		while ((index = str.find_first_of(delim, start_index)) != string::npos)
		{
			if (allowEmpty || index > start_index)
				ret.push_back(str.substr(start_index, index - start_index));
			start_index = index + 1;

			if (allowEmpty && (index == str.size() - 1))
				ret.emplace_back();
		}

		if (start_index < str.size())
			ret.push_back(str.substr(start_index));
		return ret;
	}

	string Replace(
		const string& str,
		const string& find,
		const string& replace,
		uint32_t maxReplacements)
	{
		string dest = str;
		size_t pos = 0;
		while ((pos = dest.find(find, pos)) != string::npos)
		{
			dest.replace(dest.find(find), find.size(), replace);
			maxReplacements--;
			pos += replace.size();
			if (maxReplacements == 0)
				break;
		}

		return dest;
	}
}
