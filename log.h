#ifndef __LOG_H__
#define __LOG_H__

#undef LOG_TAG
#define LOG_TAG "jm"

#define ERR(fmt, arg...) \
    ({ do { \
        printf("E " LOG_TAG " %s: %s(%d) > " fmt "\n", __FILE__, __func__, __LINE__, ##arg); \
     } while (0); })

#define DBG(fmt, arg...) \
    ({ do { \
        printf("D " LOG_TAG " %s: %s(%d) > " fmt "\n", __FILE__, __func__, __LINE__, ##arg); \
     } while (0); })

#define INF(fmt, arg...) \
    ({ do { \
        printf("I " LOG_TAG " %s: %s(%d) > " fmt "\n", __FILE__, __func__, __LINE__, ##arg); \
     } while (0); })

#define WRN(fmt, arg...) \
    ({ do { \
        printf("W " LOG_TAG " %s: %s(%d) > " fmt "\n", __FILE__, __func__, __LINE__, ##arg); \
    } while (0); })

#endif
