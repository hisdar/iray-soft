#ifndef _PARAMETER_ITEM_H_
#define _PARAMETER_ITEM_H_

#include <vector>
#include <string>
#include <common/base-def.h>

using namespace std;

class ParameterItem {

public:
	ParameterItem();
	ParameterItem(const ParameterItem &pItem);
	~ParameterItem();

	void setName(string name);
	string getName() const;

	void addValue(string value);
	void reset();

	string getValueAt(u32 index) const;
	u32 valueSize() const;

	string toString() const;

	void copy(ParameterItem &dstItem, const ParameterItem &srcItem);
	ParameterItem & operator =(const ParameterItem &pItem);

private:

	void removeAllValues();

	string m_name;
	vector<string> m_value_list;
};

#endif
