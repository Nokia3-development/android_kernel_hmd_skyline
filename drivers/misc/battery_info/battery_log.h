
#ifndef LOG_TAG
#error "Please define LOG_TAG before include log.h"
#endif

#define Loge(format, ...) pr_err(LOG_TAG  "[%s][%04d]" format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define Logd(format, ...) pr_info(LOG_TAG  "[%s][%04d]" format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define Logi(format, ...)  pr_info(LOG_TAG  "[%s][%04d]" format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define Logw(format, ...) pr_warn(LOG_TAG  "[%s][%04d]" format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);

