#include "./parameters.h"

Parameters::Parameters(int _argc, char *_argv[]): argc(_argc), argv(_argv) { }

void Parameters::execute()
{
	rawcount = 0;
	for(int i = 1; i < argc; ++i)
	{
		bool incrementI = false;
		std::string argstring = argv[i];
		std::string rawarg{}, value {};
		switch(getParamType(argstring)){
			case LONG_PARAM :
			    value = getParamValue(argstring);
			    if(hasValue(rawParam(argstring))){
					if(value.empty()){
						value = PARAM_MISSING();
					}
					paramMap[rawParam(argstring)] = value;
				} else {
					paramMap[rawParam(argstring)] = (value.empty()) ? PARAM_NOVALUE() : value;
				}
				break;

			case SHORT_PARAM :
			    rawarg = rawParam(argstring);
			    for(unsigned int x = 0; x != rawarg.size(); ++x){
					if(hasValue(charToString(rawarg[i]))){
						value = PARAM_MISSING();
						if(x == rawarg.size()-1){
							if((i +1 < argc) && getParamType(std::string(argv[i+1]))==OTHER_PARAM){
								value = argv[i+1];
								incrementI = true;
							}
						}
						paramMap[charToString(rawarg[x])] = value;
					}
					else {
						paramMap[charToString(rawarg[x])] = PARAM_NOVALUE();
					}
				}
			    break;
			case OTHER_PARAM :
			    paramMap[rawdatastr()]=std::string(argv[i]);
				rawcount++;
			    break;

			default:
			    break;
		}

		if(incrementI) ++i;
	}

}

Parameters::ParameterType Parameters::getParamType(std::string param)
{
	if(param.size() >0 && param[0] == '-'){
		if(param.size()>1 && param[1] == '-'){
			return LONG_PARAM;
		}
		return SHORT_PARAM;
	}
	return OTHER_PARAM;
}

std::string Parameters::getParamValue(std::string param)
{
	if(getParamType(param) == LONG_PARAM){
		auto found = std::find(param.begin(), param.end(), '=');
		if(found != param.end()){
			param.erase(param.begin(), found + 1);
			return param;
		}
	}
	return "";
}

std::string Parameters::rawParam(std::string param)
{
	switch(getParamType(param)){
		case LONG_PARAM :
		    {
                param.erase(0, 2);
		        auto found = std::find(param.begin(), param.end(), '=');
		        if(found != param.end()){
				    param.erase(found, param.end());
                }
			}
			break;
		case SHORT_PARAM:
		    param.erase(0, 1);
		    break;
		default:
		    break;
	}
	return param;
}
std::string Parameters::charToString(char c)
{
	/// string::string() does not accept "char". So, let's convert a char to a std::string !
	std::string o="";
	o+=c;
	return o;
}

bool Parameters::hasValue(std::string param){
	auto found = std::find(value_param.begin(), value_param.end(), param);
	if(found != value_param.end()){
		return true;
	}
	return false;
}

std::string Parameters::rawdatastr()
{
	/// returns the "raw data string", which means the string line __RAWDATAx__ where x stands for "rawcount"
	return std::string(PARAMETER_RAWBEGIN)+ std::to_string(rawcount) + PARAMETER_RAWEND;
}

void Parameters::setValueParam(const std::vector<std::string> &param)
{
	std::copy(param.begin(), param.end(), std::back_inserter(value_param));
}
