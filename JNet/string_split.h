#include <string>
#include <vector>
static std::string* string_split(std::string str, char delim)
{
	int offset = 0;
	std::vector<std::string> splits;
	while (true)
	{
		size_t pos = str.find(delim, offset);

		if (pos == std::string::npos) // last delim was found in previous search so remainder is the last string
		{
			splits.push_back(str.substr(offset, str.length() - offset));
			break;
		}

		splits.push_back(str.substr(offset, pos - offset));
		offset = pos+1;
	}

	// copy data from dynamic array to static array
	std::string* stringArray = new std::string[splits.size()];
	for (int i = 0; i < splits.size(); i++)
		stringArray[i] = splits[i];

	return stringArray;
}