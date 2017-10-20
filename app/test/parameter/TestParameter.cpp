#include <iostream>

#include <common/Parameter.h>

using namespace std;

int main(int argc, char *argv[])
{
	Parameter p;

	p.parse(argc, argv);
	cout << p.toString() << endl;
}
