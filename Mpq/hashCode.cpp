#include "hashCode.h"

// RS Hash Function   
unsigned long RSHash(const char *str)  
{  
	unsigned long b = 378551;  
	unsigned long a = 63689;  
	unsigned long hash = 0;  

	while (*str)  
	{  
		hash = hash * a + (*str++);  
		a *= b;  
	}  

	return (hash & 0x3FFF);
}

// BKDR Hash Function   
unsigned long BKDRHash(const char *str)  
{  
	unsigned long seed = 131; // 31 131 1313 13131 131313 etc..   
	unsigned long hash = 0;  

	while (*str)  
	{  
		hash = hash * seed + (*str++);
	}  

	return (hash & 0x7FFFFFFF);  
}

// Blizzard Hash Function
unsigned long BlizzardHash(const char* str,unsigned long var)
{
	static bool isHashTabled = false;
	static unsigned long ulHashTable[0x500];
	if ( !isHashTabled )
	{
		unsigned long seed = 0x00100001;
		for(unsigned long index1 = 0; index1 < 0x100; index1++ )
		{
			for(unsigned long index2 = index1,i = 0; i < 5; i++,index2 += 0x100 )
			{
				unsigned long temp1,temp2;
				seed = (seed * 125 + 3) % 0x2AAAAB;
				temp1 = (seed & 0xFFFF) << 0x10;    

				seed = (seed * 125 + 3) % 0x2AAAAB;
				temp2 = (seed & 0xFFFF);    

				ulHashTable[index2] = ( temp1 | temp2 );
			}
		}

		isHashTabled = true;
	}

	unsigned char key = 0;    
	unsigned long seed1 = 0x7FED7FED;
	unsigned long seed2 = 0xEEEEEEEE;

	while(*str != 0)    
	{     
		key = *str++;
		seed1 = ulHashTable[(var << 8) + key] ^ (seed1 + seed2);
		seed2 = key + seed1 + seed2 + (seed2 << 5) + 3;     
	}    

	return seed1;
}

// Blizzard Hash Function
unsigned long BlizzardHash(const char* str)
{
	return BlizzardHash(str,BLIZZARD_HASH_OFFSET);
}

// Blizzard Hash Function
unsigned long BlizzardHashA(const char* str)
{
	return BlizzardHash(str,BLIZZARD_HASH_A);
}

// Blizzard Hash Function
unsigned long BlizzardHashB(const char* str)
{
	return BlizzardHash(str,BLIZZARD_HASH_B); 
}
