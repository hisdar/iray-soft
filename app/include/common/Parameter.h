#ifndef _PARAMETER_H_
#define _PARAMETER_H_

#include <vector>
#include <common/ParameterItem.h>
#include <common/base-def.h>

using namespace std;

#define MAX_PARAMETER_NAME	256

class Parameter {
public:
	Parameter();
	Parameter(const Parameter &src);
	Parameter(int argc, char *argv[]);
	~Parameter();

	int parse(int argc, char *argv[]);
	string toString() const;

	void removeAllParameter();
	void addParameterItem(const ParameterItem &item);
	void copy(Parameter &dst, const Parameter &src);

	string getValueByName(string name);
	int getValueByName(const char *name, char *value, int len);
	void getValueByNameDefault(const char *name, char *value, int len, const char *def_value);
	int getIntByNameDefault(const char *name, int def_val);
	ParameterItem getParameterItemAt(u32 index) const;
	u32 size() const;
	int stringToInt(string str);

	Parameter & operator =(const Parameter &pItem);

private:
	vector<ParameterItem> m_parameters;
};


#endif
