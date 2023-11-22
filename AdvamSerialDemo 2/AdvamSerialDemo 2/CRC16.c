#include "CRC16.h"

static unsigned short crctab[256];

void setPoly(unsigned short poly)
{
  unsigned char j, ch;
  unsigned short  i, check, check2;

  for (i=0; i<256; i++)
  {
     check = 0;
     ch = (unsigned char)i;
     for (j=0; j<8; j++)
	 {
        if (check & 0x8000)
		{
           check <<= 1;
           if (! (ch & 1))
              check ^= poly;
        }
        else
		{
           check <<= 1;
           if (ch & 1)
              check ^= poly;
        };
        ch >>= 1;
     };

     check2 = 0;
     for (j=0; j<16; j++)
	 {
         ch = check & 1;
         check >>= 1;
         check2 <<= 1;
         check2 |= ch;
     };
     crctab [i] = check2;
  };
}

unsigned short getCRC16 (const void *mem, int length, unsigned short crc)
{
  unsigned char *block=(unsigned char *)mem;
  int i;

  for (i = 0; i < length; i++)
{
    crc= (crc >> 8 ) ^ crctab[(block[i] ^ crc) & 0xff];
  };

  return (crc);
}