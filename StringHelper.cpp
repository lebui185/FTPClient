#include "StringHelper.h"

void StringHelper::Split(const std::string &str, std::vector<std::string> &tokens,
                         const std::string &delimiters, bool trimEmpty)
{
    std::string::size_type pos, lastPos = 0;
    
    while(true)
    {
        pos = str.find_first_of(delimiters, lastPos);
        if(pos == std::string::npos)
        {
            pos = str.length();

            if(pos != lastPos || !trimEmpty)
            {
                tokens.push_back(str.substr(lastPos, pos - lastPos));
            }
            break;
        }
        else
        {
            if(pos != lastPos || !trimEmpty)
            {
                tokens.push_back(str.substr(lastPos, pos - lastPos));
            }
        }

        lastPos = pos + 1;
     }
}

void StringHelper::ReplaceAll(std::string& str, const std::string& search,
		const std::string& replacement)
{
	size_t pos = 0;

	while ((pos = str.find(search, pos)) != std::string::npos)
	{
		str.replace(pos, search.length(), replacement);
		pos += replacement.length();
	}
}
