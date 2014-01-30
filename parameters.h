#ifndef _PARAMETERS_H
#define _PARAMETERS_H

#include <string>
#include <map>
#include <vector>
#include <algorithm>

#define PARAMETER_RAWBEGIN "__RAWDATA"
#define PARAMETER_RAWEND "__"

typedef std::map<std::string, std::string> ParameterMap;

class Parameters
{
public:
	enum ParameterType {
		SHORT_PARAM, LONG_PARAM, OTHER_PARAM
	};
	Parameters(int _argc, char *_argv[]);
	Parameters() = delete;

	void execute();
	ParameterType getParamType(std::string param);
	std::string getParamValue(std::string);
	bool hasValue(std::string);
	std::string rawParam(std::string);
	std::string rawdatastr();
	std::string charToString(char);
	//void addValueParam(const std::string &param);
	void setValueParam(const std::vector<std::string> &);
	ParameterMap getMap(){
		return paramMap;
	}

private:
    ParameterMap paramMap;
    int argc;
    int rawcount;
    char **argv;
    std::vector<std::string> value_param;
};

inline std::string PARAM_MISSING(){
	return "__MISSING";
}
inline std::string PARAM_NOVALUE(){
	return "__NO_VALUE";
}

#endif

