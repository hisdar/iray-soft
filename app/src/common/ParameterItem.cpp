#include <common/ParameterItem.h>

#include <iostream>
using namespace std;

ParameterItem::ParameterItem()
{

}

ParameterItem::ParameterItem(const ParameterItem &pItem)
{
	copy(*this, pItem);
}


ParameterItem::~ParameterItem()
{
	removeAllValues();
}

void ParameterItem::removeAllValues()
{
	while (m_value_list.size() > 0) {
		m_value_list.pop_back();
	}
}

void ParameterItem::reset()
{
	m_name = "";
	removeAllValues();
}


void ParameterItem::setName(string name)
{
	m_name = name;
}

string ParameterItem::getName() const
{
	return m_name;
}

void ParameterItem::addValue(string value)
{
	// TODO: check !!!
	m_value_list.push_back(string(value));
}

string ParameterItem::getValueAt(u32 index) const
{
	if (index < m_value_list.size()) {
		return m_value_list.at(index);
	}

	return string("");
}

u32 ParameterItem::valueSize() const
{
	return m_value_list.size();
}

void ParameterItem::copy(ParameterItem &dstItem, const ParameterItem &srcItem)
{
	dstItem.reset();

	dstItem.setName(srcItem.getName());

	for (u32 i = 0; i < srcItem.valueSize(); i++) {
		dstItem.addValue(srcItem.getValueAt(i));
	}

}

ParameterItem & ParameterItem::operator =(const ParameterItem &pItem)
{
	copy(*this, pItem);

	return *this;
}

string ParameterItem::toString() const
{
	string str;

	str = m_name + ":";

	for (u32 i = 0; i < valueSize(); i++) {
		str += getValueAt(i) + " ";
	}

	return str;
}



