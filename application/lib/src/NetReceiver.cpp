
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>
#include <memory.h>

#include <NetRecevier.h>
#include <IrayCamera.h>

#define DATA_TYPE_FILE			0x1001
#define DATA_TYPE_IMAGE 		0x1002
#define DATA_TYPE_IMAGE_SIZE	0x1003

#define DATA_TYPE_IMG_FMT_RAW	0x1101
#define DATA_TYPE_IMG_FMT_UYVY	0X1102

#define DATA_LEN_IMAGE_SIZE		8  // width : 4 bytes, height : 4 bytes
#define DATA_LEN_INT			4  // int type data use 4 bytes

NetReceiver::NetReceiver()
{
	memset(&m_v4l2_fmt, 0x00, sizeof(struct v4l2_format));
}

NetReceiver::~NetReceiver()
{

}

int NetReceiver::bytesToInt(char* data) {
	int value = 0;
	
	value |= ((data[0] & 0xff) << 0);
	value |= ((data[1] & 0xff) << 8);
	value |= ((data[2] & 0xff) << 16);
	value |= ((data[3] & 0xff) << 24);
	
	return value;
}

void NetReceiver::intToBytes(int value, char *bytesData, int len) {
	
	bytesData[0] = (char)(value >> 0 & 0xff);
	bytesData[1] = (char)(value >> 8 & 0xff);
	bytesData[2] = (char)(value >> 16 & 0xff);
	bytesData[3] = (char)(value >> 24 & 0xff);
}

int NetReceiver::receiveFrame(IrayCameraData *frameData)
{
	int ret = 0;
	
	// iray_dbg("recvive frame data...\n");
	if (frameData == NULL) {
		return -ENODATA;
	}

	struct v4l2_format fmt;
	frameData->getFormat(&fmt);

	// send image size to client
	if ((fmt.fmt.pix.height != m_v4l2_fmt.fmt.pix.height)
		|| (fmt.fmt.pix.width != m_v4l2_fmt.fmt.pix.width)) {

		m_v4l2_fmt.fmt.pix.width = fmt.fmt.pix.width;
		m_v4l2_fmt.fmt.pix.height = fmt.fmt.pix.height;

		ret = sendFrameSizeToSocket();
		if (ret) {
			iray_err("sendFrameSizeToSocket fail, ret = %d\n", ret);
			return ret;
		}
	}

	return sendFrameDataToSocket(frameData);
}

int NetReceiver::sendFrameDataToSocket(IrayCameraData *frameData)
{
	int write_len = 0;
	char byte_data[DATA_LEN_INT] = {0};

	int length = 0;
	char *addr = NULL;
	int data_type = 0;

	if (frameData == NULL) {
		return -ENODATA;
	}

	length = frameData->getLength();
	addr = frameData->getAddr();
	if (length <= 0 || addr == NULL) {
		return -EPARAME;
	}

	if (frameData->getPixFmtType() == V4L2_PIX_FMT_UYVY) {
		data_type = DATA_TYPE_IMG_FMT_UYVY;
	} else {
		data_type = DATA_TYPE_IMG_FMT_RAW;
	}

	// write data type
	intToBytes(data_type, byte_data, DATA_LEN_INT);
	write_len = send(m_sock_fd, byte_data, DATA_LEN_INT, 0);

	// write data len
	intToBytes(length, byte_data, DATA_LEN_INT);
	write_len = send(m_sock_fd, byte_data, DATA_LEN_INT, 0);

	// write data
	write_len = send(m_sock_fd, addr, length, 0);
	if (write_len != length) {
		iray_err("socket write error, writed[%d], datalen[%d]\n", write_len, length);
		return -ENOSOCK;
	}

    return SUCCESS;
}

int NetReceiver::sendFrameSizeToSocket()
{
	int write_len = 0;
	char byte_data[DATA_LEN_INT] = {0};

	// write data type
	intToBytes(DATA_TYPE_IMAGE_SIZE, byte_data, DATA_LEN_INT);
	write_len = send(m_sock_fd, byte_data, DATA_LEN_INT, 0);

	// write data len
	intToBytes(DATA_LEN_IMAGE_SIZE, byte_data, DATA_LEN_INT);
	write_len = send(m_sock_fd, byte_data, DATA_LEN_INT, 0);

	// write image size data
	// 1.write image width
	intToBytes(m_v4l2_fmt.fmt.pix.width, byte_data, DATA_LEN_INT);
	write_len = send(m_sock_fd, byte_data, DATA_LEN_INT, 0);

	// 2.write image height	
	intToBytes(m_v4l2_fmt.fmt.pix.height, byte_data, DATA_LEN_INT);
	write_len = send(m_sock_fd, byte_data, DATA_LEN_INT, 0);
	if (write_len != DATA_LEN_INT) {
		iray_err("socket send data fail, write_len=%d\n", write_len);
	}

	return SUCCESS;
}

int NetReceiver::initSocket(char *ip_addr, int port)
{
	int ret = 0;
	int sock_fd = 0;
    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip_addr);
    server.sin_port        = htons(port);

	sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("socket fail, ret=%d\n", sock_fd);
        return -ENOSOCK;
    }

	ret = connect(sock_fd, (struct sockaddr *)&server, sizeof(struct sockaddr));
    if (ret < 0) {
        printf("connect fail, ret=%d\n", ret);
        return -ENOCONN;
    }

	m_sock_fd = sock_fd;
	return SUCCESS;  
}


