#include "stdafx.h"
#include "glyph_cache.h"
#include "ft.h"
#include "lock.h"

bool gdimm_glyph_cache::cache_trait::operator==(const gdimm_glyph_cache::cache_trait &trait) const
{
	return (memcmp(this, &trait, sizeof(gdimm_glyph_cache::cache_trait)) == 0);
}

gdimm_glyph_cache::cache_node::cache_node()
:
ref_count(0)
{
}

gdimm_glyph_cache::gdimm_glyph_cache()
:
_cached_bytes(0)
{
}

void gdimm_glyph_cache::add_ref(const void *cache_node_ptr)
{
	((cache_node *)cache_node_ptr)->ref_count += 1;
}

void gdimm_glyph_cache::release(const void *cache_node_ptr)
{
	((cache_node *)cache_node_ptr)->ref_count -= 1;
}

void gdimm_glyph_cache::erase_glyph_cache(const cache_map &glyph_cache)
{
	for (cache_map::const_iterator iter = glyph_cache.begin(); iter != glyph_cache.end(); iter++)
	{
		const FT_BitmapGlyph glyph = iter->second;

		if (glyph->bitmap.buffer != NULL)
			_cached_bytes -= _msize(glyph->bitmap.buffer);

		FT_Done_Glyph((FT_Glyph) glyph);
	}
}

const FT_BitmapGlyph gdimm_glyph_cache::lookup_glyph(const cache_trait &trait, FT_UInt glyph_index, const void *&cache_node_ptr)
{
	gdimm_lock lock(LOCK_GLYPH_CACHE);

	for (list<cache_node>::iterator font_iter = _cache.begin(); font_iter != _cache.end(); font_iter++)
	{
		if (trait == font_iter->trait)
		{
			// move the accessed font node to the beginning of the list
			_cache.splice(_cache.begin(), _cache, font_iter);

			cache_map::const_iterator glyph_iter = font_iter->glyph_cache.find(glyph_index);
			if (glyph_iter == font_iter->glyph_cache.end())
				return NULL;
			else
			{
				cache_node_ptr = &(*font_iter);
				return glyph_iter->second;
			}
		}
	}

	return NULL;
}

void gdimm_glyph_cache::add_glyph(const cache_trait &trait, FT_UInt glyph_index, const FT_BitmapGlyph glyph, const void *&cache_node_ptr)
{
	gdimm_lock lock(LOCK_GLYPH_CACHE);

	// cached bytes exceeds limit, begin to reclaim least recently used glyphs
	if ((_cached_bytes >= ft_cache_max_bytes) && !_cache.empty())
	{
		/*
		starting from the end of the cache list, erase the nodes that are not currently used by any renderer
		until the cached bytes fall into the limit again, or the whole list has been traversed
		eventually, the cached bytes may still larger than the limit
		*/

		// least recently used nodes are at the end
		list<cache_node>::const_iterator iter = _cache.end();
		bool at_begin = false;

		// if the iterator is pointing to the begin, no more node can be traversed, break
		for (iter--; (_cached_bytes >= ft_cache_max_bytes) && (!at_begin); )
		{
			at_begin = (iter == _cache.begin());

			// erase only if the cache node is not used by any renderer
			if (iter->ref_count <= 0)
			{
				assert(iter->ref_count == 0);

				erase_glyph_cache(iter->glyph_cache);
				const list<cache_node>::const_iterator erase_iter = iter;

				// iterator cannot be decremented if pointing at the begin
				if (!at_begin)
					iter--;

				_cache.erase(erase_iter);
			}
			else
			{
				if (!at_begin)
					iter--;
			}
		}
	}

	for (list<cache_node>::iterator iter = _cache.begin(); iter != _cache.end(); iter++)
	{
		if (trait == iter->trait)
		{
			// if the font is accessed previously, add the glyph to it
			iter->glyph_cache.insert(pair<FT_UInt, const FT_BitmapGlyph>(glyph_index, glyph));
			// return cache node usage handle
			cache_node_ptr = &(*iter);

			// _msize is non-standard function to get size of an allocated memory pointer
			if (glyph->bitmap.buffer != NULL)
				_cached_bytes += _msize(glyph->bitmap.buffer);

			return;
		}
	}

	// the font is not accessed previously, add new cache node at the beginning
	cache_node new_node;
	new_node.trait = trait;
	new_node.glyph_cache.insert(pair<FT_UInt, const FT_BitmapGlyph>(glyph_index, glyph));

	_cache.push_front(new_node);
	cache_node_ptr = &(_cache.front());

	if (glyph->bitmap.buffer != NULL)
		_cached_bytes += _msize(glyph->bitmap.buffer);
}

void gdimm_glyph_cache::clear()
{
	for (list<cache_node>::const_iterator iter = _cache.begin(); iter != _cache.end(); iter++)
		erase_glyph_cache(iter->glyph_cache);
}