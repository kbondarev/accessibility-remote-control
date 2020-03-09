/*
 * This header file contains all the Bluetooch protocol codes
 * TV commands range:   1 - 40
 * DVD commands range:  41 - 80
 */

#ifndef BLE_Protocol_h
#define BLE_Protocol_h

#define TV_POWER (1u)
#define TV_VOL_UP (2u)
#define TV_VOL_DOWN (3u)
#define TV_VOL_MUTE (4u)
#define TV_CHANNEL_UP (5u)
#define TV_CHANNEL_DOWN (6u)
#define TV_SOURCE (7u)

#define DVD_POWER (41u)
#define DVD_PLAY (42u)
#define DVD_PAUSE (43u)
#define DVD_STOP (44u)
#define DVD_SKIP_NEXT (45u)
#define DVD_SKIP_PREV (46u)
#define DVD_MENU (47u)
#define DVD_MENU_UP (48u)
#define DVD_MENU_DOWN (49u)
#define DVD_MENU_LEFT (50u)
#define DVD_MENU_RIGHT (51u)
#define DVD_MENU_ENTER (52u)
#define DVD_VOL_UP (53u)
#define DVD_VOL_DOWN (54u)
#define DVD_VOL_MUTE (55u)

#endif BLE_Protocol_hk
