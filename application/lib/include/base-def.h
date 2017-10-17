#ifndef __BASE_DEF_H__
#define __BASE_DEF_H__

#ifndef u32
#define u32 unsigned int
#endif

#define SUCCESS        0
#define ENOSOCK        1
#define ENOCONN        2
#define EPARAME        3
#define ENOIOCT        4

#define ENOFOPN        5

#ifndef ENOMEM
#define ENOMEM         6
#endif

#ifndef ENODATA
#define ENODATA        7
#endif

#define ENOPARA        8

#define CHECK_FREE(ptr) \
    if (ptr != NULL) {  \
        free(ptr);      \
        ptr = NULL;     \
    }


    
#define IRAY_DEBUG
    
#ifdef IRAY_DEBUG
#define iray_dbg(fmt, arg...)    \
            printf("%s%s-%d:" fmt , "[Iray][dbg]", __func__, __LINE__, ##arg)
#else
#define iray_dbg(fmt, arg...)
#endif
    
#define iray_err(fmt, arg...)    \
            printf("%s%s-%d:" fmt , "[Iray][err]", __func__, __LINE__, ##arg)
#define iray_info(fmt, arg...)    \
            printf("%s%s-%d:" fmt , "[Iray][inf]", __func__, __LINE__, ##arg)

#endif
