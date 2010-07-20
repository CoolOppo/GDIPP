//-----------------------------------------------------------------------------
// MurmurHash2, 64-bit versions, by Austin Appleby

// The same caveats as 32-bit MurmurHash2 apply here - beware of alignment 
// and endian-ness issues if used across multiple platforms.

typedef unsigned __int64 uint64_t;

// 64-bit hash for 64-bit platforms

uint64_t MurmurHash64A ( const void * key, int len, unsigned int seed );


// 64-bit hash for 32-bit platforms

uint64_t MurmurHash64B ( const void * key, int len, unsigned int seed );
