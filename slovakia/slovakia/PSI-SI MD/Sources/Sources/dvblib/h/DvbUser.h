#ifndef __DvbUser_h__
#define __DvbUser_h__


#pragma pack(1)

// Copy from card.h
typedef struct tagDvbSetCAUser {					// permissions for the (receiver) user
	unsigned char	flags;							// bit 0 = do filtering;   bit 1 = HNET allowed
	unsigned char	channelBitmap[8192];
} DvbSetCAUser ;

#define DVB_SETCAUSER			8

#pragma pack()

#endif
