#include <common/Parameter.h>
#include <common/ParameterItem.h>

Parameter::Parameter()
{
	
}

Parameter::Parameter(const Parameter &src)
{
	if (this != &src) {
		copy(*this, src);
	}
}


Parameter::Parameter(int argc, char *argv[])
{
	parse(argc, argv);
}

Parameter::~Parameter()
{
	while (m_parameters.size() > 0) {
		m_parameters.pop_back();
	}
}

int Parameter::parse(int argc, char *argv[])
{
	ParameterItem pItem;

	for (int i = 1; i < argc; i++) {
		// parameter start
		if (argv[i][0] == '-') {
			if (pItem.getName().size() > 0) {
				m_parameters.push_back(pItem);
			}

			pItem.reset();
			pItem.setName(string(argv[i]));
		} else {
			pItem.addValue(string(argv[i]));
		}
	}

	if (pItem.getName().size() > 0) {
		m_parameters.push_back(pItem);
	}

	return 0;
}

string Parameter::toString() const
{
	string str = "";
	vector<ParameterItem>::const_iterator it;

	for (it = m_parameters.begin(); it != m_parameters.end(); it++) {
		str += (*it).toString() + "\n";
	}

	return str;
}

void Parameter::copy(Parameter &dst, const Parameter &src)
{
	removeAllParameter();
	for (u32 i = 0; i < src.size(); i++) {
		dst.addParameterItem(src.getParameterItemAt(i));
	}
}

u32 Parameter::size() const
{
	return m_parameters.size();
}

ParameterItem Parameter::getParameterItemAt(u32 index) const
{
	return m_parameters.at(index);
}

void Parameter::addParameterItem(const ParameterItem &item)
{
	m_parameters.push_back(item);
}

void Parameter::removeAllParameter()
{
	while (m_parameters.size() > 0) {
		m_parameters.pop_back();
	}
}

Parameter & Parameter::operator =(const Parameter &pItem)
{
	if (this != &pItem) {
		copy(*this, pItem);
	}

	return *this;
}

string Parameter::getValueByName(string name)
{
	for (u32 i = 0; i < size(); i++) {
		ParameterItem pItem = getParameterItemAt(i);
		if (pItem.getName().compare(name) == 0) {
			return pItem.getValueAt(0);
		}
	}

	return string("");
}


