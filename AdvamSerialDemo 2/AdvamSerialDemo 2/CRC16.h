#pragma once

enum
{
	DIN=0x8005,
	CCITT=0x1021
};

void setPoly(unsigned short poly);

unsigned short getCRC16(const void *mem, int len, unsigned short crc);