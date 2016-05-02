#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include <string>
#include <vector>

class StringHelper
{
public:
    static void Split(const std::string &str, std::vector<std::string> &tokens,
                      const std::string &delimiters = " ", bool trimEmpty = true);
    static void ReplaceAll(std::string &str, const std::string &search,
    						const std::string &replacement);
};

#endif /* STRINGHELPER_H */

