#ifndef _USB_H_
#define _USB_H_
//DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: All necessary USB Library headers
// *****************************************************************************
// *****************************************************************************

#include "GenericTypeDefs.h"
#include "Compiler.h"

#include "usb_config.h"             // Must be defined by the application

#include "USB/usb_common.h"         // Common USB library definitions
#include "USB/usb_ch9.h"            // USB device framework definitions

#if defined( USB_SUPPORT_DEVICE )
    #include "USB/usb_device.h"     // USB Device abstraction layer interface
#endif

#if defined( USB_SUPPORT_HOST )
    #include "USB/usb_host.h"       // USB Host abstraction layer interface
#endif

#if defined ( USB_SUPPORT_OTG )
    #include "USB/usb_otg.h"
#endif

#include "USB/usb_hal.h"            // Hardware Abstraction Layer interface

// *****************************************************************************
// *****************************************************************************
// Section: MCHPFSUSB Firmware Version
// *****************************************************************************
// *****************************************************************************

#define USB_MAJOR_VER   2       // Firmware version, major release number.
#define USB_MINOR_VER   9       // Firmware version, minor release number.
#define USB_DOT_VER     0       // Firmware version, dot release number.

#endif // _USB_H_
/*************************************************************************
 * EOF
 */

