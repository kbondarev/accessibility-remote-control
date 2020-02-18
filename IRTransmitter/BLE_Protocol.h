/*
 * This header file contains all the Bluetooch protocol codes
 * TV commands range:   0x01 - 0x40 [1-64]
 * DVD commands range:  0x41 - 0x80 [65-128]
 */

#ifndef BLE_Protocol_h
#define BLE_Protocol_h

#define TV_POWER (0x01)
#define TV_VOL_UP (0x02u)
#define TV_VOL_DOWN (0x03u)
#define TV_VOL_MUTE (0x04u)
#define TV_CHANNEL_UP (0x05u)
#define TV_CHANNEL_DOWN (0x06u)
#define TV_SOURCE (0x07u)

#define DVD_POWER (0x41u)
#define DVD_PLAY (0x42u)
#define DVD_PAUSE (0x43u)
#define DVD_STOP (0x44u)
#define DVD_SKIP_NEXT (0x45u)
#define DVD_SKIP_PREV (0x46u)
#define DVD_MENU (0x47u)
#define DVD_MENU_UP (0x48u)
#define DVD_MENU_DOWN (0x49u)
#define DVD_MENU_LEFT (0x50u)
#define DVD_MENU_RIGHT (0x51u)
#define DVD_MENU_ENTER (0x52u)
#define DVD_VOL_UP (0x53u)
#define DVD_VOL_DOWN (0x54u)
#define DVD_VOL_MUTE (0x55u)

#endif BLE_Protocol_h
