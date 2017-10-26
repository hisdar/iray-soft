#include <stdio.h>
#include <common/base-def.h>
#include <GraphicsCard.h>

int main(int argc, char *argv[])
{
	int ret = 0;
	GraphicsCard gc;

	ret = gc.init();
	if (ret) {
		iray_err("open gc fail, ret=%d\n", ret);
		return ret;
	}

	return 0;
}
