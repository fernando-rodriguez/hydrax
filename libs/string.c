/*
 *
 */
#include "string.h"

static char buff[11];

// Compare two strings. Should return -1 if 
// str1 < str2, 0 if they are equal or 1 otherwise.
int strcmp(const char *str1, const char *str2)
{
      int i = 0;
      int failed = 0;
      while(str1[i] != '\0' && str2[i] != '\0')
      {
          if(str1[i] != str2[i])
          {
              failed = 1;
              break;
          }
          i++;
      }
      // why did the loop exit?
      if( (str1[i] == '\0' && str2[i] != '\0') || (str1[i] != '\0' && str2[i] == '\0') )
          failed = 1;

      return failed;
}

unsigned int strlen(const char *s)
{
	unsigned int len = 0;
	while (*s++ != NULL)
		len++;
	return len;
}

// Copy the NULL-terminated string src into dest, and
// return dest.
char *strcpy(char *dest, const char *src)
{
	while (*src != NULL)
		*dest++ = *src++;
	*dest = NULL;
	return dest;
}

// Concatenate the NULL-terminated string src onto
// the end of dest, and return dest.
char *strcat(char *dest, const char *src)
{
    while (*dest != 0)
    {
        /**dest = *dest++;*/
	dest++;
    }

    do
    {
        *dest++ = *src++;
    }
    while (*src != 0);
    return dest;
}

#if 0
char *itoa(int val)
{
	char neg;
	char *b = buff;
	int i, l;
	unsigned int digit = 1000000000;

	*buff = 0;
	neg = (val < 0);

	if (val == 0)
		return buff;

	for (l = 0; l < 10; l++)
	{
		i = val / digit;
		*b++ = '0' + (char) i;	
		val -= i * digit;
		digit /= 10;		
	}

	*b = 0;
	b = buff;
	i = 0;

	while (buff[i] == '0')
		i++;

	while (i <= 11)
		*b++ = buff[i++];

	return buff;	
}

char *itox(unsigned int val)
{
        //char neg;
        char *b = buff;
        int i, l;
        unsigned int digit = 0x10000000;

        *buff = 0;
        //neg = (val < 0);

	if (val == 0)
	{
		buff[0] = '0';
		buff[1] = NULL;
		return buff;
	}

        for (l = 0; l < 8; l++)
        {
		i = val / digit;
		*b++ = '0' + (char)((i >= 0xA) ? i + 7 : i);
		val -= i * digit;
		digit /= 16;
	}

	*b = 0;
	b = buff;
	i = 0;

	//while (buff[i] == '0')
	//	i++;

        //while (i <= 9)
	//	*b++ = buff[i++];

	return buff;

}
#endif
