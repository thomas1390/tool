/*
Description:
This file implements the Secure Hashing Algorithm 1 as defined 
in FIPS PUB 180-1 published April 17, 1995.

The SHA-1, produces a 160-bit message digest for a given data 
stream.This algorithm can serve as a means of providing a 
"fingerprint" for a message.

It should take about 2**n steps to find a message with the same digest as a given message and
2**(n/2) to find any two messages with the same digest,when n is the digest size in bits.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
compute_length_byte (int length_bits)
{
  int length_byte;
  length_byte = 1;
  while (1)
    {
      length_bits = length_bits / 256;
      if (length_bits == 0)
	break;
      length_byte++;
    }
  return length_byte;
}

unsigned int
f (int t, unsigned int B, unsigned int C, unsigned int D)
{
  if (0 <= t && t <= 19)
    return ((B & C) | ((~B) & D));
  if (20 <= t && t <= 39)
    return (B ^ C ^ D);
  if (40 <= t && t <= 59)
    return ((B & C) | (B & D) | (C & D));
  if (60 <= t && t <= 79)
    return (B ^ C ^ D);
}

unsigned int
K (int t)
{
  if (-1 < t && t < 20)
    return 0x5A827999;
  if (19 < t && t < 40)
    return 0x6ED9EBA1;
  if (39 < t && t < 60)
    return 0x8F1BBCDC;
  if (59 < t && t < 80)
    return 0xCA62C1D6;
}

void
usage (char *s)
{
  printf ("\tusage: %s text\n", s);
}

main (int argc, char *argv[])
{
  unsigned int H0 = 0x67452301;
  unsigned int H1 = 0xEFCDAB89;
  unsigned int H2 = 0x98BADCFE;
  unsigned int H3 = 0x10325476;
  unsigned int H4 = 0xC3D2E1F0;
  unsigned int A, B, C, D, E;
  int i, j;
  unsigned int W[80], temp;

//usage()
  if (argc != 2)
    {
      usage (argv[0]);
      exit (0);
    }

//calculate the message's total length
  int length_str, length_bits, length_byte, length_final;
  //M:16-word blocks
  //Char的取值范围是-128~127，所以要用unsigned
  unsigned char *M;
  length_str = strlen (argv[1]);
  length_bits = length_str * 8;
  length_byte = compute_length_byte (length_bits);
  i = 1;
  while (1)
    {				//容易出错的地方
      //why 1+64? 1:"10000000" =0x80        64:8 byte length
      if (i * 512 - (length_bits + 1 + 64) >= 0)
	{
	  length_final = i * 512;
	  break;
	}
      i++;
    }
/*
  printf
    ("length_str: %d bytes,length_final: %d bits,length_byte: %d bytes\n",
     length_str, length_final, length_byte);
*/

//padding the origin message
  M = malloc (length_final / 8);
  for (i = 0; i < length_final / 8; i++)
    *(M + i) = 0x00;
  memcpy (M, argv[1], length_str);
  *(M + length_str) = 0x80;
  while (length_byte != 0)
    {
      *(M + length_final / 8 - length_byte) =
	((length_bits >> (length_byte - 1) * 8) & 0xFF);
      --length_byte;
    }

/*
  for (i = 0; i < (length_final / 8); i++)
    {				//%x输出4个byte
      printf ("%x ", *(M + i));
    }
  printf ("\n");
*/

//cipher process
  for (i = 0; i < length_final / 512; ++i)
    {
      //易出错firstly,zero the W[80] array
      for (j = 0; j < 80; j++)
	{
	  W[j] = 0x00;
	}
      //then,calculate the SHA sum
      for (j = 0; j < 16; j++)
	{
	  temp = *(M + i * 64 + j * 4 + 0);
	  //这里的强制类型转换不一定正确
	  W[j] += (((unsigned int) temp) << 24);
	  temp = *(M + i * 64 + j * 4 + 1);
	  W[j] += (((unsigned int) temp) << 16);
	  temp = *(M + i * 64 + j * 4 + 2);
	  W[j] += (((unsigned int) temp) << 8);
	  temp = *(M + i * 64 + j * 4 + 3);
	  W[j] += (unsigned int) temp;
//        printf ("%x ", W[j]);
	}
      for (j = 16; j <= 79; j++)
	{
	  W[j] = (W[j - 3] ^ W[j - 8] ^ W[j - 14] ^ W[j - 16]);
	  W[j] = ((W[j] << 1) | (W[j] >> 31));
//        printf ("%x ", W[j]);
	}
      A = H0;
      B = H1;
      C = H2;
      D = H3;
      E = H4;
      for (j = 0; j <= 79; j++)
	{
	  temp = ((A << 5) | (A >> 27)) + f (j, B, C, D) + E + W[j] + K (j);
	  E = D;
	  D = C;
	  C = ((B << 30) | (B >> 2));
	  B = A;
	  A = temp;
	}
      H0 += A;
      H1 += B;
      H2 += C;
      H3 += D;
      H4 += E;
    }
//output message
  printf ("SHA-1:%x %x %x %x %x\n", H0, H1, H2, H3, H4);
  free (M);
}
