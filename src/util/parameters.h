#ifndef UTIL_PARAMETERS_H
#define UTIL_PARAMETERS_H

#include <string>
#include <map>
#include <vector>

class Parameters : public std::multimap<std::string, std::string> {
public:

    Parameters() = default;
    Parameters(Parameters &other)  = default;
    Parameters(Parameters &&other) = default;


    bool hasParam(const std::string& key) const;
    //the getX functions return the last parameter of with that name inserted.
    const std::string &get(const std::string &name) const;
    const std::string &get(const std::string &name, const std::string &defaultValue) const;
    int getInt(const std::string &name) const;
    int getInt(const std::string &name, int defaultValue) const;
    long getLong(const std::string &name) const;
    long getLong(const std::string &name, long defaultValue) const;
    bool getBool(const std::string &name) const;
    bool getBool(const std::string &name, bool defaultValue) const;

    std::vector<std::string> getAll(const std::string &name) const;
    std::vector<int> getAllInt(const std::string &name) const;
    std::vector<long> getAllLong(const std::string &name) const;
    std::vector<bool> getAllBool(const std::string &name) const;


    /**
     * Returns all parameters with a given prefix, with the prefix stripped.
     * For example, if you have the configurations
     *  my.module.paramA = 50
     *  my.module.paramB = 20
     * then parameters.getPrefixedParameters("my.module.") will return a Parameters object with
     *  paramA = 50
     *  paramB = 20
     *
     * @param prefix the prefix of the interesting parameter names. Usually, this should end with a dot.
     */
    Parameters getPrefixedParameters(const std::string &prefix);

    // These do throw exceptions when the string cannot be parsed.
    static int parseInt(const std::string &str);
    static long parseLong(const std::string &str);
    static bool parseBool(const std::string &str);
};

#endif //UTIL_PARAMETERS_H
