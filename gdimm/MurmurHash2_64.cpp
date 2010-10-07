//-----------------------------------------------------------------------------
// MurmurHash2, 64-bit versions, by Austin Appleby

// The same caveats as 32-bit MurmurHash2 apply here - beware of alignment 
// and endian-ness issues if used across multiple platforms.

#include "MurmurHash2_64.h"

// 64-bit hash for 64-bit platforms

unsigned __int64 MurmurHash64A ( const void * key, int len, unsigned int seed )
{
	const unsigned __int64 m = 0xc6a4a7935bd1e995;
	const int r = 47;

	unsigned __int64 h = seed ^ (len * m);

	const unsigned __int64 * data = (const unsigned __int64 *)key;
	const unsigned __int64 * end = data + (len/8);

	while(data != end)
	{
		unsigned __int64 k = *data++;

		k *= m; 
		k ^= k >> r; 
		k *= m; 
		
		h ^= k;
		h *= m; 
	}

	const unsigned char * data2 = (const unsigned char*)data;

	switch(len & 7)
	{
	case 7: h ^= unsigned __int64(data2[6]) << 48;
	case 6: h ^= unsigned __int64(data2[5]) << 40;
	case 5: h ^= unsigned __int64(data2[4]) << 32;
	case 4: h ^= unsigned __int64(data2[3]) << 24;
	case 3: h ^= unsigned __int64(data2[2]) << 16;
	case 2: h ^= unsigned __int64(data2[1]) << 8;
	case 1: h ^= unsigned __int64(data2[0]);
	        h *= m;
	};
 
	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
} 


// 64-bit hash for 32-bit platforms

unsigned __int64 MurmurHash64B ( const void * key, int len, unsigned int seed )
{
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	unsigned int h1 = seed ^ len;
	unsigned int h2 = 0;

	const unsigned int * data = (const unsigned int *)key;

	while(len >= 8)
	{
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;

		unsigned int k2 = *data++;
		k2 *= m; k2 ^= k2 >> r; k2 *= m;
		h2 *= m; h2 ^= k2;
		len -= 4;
	}

	if(len >= 4)
	{
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;
	}

	switch(len)
	{
	case 3: h2 ^= ((unsigned char*)data)[2] << 16;
	case 2: h2 ^= ((unsigned char*)data)[1] << 8;
	case 1: h2 ^= ((unsigned char*)data)[0];
			h2 *= m;
	};

	h1 ^= h2 >> 18; h1 *= m;
	h2 ^= h1 >> 22; h2 *= m;
	h1 ^= h2 >> 17; h1 *= m;
	h2 ^= h1 >> 19; h2 *= m;

	unsigned __int64 h = h1;

	h = (h << 32) | h2;

	return h;
} 
