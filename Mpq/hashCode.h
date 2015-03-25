#ifndef __hashCode_h_
#define __hashCode_h_

#define	BLIZZARD_HASH_OFFSET	0
#define BLIZZARD_HASH_A			1
#define BLIZZARD_HASH_B			2

// RS Hash Function   
unsigned long RSHash(const char *str);

// BKDR Hash Function   
unsigned long BKDRHash(const char *str);

// Blizzard Hash Function
unsigned long BlizzardHash(const char* str,unsigned long var);

// Blizzard Hash Function
unsigned long BlizzardHash(const char* str);

// Blizzard Hash Function
unsigned long BlizzardHashA(const char* str);

// Blizzard Hash Function
unsigned long BlizzardHashB(const char* str);

#endif