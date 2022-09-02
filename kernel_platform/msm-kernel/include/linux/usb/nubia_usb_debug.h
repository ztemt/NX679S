#ifndef __NUBIA_USB_DEBUG_H
#define __NUBIA_USB_DEBUG_H

#define NUBIA_USB_LOG_TAG "ZtemtUSB"
#define NUBIA_USB_LOG_ON

#ifdef  NUBIA_USB_LOG_ON
#define NUBIA_USB_ERROR(fmt, args...) printk(KERN_ERR "[%s] [%s: %d] "  fmt, \
        NUBIA_USB_LOG_TAG, __FUNCTION__, __LINE__, ##args)
#define NUBIA_USB_INFO(fmt, args...) printk(KERN_INFO "[%s] [%s: %d] "  fmt, \
        NUBIA_USB_LOG_TAG, __FUNCTION__, __LINE__, ##args)

#ifdef  NUBIA_USB_DEBUG_ON
#define NUBIA_USB_DEBUG(fmt, args...) printk(KERN_DEBUG "[%s] [%s: %d] "  fmt, \
        NUBIA_USB_LOG_TAG, __FUNCTION__, __LINE__, ##args)
#else
#define NUBIA_USB_DEBUG(fmt, args...)
#endif

#else
#define NUBIA_USB_ERROR(fmt, args...)
#define NUBIA_USB_INFO(fmt, args...)
#define NUBIA_USB_DEBUG(fmt, args...)
#endif

#endif /*__NUBIA_USB_DEBUG_H*/

