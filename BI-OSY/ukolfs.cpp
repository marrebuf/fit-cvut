/* This implementation of a filesystem is loosely based on that of BeOS/BFS,
 * and intentionally overengineered, out of curiosity.  Bitmaps, block cache
 * (with sequential reordering), preallocation, extents, indirect blocks,
 * all of that.  Some of the features are in fact going to make it perform
 * worse as it is about to be layered on top of another filesystem and another
 * cache.
 *
 * The filesystem has got the following structure on disk:
 *   SuperBlock Bitmap { IndirectBlock | data } ...
 *
 * Inspired by: GIAMPAOLO, Dominic. Practical File System Design with the
 *              Be File System. 1999. ISBN 1-55860-497-9.
 */

#ifndef __PROGTEST__
#include "common_fs.h"
#include <assert.h>

#include <stdarg.h>
static int DEBUG (const char *format, ...)
{
	va_list ap;
	int result;

	va_start (ap, format);
	result = vfprintf (stderr, format, ap);
	va_end (ap);
	return result;
}
#else /* __PROGTEST__ */
#define assert(cond)
static inline void DEBUG (...) {}
#endif /* __PROGTEST__ */


typedef struct TFile TFile;
typedef struct TBlkDev TBlkDev;

/** Maximum count of i-nodes at one time. */
#define INODES_MAX (DIR_ENTRIES_MAX + OPEN_FILES_MAX)

/** Logical block size, relative to SECTOR_SIZE. */
#define BLK_SIZE 4
/** The real size of a logical block in bytes. */
#define BLK_SIZE_REAL (BLK_SIZE * SECTOR_SIZE)
/** Invalid value for a block ID. */
#define BLK_INVALID 0

/** Get the value in multiplies of BLK_SIZE_REAL. */
#define BLK_BLK_SIZE(n)  ((n + BLK_SIZE_REAL - 1) / BLK_SIZE_REAL)
/** Get the size of a type in blocks. */
#define BLK_BLK_SIZEOF(T)  BLK_BLK_SIZE (sizeof (T))
/** Get the size of a type in blocks, in clusters. */
#define BLK_SCT_SIZEOF(T)  (BLK_BLK_SIZEOF(T) * BLK_SIZE)

/* ===== Disk cache ========================================================= */

#define DC_LIST_SIZE   (1 << 8)
#define DC_HASH(id)    ((id) % DC_HMAP_SIZE)
#define DC_HMAP_SIZE   (DC_LIST_SIZE >> 2)
#define DC_READ_UNIT    8
#define DC_FLUSH_LEN   (DC_LIST_SIZE >> 2)

/** A single cache entry. */
typedef struct DCEntry DCEntry;
struct DCEntry
{
	unsigned blk_id;            //! Assigned block ID.
	DCEntry *hmap_next;         //! The next entry in the hashmap sublist
	                            //! or the next entry in the free list.
	DCEntry *next, *prev;       //! Less and more recently used entries.

	unsigned dirty : 1;                 //! Data have been modified.
	unsigned char data[BLK_SIZE_REAL];  //! Cached block data.
};

/** Disk cache object. */
typedef struct
{
	DCEntry *hmap[DC_HMAP_SIZE];        //! Hashmap for faster searches.
	DCEntry *mru, *lru;                 //! Most and least recently used.

	/*          ____       ____       ____
	 *  mru >--|   n|-->--|   n|-->--|   n|--|
	 *      |--|p___|--<--|p___|--<--|p___|--< lru
	 */

	DCEntry entries[DC_LIST_SIZE];      //! Preallocated cache entries.
	DCEntry *free;                      //! First free entry to use.
}
DCache;

static void       dcache_init           (void);
static DCEntry *  dcache_get_block      (unsigned blk_id, int for_overwriting);
static int        dcache_partial_flush  (void);
static int        dcache_flush          (void);

static void       dcache_unlink_entry   (DCEntry *pentry);
static void       dcache_trash_entry    (DCEntry *pentry);
static int        dcache_entry_cmp      (const void *i1, const void *i2);

/* ===== Filesystem ========================================================= */

/*  Super block magic values. */
#define IDENT_MAGIC 0x5346594D  //! FS identifier. ("MYFS" in LE.)
#define CLEAN_MAGIC 0x1EABC150  //! The partition is clean.
#define DIRTY_MAGIC 0xEDA5CEDE  //! The partition is possibly inconsistent.

/** Identifies a file and the disk blocks associated with it. */
typedef struct
{
	unsigned size;              //! Size of the file.
	unsigned indir_id;          //! Indirect block ID.
	// TODO: Add a few (maybe just one) direct data block ID's.
}
INode;

/** Defines a contiguous run of disk blocks. */
typedef struct
{
	unsigned blk_id;            //! ID of the first block.
	unsigned short len;         //! Count of successive blocks.
}
Extent;

/** Points to the actualy disk blocks holding data in a file. */
typedef struct
{
	unsigned next_id;           //! The next indirect node in order.
	unsigned short len;         //! Count of extents defined by the block.
	// Extent blks[];           //  A bunch of extents, filling the block.
}
IndirBlk;

/** Directory entry. */
typedef struct
{
	char name[FILENAME_LEN_MAX + 1];    //! Filename.
	unsigned short inode;               //! The corresponding i-node.
}
DirEntry;

/** Partition superblock, defining anything that follows after it. */
typedef struct
{
	unsigned ident;             //! Human-readable FS identifier.
	unsigned state;             //! Defines the state of the FS.

	unsigned extents_in_indir_blk;      //! Count of Extent entries stored
	                                    //! within an indirect block.
	unsigned bmap_size;         //! Size of the block bitmap in blocks.

	unsigned short root_dir_len;        //! Count of files in root.
	DirEntry root_dir[DIR_ENTRIES_MAX]; //! Root directory.
	INode inodes[INODES_MAX];           //! i-node array.
}
SuperBlk;

/** In-memory copy of a superblock as stored on the disk. */
typedef union
{
	SuperBlk sb;                //! The superblock structure itself.

	/** Padding to align on whole disk blocks (simplifies disk operations). */
	char overlay[BLK_BLK_SIZEOF (SuperBlk) * BLK_SIZE_REAL];
}
SBPadded;

/** File descriptor. */
typedef struct
{
	unsigned open    : 1;       //! Am I open right now?
	unsigned wr_mode : 1;       //! Am I supposed to be written to?

	unsigned short inode;       //! The corresponding i-node.
	unsigned offset;            //! The current offset within the file.

	// XXX: It could be possible to replace these two with an Extent.
	//      Then the code in FileRead/Write simplifies a bit.
	unsigned blk_id;            //! Current block ID in extent.
	unsigned short ext_rem;     //! Count of remaining blocks in extent.
}
FD;

/** Helper structure to iterate through extents. */
typedef struct
{
	unsigned *blk_id;           //! The current indirect block.
	DCEntry *pentry;            //! Cache entry for the current block,
	                            //! or the previous one if we're at the end.
	unsigned i_ext;             //! Extent iterator.
	unsigned offset;            //! Where we want to get,
	                            //! offset into the extent on return.
	Extent *pext;               //! Points to the actual extent on return.
}
GECtx;

static int      fs_find_entry    (const char *filename, DirEntry **entry);
static int      fs_add_entry     (const char *filename, unsigned short inode);
static void     fs_remove_entry  (DirEntry *pentry);

static int      fs_get_extent_try     (GECtx *i, Extent *extent,
                                       unsigned short inode, unsigned offset);
static int      fs_get_extent_finish  (GECtx *i, Extent *extent);

static inline
INode *         fs_find_inode    (const char *filename, unsigned short *inode);
static void     fs_unref_inode   (unsigned short inode);
static void     fs_truncate      (unsigned short inode);

/* ===== Bitmap ============================================================= */

#define BMAP_PREALLOC  32       //! How many blocks to try to preallocate.
#define BMAP_TRIES     4        //! How many times to try to preallocate the
                                //! whole count of blocks before giving up.

#define BMAP_TYPE      unsigned
#define BMAP_UNIT     (sizeof (BMAP_TYPE) * 8)

/** Defines a bitmap to use to search for free blocks. */
typedef struct
{
	BMAP_TYPE *bits;            //! The actual bitmap data,
	                            //! beginning with the least significant bit.
	unsigned size;              //! Size of the bitmap in BMAP_UNIT's.
	unsigned free_iter;         //! Free blocks iterator (max. `size').
	unsigned full : 1;          //! Don't search the whole bitmap once
	                            //! we detect there are no free blocks.
}
BMap;

static int       bmap_find_free_unit    (unsigned *unit);
static unsigned  bmap_alloc             (void);
static unsigned  bmap_find_free_extent  (unsigned short *len);
static unsigned  bmap_alloc_prealloc    (unsigned short *len);
static void      bmap_release           (unsigned blk_id, unsigned short len);

/* ===== Implementation ===================================================== */

static struct
{
	TBlkDev dev;                //! Disk device interface.
	unsigned mounted : 1;       //! Is anything mounted right now?

	DCache cache;               //! Disk cache.
	BMap bmap;                  //! Block bitmap.

	SBPadded super_blk;         //! In-memory copy of the superblock.
	FD fds[OPEN_FILES_MAX];     //! File descriptor array.

	/** How many times an i-node is referenced by either directories or FD's.
	 *  This is effectively removing the need for vnodes. */
	unsigned short inode_ref_cnt[INODES_MAX];
}
g_ctx;

/* ----- Bitmap ------------------------------------------------------------- */
/** Try to find a bitmap block with one or more free disk blocks. */
static int
bmap_find_free_unit (unsigned *unit_id)
{
	BMap *bm = &g_ctx.bmap;
	assert (*unit_id < bm->size);

	if (bm->full)
		return 0;

	unsigned i;
	for (i = *unit_id; i < bm->size; i++)
		if (~bm->bits[i])
			goto bmap_ffu_found;
	for (i = 0; i < bm->free_iter; i++)
		if (~bm->bits[i])
			goto bmap_ffu_found;

	bm->full = 1;
	DEBUG ("II The bitmap is now full\n");
	return 0;

bmap_ffu_found:
	*unit_id = i;
	return 1;
}

/** Allocate a single disk block in the bitmap. */
static unsigned
bmap_alloc (void)
{
	BMap *bm = &g_ctx.bmap;

	unsigned unit_id = bm->free_iter;
	if (!bmap_find_free_unit (&unit_id))
		return BLK_INVALID;

	BMAP_TYPE unit_bits = bm->bits[unit_id];
	assert (~unit_bits != 0);

	/* Compute bit position. */
	unsigned bit, blk_id = unit_id * BMAP_UNIT;
	for (bit = 1; unit_bits & bit; bit <<= 1)
		blk_id++;

	bm->bits[unit_id] |= bit;
	bm->free_iter = unit_id;
	return blk_id;
}

/* Find a sequence of free blocks in the bitmap. */
static unsigned
bmap_find_free_extent (unsigned short *len)
{
	BMap *bm = &g_ctx.bmap;
	Extent result = {BLK_INVALID, 0};

	// FIXME: This is a performance disaster
	//        when the disk is nearly full.
	unsigned unit_id = bm->free_iter;
	for (unsigned tries = 0; tries < BMAP_TRIES; tries++)
	{
		if (!bmap_find_free_unit (&unit_id))
			break;

		BMAP_TYPE unit_bits = bm->bits[unit_id];
		assert (~unit_bits != 0);

		Extent e = {unit_id * BMAP_UNIT, 0};
		unsigned bit = 1;

		/* Get as much as possible from this unit. */
bmap_ffe_find_first_block:
		for (;   unit_bits & bit;         bit <<= 1)
			e.blk_id++;
bmap_ffe_count_blocks:
		for (; !(unit_bits & bit) && bit; bit <<= 1)
			if (++e.len == BMAP_PREALLOC)
			{
				result = e;
				goto bmap_ffe_return;
			}

		if (e.len > result.len)
			result = e;

		if (bit)
		{
			/* If there are some bits remaining, try again. */
			if (~unit_bits & ~(bit - 1))
			{
				e.blk_id += e.len;
				e.len = 0;
				goto bmap_ffe_find_first_block;
			}
			unit_id = (unit_id + 1) % bm->size;
		}
		else if (++unit_id < bm->size)
		{
			/* Otherwise continue in the next unit. */
			unit_bits = bm->bits[unit_id];
			if (~unit_bits)
			{
				bit = 1;
				goto bmap_ffe_count_blocks;
			}
			unit_id = (unit_id + 1) % bm->size;
		}
		else
			unit_id = 0;
	}

bmap_ffe_return:
	bm->free_iter = unit_id;
	*len = result.len;
	return result.blk_id;
}

/** Attempt to allocate a run of contiguous disk blocks in the bitmap. */
static unsigned
bmap_alloc_prealloc (unsigned short *len)
{
	BMap *bm = &g_ctx.bmap;

	unsigned short n_bits;
	unsigned blk_id = bmap_find_free_extent (&n_bits);
	*len = n_bits;

	if (blk_id == BLK_INVALID)
		return BLK_INVALID;

	unsigned unit_id = blk_id / BMAP_UNIT;
	unsigned bit_id  = blk_id % BMAP_UNIT;

	/* Because the block of bits may span multiple block units,
	 * the operation is split in three parts. */

	unsigned init_bits = n_bits;
	if (bit_id + init_bits > BMAP_UNIT)
		init_bits = BMAP_UNIT - bit_id;

	assert (sizeof (unsigned long) > sizeof (BMAP_TYPE));
	bm->bits[unit_id++] |= (((1UL << init_bits) - 1) << bit_id);
	n_bits -= init_bits;

	while (n_bits > BMAP_UNIT)
	{
		bm->bits[unit_id++] = ~0;
		n_bits -= BMAP_UNIT;
	}

	if (n_bits)
		bm->bits[unit_id] |= ((1UL << n_bits) - 1);

	return blk_id;
}

static void
bmap_release (unsigned blk_id, unsigned short len)
{
	BMap *bm = &g_ctx.bmap;
	assert ((blk_id + len + BMAP_UNIT - 1) / BMAP_UNIT <= bm->size);

	unsigned unit_id = blk_id / BMAP_UNIT;
	unsigned bit_id  = blk_id % BMAP_UNIT;

	if (bm->full)
	{
		bm->full = 0;
		bm->free_iter = unit_id;
	}

	/* Similar to bmap_alloc_prealloc(). */
	unsigned init_len = len;
	if (bit_id + init_len > BMAP_UNIT)
		init_len = BMAP_UNIT - bit_id;

	assert (sizeof (unsigned long) > sizeof (BMAP_TYPE));
	bm->bits[unit_id++] &= ~(((1UL << init_len) - 1) << bit_id);
	len -= init_len;

	while (len > BMAP_UNIT)
	{
		bm->bits[unit_id++] = 0;
		len -= BMAP_UNIT;
	}

	if (len)
		bm->bits[unit_id] &= ~((1UL << len) - 1);
}

/* ----- Disk block cache --------------------------------------------------- */
/** Initialize the internal structure. */
static void
dcache_init (void)
{
	DCache *pc = &g_ctx.cache;
	memset (pc, 0, sizeof *pc);

	/* Link entries in the free list. */
	for (unsigned i = DC_LIST_SIZE; i--; )
	{
		pc->entries[i].hmap_next = pc->free;
		pc->free = &pc->entries[i];
	}
}

/** Unlink a cache entry from the LRU list. */
static void
dcache_unlink_entry (DCEntry *pentry)
{
	DCache *pc = &g_ctx.cache;

	/* Remove from the double-linked list. */
	if (pentry->next)
		pentry->next->prev = pentry->prev;
	else
		pc->lru = pentry->prev;

	if (pentry->prev)
		pentry->prev->next = pentry->next;
	else
		pc->mru = pentry->next;
}

/** Remove an entry from the cache altogether. */
static void
dcache_trash_entry (DCEntry *pentry)
{
	DCache *pc = &g_ctx.cache;

	dcache_unlink_entry (pentry);

	/* Remove from the hashmap. */
	DCEntry **ppentry = &pc->hmap[DC_HASH (pentry->blk_id)];
	for (; *ppentry; ppentry = &(*ppentry)->hmap_next)
		if ((*ppentry) == pentry)
			break;
	assert (*ppentry != NULL);
	*ppentry = pentry->hmap_next;

	/* Insert into the free list. */
	pentry->hmap_next = pc->free;
	pc->free = pentry;
}


/** Get a cache entry for a block.  Returns NULL on failure. */
static DCEntry *
dcache_get_block (unsigned blk_id, int for_overwriting)
{
	DCache *pc = &g_ctx.cache;

	/* Search for the block in the hashmap. */
	unsigned blk_hash = DC_HASH (blk_id);
	DCEntry *pentry = pc->hmap[blk_hash];
	while (pentry)
	{
		if (pentry->blk_id == blk_id)
			break;
		pentry = pentry->hmap_next;
	}

	if (!pentry)
	{
		/* Cache miss, we have to read from disk. */
		// TODO: Readahead (DC_READ_UNIT); only if !for_overwriting.

		if (!pc->free)
			dcache_partial_flush ();
		assert (pc->free != NULL);

		/* Try to read the block from disk if requested. */
		pentry = pc->free;
		if (!for_overwriting)
			if (g_ctx.dev.m_Read (blk_id * BLK_SIZE,
				pentry->data, BLK_SIZE) != BLK_SIZE)
			{
				DEBUG ("EE Failed to read block %u\n", blk_id);
				return NULL;
			}

		/* We have succeeded, so allocate the cache entry. */
		pc->free = pentry->hmap_next;
		pentry->blk_id = blk_id;
		pentry->dirty = 0;

		/* Place it in the hashmap. */
		pentry->hmap_next = pc->hmap[blk_hash];
		pc->hmap[blk_hash] = pentry;

		/* Place it in the linked list. */
		pentry->prev = pentry->next = NULL;
		if (pc->mru)
		{
			pc->mru->prev = pentry;
			pentry->next = pc->mru;
			pc->mru = pentry;
		}
		else
			pc->mru = pc->lru = pentry;
	}
	else if (pentry != pc->mru)
	{
		/* Move the entry to MRU. */
		dcache_unlink_entry (pentry);
		pentry->next = pc->mru;
		pentry->prev = NULL;

		if (pc->mru)
			pc->mru->prev = pentry;
		else
			pc->lru = pentry;

		pc->mru = pentry;
	}

	return pentry;
}

/** Compare two entries by block ID. */
static int
dcache_entry_cmp (const void *i1, const void *i2)
{
	DCEntry *e1 = (DCEntry *) i1;
	DCEntry *e2 = (DCEntry *) i2;
	return (signed) e1->blk_id - (signed) e2->blk_id;
}

/** Make space for new cache entries. */
static int
dcache_partial_flush (void)
{
	DCache *pc = &g_ctx.cache;

	/* We should only call this function when the cache is full. */
	assert (pc->free == NULL);

	/* Put pointers on items into an array and sort them by block ID. */
	DCEntry *iter, *array[DC_LIST_SIZE];
	unsigned i, to_write = 0;
	for (iter = pc->lru, i = 0;
		iter && i < DC_FLUSH_LEN; iter = iter->prev, i++)
	{
		if (iter->dirty)
			array[to_write++] = iter;
		else
			dcache_trash_entry (iter);
	}

	qsort (array, to_write, sizeof *array, dcache_entry_cmp);

	/* Write blocks from array to disk. */
	int fail = 0;
	for (i = 0; i < to_write; i++)
	{
		if (g_ctx.dev.m_Write (array[i]->blk_id * BLK_SIZE,
			array[i]->data, BLK_SIZE) != BLK_SIZE)
			fail = 1;
		dcache_trash_entry (array[i]);
	}

	if (fail)
		DEBUG ("EE Failed to flush some blocks\n");
	return !fail;
}

/** Flush the entire cache to disk. */
static int
dcache_flush (void)
{
	DCache *pc = &g_ctx.cache;

	/* Put pointers on items into an array and sort them by block ID. */
	DCEntry *iter, *array[DC_LIST_SIZE];
	unsigned i, to_write = 0;
	for (iter = pc->lru; iter; iter = iter->prev)
		if (iter->dirty)
			array[to_write++] = iter;

	qsort (array, to_write, sizeof *array, dcache_entry_cmp);

	/* Write blocks from array to disk. */
	int fail = 0;
	for (i = 0; i < to_write; i++)
	{
		if (g_ctx.dev.m_Write (array[i]->blk_id * BLK_SIZE,
			array[i]->data, BLK_SIZE) != BLK_SIZE)
			fail = 1;
	}

	dcache_init ();
	if (fail)
		DEBUG ("EE Failed to flush some blocks\n");
	return !fail;
}

/* ----- File system internals ---------------------------------------------- */
/** Find a directory entry, or where it should be placed. */
static int
fs_find_entry (const char *filename, DirEntry **entry)
{
	SuperBlk *psb = &g_ctx.super_blk.sb;
	int min = 0, max = psb->root_dir_len - 1;

	while (max >= min)
	{
		unsigned mid = (min + max) / 2;
		int cmp = strcmp (psb->root_dir[mid].name, filename);

		if (cmp < 0)
			min = mid + 1;
		else if (cmp > 0)
			max = mid - 1;
		else
		{
			*entry = &psb->root_dir[mid];
			return 1;
		}
	}

	*entry = &psb->root_dir[min];
	return 0;
}

/** Add an entry to the root directory. */
static int
fs_add_entry (const char *filename, unsigned short inode)
{
	SuperBlk *psb = &g_ctx.super_blk.sb;
	if (psb->root_dir_len == DIR_ENTRIES_MAX)
	{
		DEBUG ("EE Root directory full\n");
		return 0;
	}

	DirEntry *pentry;
	if (fs_find_entry (filename, &pentry))
		return 0;

	/* Insert into the array. */
	memmove (pentry + 1, pentry,
		(psb->root_dir_len++ - (pentry - psb->root_dir)) * sizeof *pentry);
	strncpy (pentry->name, filename, FILENAME_LEN_MAX);
	pentry->inode = inode;

	return 1;
}

static void
fs_remove_entry (DirEntry *pentry)
{
	SuperBlk *psb = &g_ctx.super_blk.sb;
	assert (psb->root_dir_len != 0);

	memmove (pentry, pentry + 1,
		(--psb->root_dir_len - (pentry - psb->root_dir)) * sizeof *pentry);
}

static void
fs_truncate (unsigned short inode)
{
	/* Invalidate blk_id in associated FD's. */
	for (unsigned i = 0; i < OPEN_FILES_MAX; i++)
	{
		FD *pfd = &g_ctx.fds[i];
		if (pfd->open && pfd->inode == inode)
		{
			pfd->blk_id = BLK_INVALID;
			pfd->ext_rem = 0;
		}
	}

	/* Release all associated disk blocks. */
	SuperBlk *psb = &g_ctx.super_blk.sb;
	unsigned blk_id = psb->inodes[inode].indir_id;
	psb->inodes[inode].indir_id = BLK_INVALID;
	psb->inodes[inode].size = 0;

	while (blk_id != BLK_INVALID)
	{
		DCEntry  *pentry = dcache_get_block (blk_id, 0);
		assert (pentry != NULL);
		IndirBlk *pindir = (IndirBlk *) pentry->data;
		Extent   *pexts  = (Extent   *) (pindir + 1);

		assert (pindir->len <= psb->extents_in_indir_blk);
		for (unsigned i = 0; i < pindir->len; i++)
			bmap_release (pexts[i].blk_id, pexts[i].len);

		bmap_release (blk_id, 1);
		pentry->dirty = 0;
		blk_id = pindir->next_id;
	}
}

#define FSGE_RETURN \
	{ unsigned blk_off = i->offset / BLK_SIZE_REAL;      \
	  i->pext = &pexts[i->i_ext];                        \
	  extent->blk_id = pexts[i->i_ext].blk_id + blk_off; \
	  extent->len    = pexts[i->i_ext].len    - blk_off; \
	  return 1; }

/** Try to get the corresponding extent for an offset in a file.  If the
  * offset doesn't lie within the first block of an extent, the returned
  * structure is modified, so that this condition holds true.
  * The function returns 0 if no such extent has been allocated.
  * To allocate the required extents, call fs_get_extent_finish().
  */
static int
fs_get_extent_try (GECtx *i, Extent *extent,
	unsigned short inode, unsigned offset)
{
	SuperBlk *psb = &g_ctx.super_blk.sb;

	i->pentry = NULL;
	i->blk_id = &psb->inodes[inode].indir_id;
	i->offset = offset;
	while (*i->blk_id != BLK_INVALID)
	{
		i->pentry = dcache_get_block (*i->blk_id, 0);
		assert (i->pentry != NULL);
		IndirBlk *pindir = (IndirBlk *) i->pentry->data;
		Extent   *pexts  = (Extent   *) (pindir + 1);

		/* Go through all the extents. */
		assert (pindir->len <= psb->extents_in_indir_blk);
		for (i->i_ext = 0; i->i_ext < pindir->len; i->i_ext++)
		{
			unsigned extent_size = pexts[i->i_ext].len * BLK_SIZE_REAL;
			if (i->offset <  extent_size)
				FSGE_RETURN
			else
				i->offset -= extent_size;
		}

		if (i->i_ext < psb->extents_in_indir_blk)
		{
			/* All but the last indirect block must be full. */
			assert (pindir->next_id == BLK_INVALID);
			break;
		}

		i->blk_id = &pindir->next_id;
	}

	return 0;
}

/** Append further extents to get the offset within an extent. */
static int
fs_get_extent_finish (GECtx *i, Extent *extent)
{
	SuperBlk *psb = &g_ctx.super_blk.sb;
	IndirBlk *pindir;
	Extent   *pexts;

	/* We expect i->pentry to be still in the cache. */
	if (*i->blk_id != BLK_INVALID)
	{
		pindir = (IndirBlk *) i->pentry->data;
		pexts  = (Extent   *) (pindir + 1);
	}

	/* Allocate extents until the offset is reached. */
	while (1)
	{
		if (*i->blk_id == BLK_INVALID)
		{
			/* Allocate an indirect block. */
			if ((*i->blk_id = bmap_alloc ()) == BLK_INVALID)
				return 0;

			/* Set the previous indirect block dirty,
			 * as we've changed the next block pointer. */
			if (i->pentry != BLK_INVALID)
				i->pentry->dirty = 1;

			/* Fill out the header. */
			i->pentry = dcache_get_block (*i->blk_id, 1);
			assert (i->pentry != NULL);
			pindir = (IndirBlk *) i->pentry->data;
			pexts  = (Extent   *) (pindir + 1);

			pindir->next_id = BLK_INVALID;
			pindir->len = 0;
			i->pentry->dirty = 1;

			i->i_ext = 0;
		}

		/* Allocate an extent. */
		// TODO: Merge with the previous extent, if possible.
		// XXX: We should zeroize the new blocks but fuck that.
		if ((pexts[i->i_ext].blk_id = bmap_alloc_prealloc
			(&pexts[i->i_ext].len)) == BLK_INVALID)
			return 0;
		pindir->len++;
		i->pentry->dirty = 1;

		if (pindir->len == psb->extents_in_indir_blk)
			i->blk_id = &pindir->next_id;

		unsigned allocated = pexts[i->i_ext].len * BLK_SIZE_REAL;
		if (i->offset < allocated)
			break;

		i->offset -= allocated;
		i->i_ext++;
	}

	FSGE_RETURN
}

/** Search for an i-node by filename. */
static inline INode *
fs_find_inode (const char *filename, unsigned short *inode)
{
	DirEntry *pentry;
	if (!fs_find_entry (filename, &pentry))
		return NULL;
	assert (pentry->inode < INODES_MAX);
	if (inode)  *inode = pentry->inode;
	return &g_ctx.super_blk.sb.inodes[pentry->inode];
}

/** Unreference an i-node and possibly free all of its data. */
static void
fs_unref_inode (unsigned short inode)
{
	assert (g_ctx.inode_ref_cnt[inode] != 0);
	if (!--g_ctx.inode_ref_cnt[inode])
	{
		fs_truncate (inode);
		return;
	}

	/* Trim excess extents if not open for writing. */
	for (int fd = 0; fd < OPEN_FILES_MAX; fd++)
	{
		FD *pfd = &g_ctx.fds[fd];
		if (pfd->open && pfd->wr_mode && pfd->inode == inode)
			return;
	}

	SuperBlk *psb = &g_ctx.super_blk.sb;

	GECtx ctx;
	Extent e;
	if (!fs_get_extent_try (&ctx, &e, inode, psb->inodes[inode].size))
		return;

	/* Skip the last block with data. */
	if (ctx.offset)
	{
		e.blk_id++;
		e.len--;
	}

	if (e.len)
	{
		bmap_release (e.blk_id, e.len);
		ctx.pext->len -= e.len;
	}
}

/* ----- Public interface --------------------------------------------------- */
int
FsCreate (TBlkDev *dev)
{
	if (g_ctx.mounted || !dev)
		return 0;

	SBPadded super_blk;

	/* Size of the block bitmap in, again, blocks. */
	unsigned bmap_size = BLK_BLK_SIZE ((dev->m_Sectors / BLK_SIZE + 7) / 8);

	/* Initialize the superblock. */
	memset (super_blk.overlay, 0, sizeof super_blk.overlay);
	super_blk.sb.ident = IDENT_MAGIC;
	super_blk.sb.state = CLEAN_MAGIC;
	super_blk.sb.extents_in_indir_blk =
		(BLK_SIZE_REAL - sizeof (IndirBlk)) / sizeof (Extent);
	super_blk.sb.bmap_size = bmap_size;

	if (dev->m_Write (0, super_blk.overlay, BLK_SCT_SIZEOF (SuperBlk))
		!= BLK_SCT_SIZEOF (SuperBlk))
	{
		DEBUG ("EE Failed to write the superblock\n");
		return 0;
	}

	DEBUG ("-- Initializing filesystem\n");
	DEBUG ("II %u sectors available (%u blk)\n",
		dev->m_Sectors, dev->m_Sectors / BLK_SIZE);
	DEBUG ("II Superblock size: %u bytes (%u blk)\n",
		sizeof (SuperBlk), BLK_BLK_SIZEOF (SuperBlk));
	DEBUG ("II Bitmap size: %u blk\n", bmap_size);

	/* Initialize the block bitmap. */
	assert (BLK_SIZE_REAL % sizeof (BMAP_TYPE) == 0);
	BMAP_TYPE *bmap = (BMAP_TYPE *) calloc (bmap_size, BLK_SIZE_REAL);

	/* Ban the superblock, the bitmap itself, and the padding. */
	unsigned unusable, i;

	unusable = BLK_BLK_SIZEOF (SuperBlk) + bmap_size;
	DEBUG ("-- Banning %u blk from start\n", unusable);
	assert (unusable < (unsigned) dev->m_Sectors / BLK_SIZE);
	for (i = 0; unusable > BMAP_UNIT; i++)
	{
		bmap[i] = ~0U;
		unusable -= BMAP_UNIT;
	}
	bmap[i] = (1U << unusable) - 1;

	unusable = (bmap_size * BLK_SIZE_REAL * 8) - (dev->m_Sectors / BLK_SIZE);
	DEBUG ("-- Banning %u blk from end\n", unusable);
	for (i = bmap_size * BLK_SIZE_REAL / sizeof (BMAP_TYPE) - 1;
		unusable > BMAP_UNIT; i--)
	{
		bmap[i] = ~0U;
		unusable -= BMAP_UNIT;
	}

	/* (1U << sizeof (unsigned int) * 8) is undefined. */
	if (unusable)
		bmap[i] |= ~((1U << (BMAP_UNIT - unusable)) - 1);

	unsigned written = dev->m_Write (BLK_SCT_SIZEOF (SuperBlk),
		bmap, bmap_size * BLK_SIZE);
	free (bmap);
	return written == bmap_size * BLK_SIZE;
}

int
FsMount (TBlkDev *dev)
{
	if (g_ctx.mounted || !dev)
	{
		DEBUG ("EE Rejected Mount\n");
		return 0;
	}

	if (dev->m_Read (0, g_ctx.super_blk.overlay, BLK_SCT_SIZEOF (SuperBlk))
		!= BLK_SCT_SIZEOF (SuperBlk))
		return 0;

	SuperBlk *psb = &g_ctx.super_blk.sb;
	if (psb->ident != IDENT_MAGIC
	 || psb->state != CLEAN_MAGIC
	 || psb->root_dir_len > DIR_ENTRIES_MAX)
	{
		DEBUG ("EE Superblock check failed\n");
		return 0;
	}

	/* Mark the on-disk superblock dirty. */
	psb->state = DIRTY_MAGIC;
	if (dev->m_Write (0, g_ctx.super_blk.overlay, BLK_SCT_SIZEOF (SuperBlk))
		!= BLK_SCT_SIZEOF (SuperBlk))
	{
		DEBUG ("EE Cannot overwrite the superblock\n");
		return 0;
	}

	BMAP_TYPE *bmap = (BMAP_TYPE *) malloc (psb->bmap_size * BLK_SIZE_REAL);
	if (dev->m_Read (BLK_SCT_SIZEOF (SuperBlk),
		bmap, psb->bmap_size * BLK_SIZE)
		!= (signed) psb->bmap_size * BLK_SIZE)
	{
		DEBUG ("EE Failed to read the bitmap\n");
		return 0;
	}

	g_ctx.dev = *dev;
	g_ctx.mounted = 1;

	dcache_init ();

	g_ctx.bmap.size = psb->bmap_size * BLK_SIZE_REAL * 8 / BMAP_UNIT;
	g_ctx.bmap.free_iter = 0;
	g_ctx.bmap.bits = bmap;

	/* Compute reference count of i-nodes. */
	for (unsigned inode = 0; inode < INODES_MAX; inode++)
		g_ctx.inode_ref_cnt[inode] = 0;
	for (unsigned i = 0; i < psb->root_dir_len; i++)
		g_ctx.inode_ref_cnt[psb->root_dir[i].inode]++;

	return 1;
}

int
FsUmount (void)
{
	if (!g_ctx.mounted)
	{
		DEBUG ("EE Rejected Umount\n");
		return 0;
	}

	SuperBlk *psb = &g_ctx.super_blk.sb;
	dcache_flush ();

	/* Update the on-disk block bitmap. */
	if (g_ctx.dev.m_Write (BLK_SCT_SIZEOF (SuperBlk),
		g_ctx.bmap.bits, psb->bmap_size * BLK_SIZE)
		!= (signed) psb->bmap_size * BLK_SIZE)
	{
		DEBUG ("EE Failed writing the bitmap\n");
		return 0;
	}

	/* Mark the on-disk superblock clean. */
	psb->state = CLEAN_MAGIC;
	if (g_ctx.dev.m_Write (0, g_ctx.super_blk.overlay,
		BLK_SCT_SIZEOF (SuperBlk)) != BLK_SCT_SIZEOF (SuperBlk))
	{
		DEBUG ("EE Failed writing the superblock\n");
		return 0;
	}

	free (g_ctx.bmap.bits);
	memset (&g_ctx, 0, sizeof g_ctx);
	return 1;
}

int
FileOpen (const char *filename, int write_mode)
{
	if (!g_ctx.mounted || !filename || (write_mode & ~1)
	 || strlen (filename) > FILENAME_LEN_MAX)
		return -1;

	/* Find a free file descriptor. */
	int fd;
	for (fd = 0; fd < OPEN_FILES_MAX; fd++)
		if (!g_ctx.fds[fd].open)
			break;
	if (fd == OPEN_FILES_MAX)
		return -1;

	unsigned short inode;
	if (!fs_find_inode (filename, &inode))
	{
		if (!write_mode)
			return -1;

		/* Find a free i-node. */
		for (inode = 0; inode < INODES_MAX; inode++)
			if (!g_ctx.inode_ref_cnt[inode])
				break;
		assert (inode != INODES_MAX);

		/* Write a directory entry. */
		if (!fs_add_entry (filename, inode))
			return -1;

		/* Initialize the i-node. */
		g_ctx.inode_ref_cnt[inode]++;

		INode *pi = &g_ctx.super_blk.sb.inodes[inode];
		pi->indir_id = BLK_INVALID;
		pi->size = 0;
	}
	else if (write_mode)
		fs_truncate (inode);

	/* Initialize the descriptor. */
	FD *pfd = &g_ctx.fds[fd];
	pfd->open = 1;
	pfd->wr_mode = write_mode;
	pfd->inode = inode;
	pfd->offset = 0;
	pfd->blk_id = BLK_INVALID;
	pfd->ext_rem = 0;

	g_ctx.inode_ref_cnt[inode]++;
	return fd;
}

int
FileRead (int fd, void *buffer, int len)
{
	if (!g_ctx.mounted || !buffer || len <= 0
	 || fd < 0 || fd >= OPEN_FILES_MAX)
		return 0;

	FD *pfd = &g_ctx.fds[fd];
	if (!pfd->open || pfd->wr_mode)
		return 0;

	/* First compute how much we can actually get. */
	INode *pinode = &g_ctx.super_blk.sb.inodes[pfd->inode];

	unsigned remains = len;
	if (pfd->offset > pinode->size)
		remains = 0;
	else if (pfd->offset + remains > pinode->size)
		remains = pinode->size - pfd->offset;

	unsigned read = 0;
	unsigned blk_offset = pfd->offset % BLK_SIZE_REAL;

	while (remains)
	{
		/* Eventually request ID's of blocks to read from. */
		if (pfd->blk_id == BLK_INVALID)
		{
			GECtx ctx;
			Extent ext;

			if (!fs_get_extent_try (&ctx, &ext, pfd->inode, pfd->offset))
			{
				DEBUG ("EE Failed to get extent for file data\n");
				abort ();
			}

			pfd->blk_id  = ext.blk_id;
			pfd->ext_rem = ext.len;
		}

		/* Read as much as we can from the current block. */
		unsigned to_read = BLK_SIZE_REAL - blk_offset;
		if (to_read > remains)
			to_read = remains;

		DCEntry *pentry = dcache_get_block (pfd->blk_id, 0);
		assert (pentry != NULL);
		memcpy ((char *) buffer + read,
			pentry->data + blk_offset, to_read);

		remains -= to_read;
		read += to_read;
		pfd->offset += to_read;

		/* If we're not at the end of the block, we're done. */
		if (pfd->offset % BLK_SIZE_REAL != 0)
			break;

		/* Move on to the next block in extent, if possible. */
		blk_offset = 0;
		if (--pfd->ext_rem)
			pfd->blk_id++;
		else
			pfd->blk_id = BLK_INVALID;
	}

	return read;
}

int
FileWrite (int fd, const void *buffer, int len)
{
	if (!g_ctx.mounted || !buffer || len <= 0
	 || fd < 0 || fd >= OPEN_FILES_MAX)
		return 0;

	FD *pfd = &g_ctx.fds[fd];
	if (!pfd->open || !pfd->wr_mode)
	{
		DEBUG ("EE Tried to write into a bad file descriptor\n");
		return 0;
	}

	unsigned remains = len;
	unsigned written = 0;
	unsigned blk_offset = pfd->offset % BLK_SIZE_REAL;

	int overwriting = 0;

	while (remains)
	{
		/* Eventually request ID's of blocks to write to. */
		if (pfd->blk_id == BLK_INVALID)
		{
			GECtx ctx;
			Extent ext;

			if (!fs_get_extent_try (&ctx, &ext, pfd->inode, pfd->offset))
			{
				/* If this fails, probably no disk space left. */
				if (!fs_get_extent_finish (&ctx, &ext))
					break;
				overwriting = 1;
			}

			pfd->blk_id  = ext.blk_id;
			pfd->ext_rem = ext.len;
		}

		/* Write as much as we can to the current block. */
		unsigned to_write = BLK_SIZE_REAL - blk_offset;
		if (to_write > remains)
			to_write = remains;

		DCEntry *pentry = dcache_get_block (pfd->blk_id,
			overwriting | (to_write == BLK_SIZE_REAL));
		assert (pentry != NULL);
		memcpy (pentry->data + blk_offset,
			(const char *) buffer + written, to_write);
		pentry->dirty = 1;

		remains -= to_write;
		written += to_write;
		pfd->offset += to_write;

		/* If we're not at the end of the block, we're done. */
		if (pfd->offset % BLK_SIZE_REAL != 0)
			break;

		/* Move on to the next block in extent, if possible. */
		blk_offset = 0;
		if (--pfd->ext_rem)
			pfd->blk_id++;
		else
			pfd->blk_id = BLK_INVALID;
	}

	/* Increase size of the file, if needed. */
	INode *pinode = &g_ctx.super_blk.sb.inodes[pfd->inode];
	if (pinode->size < pfd->offset)
		pinode->size = pfd->offset;

	return written;
}

int
FileClose (int fd)
{
	if (!g_ctx.mounted || fd < 0 || fd >= OPEN_FILES_MAX)
		return -1;

	FD *pfd = &g_ctx.fds[fd];
	if (!pfd->open)
		return -1;

	pfd->open = 0;
	fs_unref_inode (pfd->inode);

	return 0;
}

int
FileDelete (const char *filename)
{
	if (!g_ctx.mounted || !filename)
		return 0;

	DirEntry *pentry;
	if (!fs_find_entry (filename, &pentry))
		return 0;

	fs_unref_inode (pentry->inode);
	fs_remove_entry (pentry);

	return 1;
}

int
FileFindFirst (TFile *info)
{
	if (!g_ctx.mounted || !g_ctx.super_blk.sb.root_dir_len || !info)
		return 0;

	SuperBlk *psb = &g_ctx.super_blk.sb;
	DirEntry *pentry = &psb->root_dir[0];

	strcpy (info->m_FileName, pentry->name);
	info->m_FileSize = psb->inodes[pentry->inode].size;

	return 1;
}

int
FileFindNext (TFile *info)
{
	if (!g_ctx.mounted || !info)
		return 0;

	DirEntry *pentry;
	if (fs_find_entry (info->m_FileName, &pentry))
		pentry++;

	SuperBlk *psb = &g_ctx.super_blk.sb;
	if (pentry - psb->root_dir == psb->root_dir_len)
		return 0;

	strcpy (info->m_FileName, pentry->name);
	info->m_FileSize = psb->inodes[pentry->inode].size;

	return 1;
}

int
FileSize (const char *filename)
{
	if (!g_ctx.mounted || !filename)
		return -1;

	INode *pi = fs_find_inode (filename, NULL);
	return pi ? pi->size : -1;
}


