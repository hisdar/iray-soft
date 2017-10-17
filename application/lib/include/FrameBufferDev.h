#ifndef _FRAME_BUFFER_DEV_H_
#define _FRAME_BUFFER_DEV_H_

#include <linux/fb.h>
#include <IrayCameraRcv.h>
#include "IrayRgbImage.h"

#define SUCCESS 0

#ifndef ENODEV
#define ENODEV    1
#endif

#ifndef u32
#define u32 unsigned int
#endif

class FrameBufferDev : public IrayCameraRcv {

public:
    FrameBufferDev();
    ~FrameBufferDev();

    int open(const char *dev_path);
    int showScreenInfo();
    int prepareOutput();
    int outputImage(IrayRgbImage *img, int x, int y, int pix);
    
    int receiveFrame(IrayCameraData *frameData);
private:
    int m_fb;
    u32 m_fb_buf_len;
    char *m_fb_mem_addr;
    
    struct fb_fix_screeninfo m_fix;
    struct fb_var_screeninfo m_var;
    struct fb_con2fbmap      m_con;

    int queryScreenInfo();
    int prepare_output();
};

#endif