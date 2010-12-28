//-----------------------------------------------------------------------------
// MurmurHash2, 64-bit versions, by Austin Appleby

// The same caveats as 32-bit MurmurHash2 apply here - beware of alignment 
// and endian-ness issues if used across multiple platforms.

#pragma once

// 64-bit hash for 64-bit platforms

unsigned __int64 MurmurHash64A ( const void * key, int len, unsigned int seed );


// 64-bit hash for 32-bit platforms

unsigned __int64 MurmurHash64B ( const void * key, int len, unsigned int seed );
