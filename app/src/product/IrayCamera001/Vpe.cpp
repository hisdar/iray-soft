#include <Vpe.h>

Vpe::Vpe()
{
	m_output = NULL;
	m_vpeDev = NULL;
	m_capture = NULL;
}

Vpe::~Vpe()
{
	delete m_capture;
	m_capture = NULL;

	delete m_output;
	m_output = NULL;

	delete m_vpeDev;
	m_vpeDev = NULL;
}

int Vpe::init(int srcWidth, int srcHeight, int srcFmt, int dstWidth, int dstHeight, int dstFmt)
{
	int ret = 0;
	m_vpeDev = new VpeDev();
	ret = m_vpeDev->init(VPE_DEVICE_PATH);
	if (ret) {
		iray_err("vpe device open fail, ret=%d, path=%s\n", ret, VPE_DEVICE_PATH);
		return ret;
	}

	m_output = new VpeOutput(m_vpeDev);
	ret = m_output->init(srcWidth, srcHeight, srcFmt);
	if (ret) {
		iray_err("vpe output init fail, ret=%d\n", ret);
		return ret;
	}

	m_capture = new VpeCapture(m_vpeDev);
	ret = m_capture->init(dstWidth, dstHeight, dstFmt);
	if (ret) {
		iray_err("vpe capture init fail, ret=%d\n", ret);
		return ret;
	}

	return SUCCESS;
}

int Vpe::put(void *data, u32 len)
{
	return m_output->ouput(data, len);
}

int Vpe::get(void *data, u32 len)
{
	return m_capture->capture(data, len);
}

