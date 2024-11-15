#include <vector>
#include <sstream>
#include <string>

std::string vector2string(const std::vector<std::vector<std::string>> &records)
{
    std::ostringstream oss;
    for (const auto &record : records)
    {
        bool firstField = true;
        for (const auto &field : record)
        {
            if (!firstField)
            {
                oss << ","; // Add comma only before fields that are not the first in the row
            }
            oss << field;
            firstField = false;
        }
        oss << "\n";
    }
    return oss.str();
}

std::string singleVector2string(const std::vector<std::string> &record)
{
    std::ostringstream oss;
    bool firstField = true;
    for (const auto &field : record)
    {
        if (!firstField)
        {
            oss << ","; // Add comma only before fields that are not the first in the row
        }
        oss << field;
        firstField = false;
    }
    return oss.str();
}

std::vector<std::string> split(const std::string &str, char delimiter) // split string by delimiter (mainly for csv)
{
    std::vector<std::string> tokens; // parts of the string
    std::stringstream ss(str);       // string stream
    std::string token;
    while (std::getline(ss, token, delimiter)) // split by delimiter
    {
        tokens.push_back(token); // add to tokens
    }
    return tokens;
}

std::vector<std::string> csv2vector(const std::string &csv) // convert csv to 1D vector
{
    std::vector<std::string> vec;
    std::stringstream ss(csv); // string stream
    std::string item;
    while (getline(ss, item, ',')) // split by comma
    {
        vec.push_back(item); // add to vector
    }
    return vec;
}