#ifndef __MD5_H_
#define __MD5_H_

#include "common.h"
#include <string>
#include <fstream>

using namespace std;

/* Constants for MD5Transform routine. */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

/* 
 * F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* 
 * ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* 
 * FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
 * Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
	(a) += F ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
	(a) += G ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
	(a) += H ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
	(a) += I ((b), (c), (d)) + (x) + ac; \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}

class MD5
{
public:
	MD5();
	~MD5();

	void update(const void* input, size_t length);
	void update(const string& str);
	void update(FILE* fp);
	void update(ifstream& in);
	void update(const unsigned char* input, size_t length);
	
	void begin();
	void end();

	const uchar* result();

	string to_string();

private:
	void transform(const unsigned char block[64]);
	void encode(const uint32* input, unsigned char* output, size_t length);
	void decode(const unsigned char* input, uint32* output, size_t length);

private:
	uint32 _state[4];	/* state (ABCD) */
	uint32 _count[2];	/* number of bits, modulo 2^64 (low-order word first) */
	unsigned char _buffer[64];	/* input buffer */
	unsigned char _digest[16];	/* message digest */
	bool _finished;		/* calculate finished ? */

public:
	static const unsigned char	PADDING[64];	/* padding for calculate */
	static const char	HEX_TABLE[16];
	static const int	BUFFER_SIZE = 1024;

	static string binaryToHexString(const unsigned char* input, size_t length);
};

#endif //__MD5_H_
