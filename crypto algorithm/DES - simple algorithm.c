/*
DES, on which TDES is based, was originally designed by Horst Feistel at IBM in 1974, and was adopted as a standard by NIST (formerly NBS).
http://csrc.nist.gov/publications/fips/fips46-3/fips46-3.pdf
*/
#include <stdio.h>
#include <string.h>

int First_Tb[64] = {
  58, 50, 42, 34, 26, 18, 10, 2,
  60, 52, 44, 36, 28, 20, 12, 4,
  62, 54, 46, 38, 30, 22, 14, 6,
  64, 56, 48, 40, 32, 24, 16, 8,
  57, 49, 41, 33, 25, 17, 9, 1,
  59, 51, 43, 35, 27, 19, 11, 3,
  61, 53, 45, 37, 29, 21, 13, 5,
  63, 55, 47, 39, 31, 23, 15, 7
};

int Second_Tb[56] = {
  57, 49, 41, 33, 25, 17, 9, 1,
  58, 50, 42, 34, 26, 18, 10, 2,
  59, 51, 43, 35, 27, 19, 11, 3,
  60, 52, 44, 36, 63, 55, 47, 39,
  31, 23, 15, 7, 62, 54, 46, 38,
  30, 22, 14, 6, 61, 53, 45, 37,
  29, 21, 13, 5, 28, 20, 12, 4
};

int Third_Tb[48] = {
  14, 17, 11, 24, 1, 5, 3, 28, 15, 6, 21, 10,
  23, 19, 12, 4, 26, 8, 16, 7, 27, 20, 13, 2,
  41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
  44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32
};

int LOOP_Table[16] = { 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 };

int Forth_Tb[48] = {
  32, 1, 2, 3, 4, 5, 4, 5,
  6, 7, 8, 9, 8, 9, 10, 11,
  12, 13, 12, 13, 14, 15, 16, 17,
  16, 17, 18, 19, 20, 21, 20, 21,
  22, 23, 24, 25, 24, 25, 26, 27,
  28, 29, 28, 29, 30, 31, 32, 1
};

int Fifth_Tb[32] = {
  16, 7, 20, 21, 29, 12, 28, 17,
  1, 15, 23, 26, 5, 18, 31, 10,
  2, 8, 24, 14, 32, 27, 3, 9,
  19, 13, 30, 6, 22, 11, 4, 25
};

int Last_Tb[64] = {
  40, 8, 48, 16, 56, 24, 64, 32,
  39, 7, 47, 15, 55, 23, 63, 31,
  38, 6, 46, 14, 54, 22, 62, 30,
  37, 5, 45, 13, 53, 21, 61, 29,
  36, 4, 44, 12, 52, 20, 60, 28,
  35, 3, 43, 11, 51, 19, 59, 27,
  34, 2, 42, 10, 50, 18, 58, 26,
  33, 1, 41, 9, 49, 17, 57, 25
};

int S_Box[8][4][16] = {
  // S1    
  14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7,
  0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8,
  4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0,
  15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13,
  // S2    
  15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10,
  3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5,
  0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15,
  13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9,
  // S3    
  10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8,
  13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1,
  13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7,
  1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12,
  // S4    
  7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15,
  13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9,
  10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4,
  3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14,
  // S5    
  2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9,
  14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6,
  4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14,
  11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3,
  // S6    
  12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11,
  10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8,
  9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6,
  4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13,
  // S7    
  4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1,
  13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6,
  1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2,
  6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12,
  // S8    
  13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7,
  1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2,
  7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8,
  2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11
};

int *
crypt (int plain[64], int key[64])
{
  int i, m, n;
  int first_round[64];
  int second_round[56];
  int low[28], high[28];
  int third_round[48];
  int subkey[16][48];
  int l[32], r[32];
  int fourth_round[32];
  int swap, tmp;
  int fifth_round[32];
  int tmp_array[32];
  int sixth_round[64];
  int seventh_round[64];
  for (i = 0; i < 56; i++)
    second_round[i] = key[Second_Tb[i] - 1];
  for (i = 0; i < 28; i++)
    low[i] = second_round[i];
  for (i = 0; i < 28; i++)
    high[i] = second_round[i + 28];
  for (i = 0; i < 16; i++)
    {
      //下面的是循环移位
      for (m = 0; m < LOOP_Table[i]; m++)
	{
	  swap = low[0];
	  for (n = 0; n < 27; ++n)
	    low[n] = low[n + 1];
	  low[27] = swap;
	  swap = high[0];
	  for (n = 0; n < 27; ++n)
	    high[n] = high[n + 1];
	  high[27] = swap;
	}
      for (n = 0; n < 28; n++)
	{
	  second_round[n] = low[n];
	  second_round[n + 28] = high[n];
	}
      for (m = 0; m < 48; m++)
	subkey[i][m] = second_round[Third_Tb[m] - 1];
    }
  for (i = 0; i < 64; i++)
    first_round[i] = plain[First_Tb[i] - 1];
  for (i = 0; i < 32; i++)
    l[i] = first_round[i];
  for (i = 0; i < 32; i++)
    r[i] = first_round[i + 32];
  for (i = 0; i < 16; i++)
    {
      for (m = 0; m < 48; m++)
	third_round[m] = r[Forth_Tb[m] - 1];
      for (m = 0; m < 48; m++)
	third_round[m] = third_round[m] ^ subkey[i][m];
      for (m = 0; m < 8; m++)
	{
	  tmp =
	    S_Box[m][third_round[6 * m + 0] * 2 +
		     third_round[6 * m + 5]][third_round[6 * m + 1] * 8 +
					     third_round[6 * m + 2] * 4 +
					     third_round[6 * m + 3] * 2 +
					     third_round[6 * m + 4]];
	  fourth_round[4 * m + 3] = tmp % 2;
	  tmp = tmp / 2;
	  fourth_round[4 * m + 2] = tmp % 2;
	  tmp = tmp / 2;
	  fourth_round[4 * m + 1] = tmp % 2;
	  tmp = tmp / 2;
	  fourth_round[4 * m + 0] = tmp % 2;
	}
      for (m = 0; m < 32; m++)
	fifth_round[m] = fourth_round[Fifth_Tb[m] - 1];
      for (m = 0; m < 32; m++)
	{
	  tmp_array[m] = l[m];
	  l[m] = r[m];
	  r[m] = fifth_round[m] ^ tmp_array[m];
	}
    }

//注意:这里的低32位和高32位与开始时相反
  for (i = 0; i < 32; i++)
    {
      sixth_round[i] = r[i];
      sixth_round[i + 32] = l[i];
    }
  for (i = 0; i < 64; i++)
    {
      seventh_round[i] = sixth_round[Last_Tb[i] - 1];
    }
  printf ("the Encrypted data :\n\t");
  for (i = 0; i < 64; i++)
    {
      printf ("%d ", seventh_round[i]);
      if ((i + 1) % 8 == 0 && i != 63)
	printf ("\n\t");
      if (i == 63)
	printf ("\n");
    }
  memcpy (plain, seventh_round, 64);
}

int *
decrypt (int ciphertext[64], int key[64])
{
  int i, m, n;
  int first_round[64];
  int second_round[56];
  int low[28], high[28];
  int third_round[48];
  int subkey[16][48];
  int l[32], r[32];
  int fourth_round[32];
  int swap, tmp;
  int fifth_round[32];
  int tmp_array[32];
  int sixth_round[64];
  int seventh_round[64];
  for (i = 0; i < 56; i++)
    second_round[i] = key[Second_Tb[i] - 1];
  for (i = 0; i < 28; i++)
    low[i] = second_round[i];
  for (i = 0; i < 28; i++)
    high[i] = second_round[i + 28];
  for (i = 0; i < 16; i++)
    {
      //下面的是循环移位
      for (m = 0; m < LOOP_Table[i]; m++)
	{
	  swap = low[0];
	  for (n = 0; n < 27; ++n)
	    low[n] = low[n + 1];
	  low[27] = swap;
	  swap = high[0];
	  for (n = 0; n < 27; ++n)
	    high[n] = high[n + 1];
	  high[27] = swap;
	}
      for (n = 0; n < 28; n++)
	{
	  second_round[n] = low[n];
	  second_round[n + 28] = high[n];
	}
      for (m = 0; m < 48; m++)
	subkey[i][m] = second_round[Third_Tb[m] - 1];
    }
  for (i = 0; i < 64; i++)
    first_round[i] = ciphertext[First_Tb[i] - 1];
  for (i = 0; i < 32; i++)
    r[i] = first_round[i];
  for (i = 0; i < 32; i++)
    l[i] = first_round[i + 32];
  for (i = 0; i < 16; i++)
    {
      for (m = 0; m < 48; m++)
	third_round[m] = l[Forth_Tb[m] - 1];
      for (m = 0; m < 48; m++)
	third_round[m] = third_round[m] ^ subkey[16 - 1 - i][m];
      for (m = 0; m < 8; m++)
	{
	  tmp =
	    S_Box[m][third_round[6 * m + 0] * 2 +
		     third_round[6 * m + 5]][third_round[6 * m + 1] * 8 +
					     third_round[6 * m + 2] * 4 +
					     third_round[6 * m + 3] * 2 +
					     third_round[6 * m + 4]];
	  fourth_round[4 * m + 3] = tmp % 2;
	  tmp = tmp / 2;
	  fourth_round[4 * m + 2] = tmp % 2;
	  tmp = tmp / 2;
	  fourth_round[4 * m + 1] = tmp % 2;
	  tmp = tmp / 2;
	  fourth_round[4 * m + 0] = tmp % 2;
	}
      for (m = 0; m < 32; m++)
	fifth_round[m] = fourth_round[Fifth_Tb[m] - 1];
      for (m = 0; m < 32; m++)
	{
	  tmp_array[m] = r[m];
	  r[m] = l[m];
	  l[m] = fifth_round[m] ^ tmp_array[m];
	}
    }

//注意:这里的低32位和高32位与开始时相反
  for (i = 0; i < 32; i++)
    {
      sixth_round[i] = l[i];
      sixth_round[i + 32] = r[i];
    }
  for (i = 0; i < 64; i++)
    {
      seventh_round[i] = sixth_round[Last_Tb[i] - 1];
    }
  printf ("the Decrypted data :\n\t");
  for (i = 0; i < 64; i++)
    {
      printf ("%d ", seventh_round[i]);
      if ((i + 1) % 8 == 0 && i != 63)
	printf ("\n\t");
      if (i == 63)
	printf ("\n");
    }
}

int
main ()
{
  int i, plain[64], ciphertext[64], key[64];
  for (i = 0; i < 64; i++)
    plain[i] = 0;
  plain[2] = 1;
  plain[3] = 1;
  plain[7] = 1;
  plain[10] = 1;
  plain[11] = 1;
  plain[15] = 1;
  plain[18] = 1;
  plain[19] = 1;
  plain[23] = 1;
  plain[26] = 1;
  plain[27] = 1;
  plain[31] = 1;
  plain[34] = 1;
  plain[35] = 1;
  plain[39] = 1;
  plain[42] = 1;
  plain[43] = 1;
  plain[47] = 1;
  plain[50] = 1;
  plain[51] = 1;
  plain[55] = 1;
  plain[58] = 1;
  plain[59] = 1;
  plain[63] = 1;
  for (i = 0; i < 64; i++)
    ciphertext[i] = 0;
  ciphertext[1] = 1;
  ciphertext[2] = 1;
  ciphertext[5] = 1;
  ciphertext[7] = 1;
  ciphertext[9] = 1;
  ciphertext[11] = 1;
  ciphertext[12] = 1;
  ciphertext[13] = 1;
  ciphertext[14] = 1;
  ciphertext[16] = 1;
  ciphertext[18] = 1;
  ciphertext[21] = 1;
  ciphertext[22] = 1;
  ciphertext[26] = 1;
  ciphertext[28] = 1;
  ciphertext[32] = 1;
  ciphertext[33] = 1;
  ciphertext[36] = 1;
  ciphertext[37] = 1;
  ciphertext[38] = 1;
  ciphertext[39] = 1;
  ciphertext[41] = 1;
  ciphertext[42] = 1;
  ciphertext[46] = 1;
  ciphertext[49] = 1;
  ciphertext[51] = 1;
  ciphertext[52] = 1;
  ciphertext[57] = 1;
  ciphertext[59] = 1;
  ciphertext[60] = 1;
  ciphertext[61] = 1;
  ciphertext[62] = 1;
  ciphertext[63] = 1;
  for (i = 0; i < 64; i++)
    key[i] = 0;
  key[2] = 1;
  key[3] = 1;
  key[7] = 1;
  key[10] = 1;
  key[11] = 1;
  key[15] = 1;
  key[18] = 1;
  key[19] = 1;
  key[23] = 1;
  key[26] = 1;
  key[27] = 1;
  key[31] = 1;
  key[34] = 1;
  key[35] = 1;
  key[39] = 1;
  key[42] = 1;
  key[43] = 1;
  key[47] = 1;
  key[50] = 1;
  key[51] = 1;
  key[55] = 1;
  key[58] = 1;
  key[59] = 1;
  key[63] = 1;
  crypt (plain, key);
  decrypt (ciphertext, key);
}
