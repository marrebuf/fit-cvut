/** Because I already had an unfinished implementation of a B-tree laying
 *  around in libxtnd for quite some time, I decided to complete the code and
 *  just wrap it around in a template so that progtest is happy.  It looks
 *  kind of weird, though.
 *
 *  The implementation of B-tree is licensed under the same terms as libxtnd.
 *  Copyright Premysl Janouch 2009, 2011, 2012. All rights reserved.
 *
 *  For the rest of this file, I don't care, as usual.
 *
 */

#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

#include <cassert>
using namespace std;

class InvalidIndexException
{
public:
	InvalidIndexException (int idx) {m_Idx = idx;}
	friend ostream &operator << (ostream &os, const InvalidIndexException &x)
	{
		return os << x.m_Idx;
	}
private:
	int m_Idx;
};

#else /* __PROGTEST__ */
#define assert(cond)
#endif /* __PROGTEST__ */


/* ===== B-tree from libxtnd =============================================== */

/** Defines a callback function for comparing two items.
 *  It should return > 0 if @a first is greater than @a second,
 *  0 if they're equal and < 0 if @a second is greater than @a first.
 */
typedef int (*BtKeyCmpCB) (const void *first, const void *second);

/** Defines a function for freeing items in a B-tree (ie.\ the item
 *  destructor). The function takes two arguments. The first is a pointer to
 *  the item to be freed. The second argument is unspecified.
 */
typedef void (*BtWalkCB) (void *p, void *extra);

/** The B-tree class. */
struct BTree
{
	/** The minimum count of items in a node. */
	unsigned min;
	/** The maximum count of items in a node. */
	unsigned max;
	/** Callback function for comparing keys. */
	BtKeyCmpCB keycmp;
	/** Callback function for freeing keys. */
	BtWalkCB free;
	/** Specifies the extra parameter for BtFreeCB. */
	void *freeExtra;
	/** Points to the root node. */
	void **rootNode;
};


/*  A node is formed from an array of (order * 2 - 1) pointers.
 *  The node tree of a B-tree of order 3 could look like this:
 *
 *                           ___ _0_ ___ _1_ ___
 *                          |_P_|_#_|_P_|_#_|_P_|
 *    ______________________/ ________|         \___
 *   /__ _0_ ___ _1_ ___     /__ _0_ ___ _1_ ___    \___ _0_ ___ _1_ ___
 *  |_0_|_#_|_0_|_0_|_0_|   |_0_|_#_|_0_|_#_|_0_|   |_0_|_#_|_0_|_#_|_0_|
 *
 *  where 0 is a NULL pointer, # is non-NULL item data, P is a child pointer
 *  and the numbers above are key indexes.
 */

#define BT_NODE_ALLOC(bt) \
	((void **) (calloc ((bt)->max * 2 + 1, sizeof (void *))))

#define BT_NODE_BEFORE(node, index)  ((node)[2 * (index)])
#define BT_NODE_DATA(node, index)    ((node)[2 * (index) + 1])
#define BT_NODE_AFTER(node, index)   ((node)[2 * (index) + 2])

#define BT_NODE_IS_LEAF(node)       (!*(node))
#define BT_NODE_IS_FULL(bt, node)    ((node)[2 * ((bt)->max) - 1])

#define BT_NODE_SIMPLE_INSERT(node, i, size, key, nodeAfterKey) \
{ \
	/* Move what follows. */ \
	if ((size) - (i)) \
		memmove ((node) + (i) * 2 + 3, (node) + (i) * 2 + 1, \
			((size) - (i)) * 2 * sizeof (void *)); \
	BT_NODE_DATA ((node), (i)) = (key); \
	BT_NODE_AFTER ((node), (i)) = (nodeAfterKey); \
}


/** Create a new B-tree object.
 *  @param[in] order  The order of the B-tree (one less than the maximum
 *                    count of children a node can have).
 *  @param[in] cmp  A callback function for comparing items.
 *  @param[in] free  Function for freeing items (can be NULL).
 *  @param[in] freeExtra  The extra parameter given to @a free.
 *  @return A handle to the object.
 *  @see btFree()
 */
BTree *btCreate (unsigned order, BtKeyCmpCB cmp, BtWalkCB free, void *freeExtra)
{
	BTree *bt;

	assert (order >= 3);
	assert (cmp != NULL);

	bt = (BTree *) malloc (sizeof (struct BTree));
	bt->max = order - 1;
	bt->min = bt->max / 2;
	bt->keycmp = cmp;
	bt->free = free;
	bt->freeExtra = freeExtra;
	bt->rootNode = NULL;

	return bt;
}

/** Recursively delete a node with its children. */
static void btClearNode (BTree *bt, void **node)
{
	unsigned i;

	if (bt->free)
		for (i = 0; i < bt->max && BT_NODE_DATA (node, i); i++)
			bt->free (BT_NODE_DATA (node, i), bt->freeExtra);
	for (i = 0; i <= bt->max && BT_NODE_BEFORE (node, i); i++)
		btClearNode (bt, (void **) BT_NODE_BEFORE (node, i));

	free (node);
}

/** Clear a B-tree.
 *  @param[in] bt  A B-tree object.
 */
void btClear (BTree *bt)
{
	assert (bt != NULL);

	if (bt->rootNode)
		btClearNode (bt, bt->rootNode);
	bt->rootNode = NULL;
}

/** Free a B-tree object.
 *  @param[in] bt  A B-tree object.
 *  @see btCreate()
 */
void btFree (BTree *bt)
{
	assert (bt != NULL);

	btClear (bt);
	free (bt);
}

/** Try to find an item matching with the key and return it.
 *  @param[in] bt  A B-tree object.
 *  @param[in] key  Specifies the item to search for.
 *  @return The stored item if found, NULL otherwise.
 */
void *btFind (BTree *bt, const void *key)
{
	void **node, *el;
	unsigned i;
	int c;

	assert (bt != NULL);

	if (!key)
		return NULL;

	for (node = bt->rootNode, i = 0; node;)
	{
		el = BT_NODE_DATA (node, i);
		c = bt->keycmp (el, key);
		if (!c)
			return el;

		if (c < 0)
		{
			i++;
			if (i < bt->max && BT_NODE_DATA (node, i))
				continue;
		}
		node = (void **) BT_NODE_BEFORE (node, i);
		i = 0;
	}
	return NULL;
}

/** Compute the length of @a node. */
static inline unsigned btNodeSize (BTree *bt, void **node)
{
	unsigned i, size = 0;
	for (i = 0; i < bt->max; i++)
		if (BT_NODE_DATA (node, i))
			size++;
	return size;
}

/** Insert into a full node and split it.
 *  @param[in] bt  A B-tree object.
 *  @param[in] node  The node the item is to be inserted into.
 *  @param[in] i  The index to insert at.
 *  @param[in] item  The item to be inserted.
 *  @param[in] nodeAfterItem  This node follows @a item.
 *  @param[out] upItem  The item that moves up.
 *  @param[out] upNode  This node follows @a upItem.
 */
static void btInsertSplit (BTree *bt, void **node, int i,
	void *item, void **nodeAfterItem,
	void **upItem, void **upNode)
{
	void **newNode, **currentNodeAfter, *currentItem;
	int median, sourceIndex, k;

	assert (BT_NODE_IS_FULL (bt, node));

	*upNode = newNode = BT_NODE_ALLOC (bt);
	median = bt->max / 2;

	/* Move data into the new node. */
	for (k = bt->max; ; k--)
	{
		if (k == i)
		{
			currentItem = item;
			currentNodeAfter = nodeAfterItem;
		}
		else
		{
			sourceIndex = k > i ? k - 1 : k;

			currentItem = BT_NODE_DATA (node, sourceIndex);
			currentNodeAfter = (void **) BT_NODE_AFTER (node, sourceIndex);
			BT_NODE_DATA (node, sourceIndex) = NULL;
			BT_NODE_AFTER (node, sourceIndex) = NULL;
		}

		if (k == median)
		{
			*upItem = currentItem;
			BT_NODE_BEFORE (newNode, 0) = currentNodeAfter;
			break;
		}

		BT_NODE_DATA (newNode, k - 1 - median) = currentItem;
		BT_NODE_AFTER (newNode, k - 1 - median) = currentNodeAfter;
	}

	/* Since the original node is half-empty now, we do a simple insertion. */
	if (i < median)
		BT_NODE_SIMPLE_INSERT (node, i, median, item, nodeAfterItem);
}

/** The result of a call to btInsertInternal(). */
enum BtInsertInternalResult
{
	/** Item already found in the tree. */
	BT_INSERT_INTERNAL_FOUND,
	/** Item inserted, there was enough space in the node. */
	BT_INSERT_INTERNAL_INSERTED,
	/** Item inserted, node split into two, moving the median up. */
	BT_INSERT_INTERNAL_INSERTED_SPLIT
};

/** Internal B-tree insertion function.
 *  @param[in] bt  A B-tree object.
 *  @param[in] key  An item to be inserted.
 *  @param[in] node  The node the item is to be inserted into.
 *  @param[out] upItem  If split, this is the item that moves up.
 *  @param[out] upNode  If split, this is the new node after @a upItem.
 *  @return See @a BtInsertInternalResult. If the node is already full
 *  	and splits because of this, @a upItem and @a upNode are set.
 */
static enum BtInsertInternalResult btInsertInternal (BTree *bt,
	void *item, void **node, void **upItem, void **upNode)
{
	void *nodeAfterItem = NULL;
	unsigned i, insertIndex, nItems;
	int c;

	if (!node)
	{
		*upItem = item;
		*upNode = NULL;
		return BT_INSERT_INTERNAL_INSERTED_SPLIT;
	}

	nItems = btNodeSize (bt, node);

	for (i = 0; ; i++)
	{
		c = bt->keycmp (BT_NODE_DATA (node, i), item);
		if (!c)
			return BT_INSERT_INTERNAL_FOUND;

		/* Insert, append or continue. */
		if (c > 0)
			insertIndex = i;
		else if (i + 1 == nItems)
			insertIndex = i + 1;
		else
			continue;

		if (!BT_NODE_IS_LEAF (node))
		{
			enum BtInsertInternalResult result;

			/* Recurse; if a key is promoted up, just fall through
			 * and insert it as if this node was a leaf node.
			 */
			result = btInsertInternal (bt, item,
				(void **) BT_NODE_BEFORE (node, insertIndex),
				(void **) &item, &nodeAfterItem);
			if (result != BT_INSERT_INTERNAL_INSERTED_SPLIT)
				return result;
		}

		/* Full nodes need splitting. */
		if (nItems == bt->max)
		{
			btInsertSplit (bt, node, insertIndex,
				item, (void **) nodeAfterItem, upItem, upNode);
			return BT_INSERT_INTERNAL_INSERTED_SPLIT;
		}

		BT_NODE_SIMPLE_INSERT (node, insertIndex, nItems, item, nodeAfterItem);
		return BT_INSERT_INTERNAL_INSERTED;
	}
}

/** Insert an item into the B-tree.
 *  @param[in] bt  A B-tree object.
 *  @param[in] item  The item to insert.
 *  @return @a item if the item was inserted, NULL otherwise.
 */
void *btInsert (BTree *bt, void *item)
{
	void *upItem, *upNode;
	void **newRoot;

	assert (bt != NULL);

	if (!item)
		return NULL;

	switch (btInsertInternal (bt, item, bt->rootNode, &upItem, &upNode))
	{
	case BT_INSERT_INTERNAL_FOUND:
	default:
		return NULL;
	case BT_INSERT_INTERNAL_INSERTED_SPLIT:
		newRoot = BT_NODE_ALLOC (bt);
		BT_NODE_BEFORE (newRoot, 0) = bt->rootNode;
		BT_NODE_DATA (newRoot, 0) = upItem;
		BT_NODE_AFTER (newRoot, 0) = upNode;
		bt->rootNode = newRoot;
	case BT_INSERT_INTERNAL_INSERTED:
		return item;
	}
}

enum BtDeleteInternalResult
{
	BT_DELETE_INTERNAL_OK,
	BT_DELETE_INTERNAL_DEFICIENT
};

/** Delete item on index @a i and its right child from @a node,
 *  which has @a size items total, and return if the node has become deficient.
 */
static inline enum BtDeleteInternalResult btDeleteSimple (BTree *bt,
	void **node, unsigned i, unsigned size)
{
	if (--size - i)
		memmove (node + i * 2 + 1, node + i * 2 + 3,
			(size - i) * 2 * sizeof *node);
	BT_NODE_DATA (node, size) = NULL;
	BT_NODE_AFTER (node, size) = NULL;

	if (size < bt->min)
		return BT_DELETE_INTERNAL_DEFICIENT;
	return BT_DELETE_INTERNAL_OK;
}

/** Fix the deficient left child of item @a i in @a node, sized @a size. */
static enum BtDeleteInternalResult btRebalance (BTree *bt,
	void **node, unsigned i, unsigned size)
{
	unsigned defSize, leftSize, rightSize;
	void **defNode, **leftSibling = NULL, **rightSibling = NULL;

	defNode = (void **) BT_NODE_BEFORE (node, i);
	defSize = btNodeSize (bt, defNode);

	if (i > 0)
	{
		leftSibling = (void **) BT_NODE_BEFORE (node, i - 1);
		leftSize = btNodeSize (bt, leftSibling);
		/* Try to steal an item from the left sibling. */
		if (leftSize > bt->min)
		{
			/* Make place for an item. */
			if (defSize)
				memmove (defNode + 2, defNode,
					(defSize * 2 + 1) * sizeof *defNode);
			/* Use the sibling's rightmost subtree as our leftmost. */
			leftSize--;
			BT_NODE_BEFORE (defNode, 0) = BT_NODE_AFTER (leftSibling, leftSize);
			/* And right shift the item via the parent node. */
			BT_NODE_DATA (defNode, 0) = BT_NODE_DATA (node, i - 1);
			BT_NODE_DATA (node, i - 1) = BT_NODE_DATA (leftSibling, leftSize);
			/* Delete the stolen item. */
			BT_NODE_DATA (leftSibling, leftSize) = NULL;
			BT_NODE_AFTER (leftSibling, leftSize) = NULL;
			return BT_DELETE_INTERNAL_OK;
		}
	}
	if (i < size)
	{
		rightSibling = (void **) BT_NODE_AFTER (node, i);
		rightSize = btNodeSize (bt, rightSibling);
		/* Try to steal an item from the right sibling. */
		if (rightSize > bt->min)
		{
			/* Use the sibling's leftmost subtree as our rightmost. */
			BT_NODE_AFTER (defNode, defSize) = BT_NODE_BEFORE (rightSibling, 0);
			/* And left shift the item via the parent node. */
			BT_NODE_DATA (defNode, defSize) = BT_NODE_DATA (node, i);
			BT_NODE_DATA (node, i) = BT_NODE_DATA (rightSibling, 0);
			/* Delete the stolen item. */
			rightSize--;
			memmove (rightSibling, rightSibling + 2,
				(rightSize * 2 + 1) * sizeof *defNode);
			BT_NODE_DATA (rightSibling, rightSize) = NULL;
			BT_NODE_AFTER (rightSibling, rightSize) = NULL;
			return BT_DELETE_INTERNAL_OK;
		}
	}

	/* We have to merge the node with one of the siblings. */
	if (leftSibling)
	{
		/* Move the separator and the deficient node to the sibling. */
		BT_NODE_DATA (leftSibling, leftSize++) = BT_NODE_DATA (node, i - 1);
		memmove (leftSibling + leftSize * 2, defNode,
			(defSize * 2 + 1) * sizeof *defNode);
		free (defNode);
		/* Remove the separator and the deficient node from the parent. */
		return btDeleteSimple (bt, node, i - 1, size);
	}
	if (rightSibling)
	{
		/* Move the separator and the sibling to the deficient node. */
		BT_NODE_DATA (defNode, defSize++) = BT_NODE_DATA (node, i);
		memmove (defNode + defSize * 2, rightSibling,
			(rightSize * 2 + 1) * sizeof *defNode);
		free (rightSibling);
		/* Remove the separator and the right sibling from the parent. */
		return btDeleteSimple (bt, node, i, size);
	}

	abort ();
}

/** Like btDeleteInternal(), but this removes the leftmost item in a subtree. */
static enum BtDeleteInternalResult btStealSuccessor (BTree *bt,
	void **node, void **successor)
{
	unsigned nItems;
	enum BtDeleteInternalResult result;

	assert (node != NULL);
	nItems = btNodeSize (bt, node);

	/* Delete the leftmost item in this subtree and return it. */
	if (BT_NODE_IS_LEAF (node))
	{
		*successor = BT_NODE_DATA (node, 0);
		return btDeleteSimple (bt, node, 0, nItems);
	}

	result = btStealSuccessor (bt,
		(void **) BT_NODE_BEFORE (node, 0), successor);
	if (result == BT_DELETE_INTERNAL_DEFICIENT)
		return btRebalance (bt, node, 0, nItems);
	return BT_DELETE_INTERNAL_OK;
}

/** Remove an item matching with @a key from the subtree rooted at @a node,
 *  return this item in @a outputItem and return whether the node has become
 *  deficient.
 */
static enum BtDeleteInternalResult btDeleteInternal (BTree *bt,
	void **node, const void *key, void **outputItem)
{
	unsigned i, nItems;
	enum BtDeleteInternalResult result;
	int c;

	if (!node)
		return BT_DELETE_INTERNAL_OK;

	nItems = btNodeSize (bt, node);

	for (i = 0; ; i++)
	{
		c = bt->keycmp (BT_NODE_DATA (node, i), key);
		if (!c)
		{
			*outputItem = BT_NODE_DATA (node, i);
			if (BT_NODE_IS_LEAF (node))
				return btDeleteSimple (bt, node, i, nItems);

			/* If we have to remove from a non-leaf node and the node
			 * becomes deficient, we must replace it with its successor
			 * (or predecessor, which is not implemented here). */
			result = btStealSuccessor (bt, (void **) BT_NODE_AFTER (node, i),
				&BT_NODE_DATA (node, i));
			i++;
		}
		else if (c > 0)
			result = btDeleteInternal (bt,
				(void **) BT_NODE_BEFORE (node, i), key, outputItem);
		else if (i + 1 == nItems)
			result = btDeleteInternal (bt,
				(void **) BT_NODE_BEFORE (node, ++i), key, outputItem);
		else
			continue;

		if (result == BT_DELETE_INTERNAL_DEFICIENT)
			return btRebalance (bt, node, i, nItems);
		return BT_DELETE_INTERNAL_OK;
	}
}

/** Delete the item matching with the key and return it.
 *  @param[in] bt  A B-tree object.
 *  @param[in] key  Specifies the item to be deleted.
 *  @return The stored item if found and deleted, NULL otherwise.
 */
void *btDelete (BTree *bt, const void *key)
{
	void *item = NULL;

	assert (bt != NULL);

	if (!key)
		return NULL;

	/* If the root has no elements but one child, replace it with the child.
	 * Otherwise we don't care if it's deficient.
	 */
	if (btDeleteInternal (bt, bt->rootNode, key, &item)
		== BT_DELETE_INTERNAL_DEFICIENT && !BT_NODE_DATA (bt->rootNode, 0))
	{
		void **root;

		root = bt->rootNode;
		bt->rootNode = (void **) BT_NODE_BEFORE (bt->rootNode, 0);
		free (root);
	}

	return item;
}

static void btWalkInternal (BTree *bt, void **node, BtWalkCB walk, void *extra)
{
	unsigned i;

	if (!node)
		return;

	for (i = 0; i < bt->max && BT_NODE_DATA (node, i); i++)
	{
		btWalkInternal (bt, (void **) BT_NODE_BEFORE (node, i), walk, extra);
		walk (BT_NODE_DATA (node, i), extra);
	}
	btWalkInternal (bt, (void **) BT_NODE_BEFORE (node, i), walk, extra);
}

/** Walk through all items in the tree in ascending order.
 *  @param[in] bt  A B-tree object.
 *  @param[in] walk  A user-defined walk callback.
 *  @param[in] extra  A user-defined parameter for @a walk.
 */
void btWalk (BTree *bt, BtWalkCB walk, void *extra)
{
	assert (bt != NULL);
	assert (walk != NULL);

	btWalkInternal (bt, bt->rootNode, walk, extra);
}


/* ===== The wrapper template class ======================================== */

template <class T>
class CSparseArray
{
	struct Item
	{
		int idx;
		T val;

		Item (const T &_val, int _idx = 0)
			: val (_val) {idx = _idx;}
		Item (const Item &i)
			: val (i.val) {idx = i.idx;}
	};

	struct FakeItem
	{
		int idx;
	};

	struct PrintCtx
	{
		std::ostream *os;
		bool first;
	};

protected:
	BTree *bt;
	int size;

	static void item_free (void *item, void *extra)
	{
		delete (Item *) item;
	}

	static int item_cmp (const void *a, const void *b)
	{
		return ((const Item *) a)->idx - ((const Item *) b)->idx;
	}

	static int item_equals (const void *a, const void *b)
	{
		Item *item1 = (Item *) a;
		Item *item2 = (Item *) b;
		return item1->idx == item2->idx && item1->val == item2->val;
	}

	static void item_cmp_trees (void *item, void *extra)
	{
		BTree *bt = (BTree *) extra;

		void *found = btFind (bt, item);
		if (!found || !item_equals (item, found))
			throw 0xDEADBEEF;
	}

	static void item_print (void *item, void *extra)
	{
		PrintCtx *ctx = (PrintCtx *) extra;
		Item *i = (Item *) item;
		std::ostream &os = *ctx->os;
		if (!ctx->first)  os << ", ";
		os << "[" << i->idx;
		os << "] => " << i->val;
		ctx->first = false;
	}

	static void item_clone (void *item, void *tree)
	{
		BTree *bt = (BTree *) tree;
		Item *i = new Item (*(Item *) item);
		btInsert (bt, i);
	}

public:
	~CSparseArray ()
	{
		btFree (bt);
	}

	CSparseArray ()
	{
		bt = btCreate (5, item_cmp, item_free, NULL);
		size = 0;
	}

	CSparseArray (const CSparseArray<T> &a)
	{
		bt = btCreate (5, item_cmp, item_free, NULL);
		size = a.size;

		btWalk (a.bt, item_clone, bt);
	}

	CSparseArray<T> &operator = (const CSparseArray<T> &a)
	{
		if (this != &a)
		{
			this->~CSparseArray ();
			new (this) CSparseArray<T> (a);
		}
		return *this;
	}

	CSparseArray<T> &Set (int index, const T &x)
	{
		FakeItem key = {index};
		Item *i = (Item *) btFind (bt, &key);
		if (i)
			i->val = x;
		else
		{
			btInsert (bt, new Item (x, index));
			size++;
		}
		return *this;
	}

	CSparseArray<T> &Unset (int index)
	{
		FakeItem key = {index};
		Item *i = (Item *) btDelete (bt, &key);
		if (i)
		{
			delete i;
			size--;
		}
		return *this;
	}

	bool IsSet (int index) const
	{
		FakeItem key = {index};
		Item *i = (Item *) btFind (bt, &key);
		return i != NULL;
	}

	int Size () const
	{
		return size;
	}

	bool operator == (const CSparseArray<T> &a) const
	{
		if (size != a.size)  return false;

		try {
			btWalk (bt, item_cmp_trees, a.bt);
			return true;
		} catch (...) {
			return false;
		}
	}

	bool operator != (const CSparseArray<T> &a) const
	{
		return !(*this == a);
	}

	const T &operator [] (int index) const
	{
		FakeItem key = {index};
		Item *i = (Item *) btFind (bt, &key);
		if (i)
			return i->val;
		throw InvalidIndexException (index);
	}

	T &operator [] (int index)
	{
		FakeItem key = {index};
		Item *i = (Item *) btFind (bt, &key);
		if (i)
			return i->val;
		throw InvalidIndexException (index);
	}

	friend std::ostream &operator <<
		(std::ostream &os, const CSparseArray<T> &a)
	{
		os << "{ ";

		PrintCtx ctx;
		ctx.os = &os;
		ctx.first = true;

		btWalk (a.bt, item_print, &ctx);
		if (!ctx.first)
			os << " ";

		os << "}";
		return os;
	}
};


/* ===== Some very thorough testing ======================================== */

#ifndef __PROGTEST__
#include <cassert>
#include <sstream>

class CInt
{
	int *val;
public:
	CInt (int new_val) : val (new int (new_val)) {}
	CInt (const CInt &c) : val (new int (*c.val)) {}
	~CInt () {delete val;}
	bool operator == (const CInt &i) {return *i.val == *val;}
	bool operator != (const CInt &i) {return *i.val != *val;}

	CInt &operator = (const CInt &c)
	{
		if (this != &c)
			*val = *c.val;
		return *this;
	}

	friend std::ostream &operator << (std::ostream &os, const CInt &i)
	{
		return os << *i.val;
	}
};

template <class T>
class CTestArray : public CSparseArray<T>
{
public:
	void self_check () const
	{
		assert (this->size >= 0);
	}
};

template<typename T> void
check_output (const T &x, const char *ref)
{
	ostringstream os;
	os << x;
	assert (os.str () == ref);
}

int
main (int argc, char *argv[])
{
	CSparseArray<int>  A1;
	A1.Set (100, 20);
	A1.Set (30000, 80);
	A1.Set (-6000, 120);
	assert (A1.IsSet (0) == false);
	assert (A1.IsSet (30000) == true);
	assert (A1 [ 30000 ] == 80);
	assert (A1.Size () == 3);
	check_output (A1, "{ [-6000] => 120, [100] => 20, [30000] => 80 }");
	A1.Unset (100);
	assert (A1.Size () == 2);
	check_output (A1, "{ [-6000] => 120, [30000] => 80 }");
	A1.Set (30000, 666);
	assert (A1.Size () == 2);
	check_output (A1, "{ [-6000] => 120, [30000] => 666 }");
	try {
		A1 [ 30001 ];
		abort ();
	} catch (const InvalidIndexException &ex) {
		check_output (ex, "30001");
	} catch (...) {
		abort ();
	}

	CSparseArray<int>  A2;
	CSparseArray<int>  B2;
	A2.Set (5000, 20);
	A2.Set (30000, 80);
	A2.Set (-6000, 120);
	A2.Set (-9999999, 444);
	B2.Set (30000, 80);
	B2.Set (-6000, 120);
	B2.Set (-9999999, 444);

	check_output (A2,
		"{ [-9999999] => 444, [-6000] => 120, [5000] => 20, [30000] => 80 }");
	check_output (B2,
		"{ [-9999999] => 444, [-6000] => 120, [30000] => 80 }");
	assert (!(A2 == B2));
	assert (A2 != B2);

	B2.Set (5000, 20);
	B2.Set (5000, 20);

	check_output (A2,
		"{ [-9999999] => 444, [-6000] => 120, [5000] => 20, [30000] => 80 }");
	check_output (B2,
		"{ [-9999999] => 444, [-6000] => 120, [5000] => 20, [30000] => 80 }");
	assert (A2 == B2);
	assert (!(A2 != B2));

	CSparseArray<int>  A3;
	A3.Set (100, 0);
	A3.Set (99, 1);
	A3.Set (102, 2).Set (103, 2);
	check_output (A3, "{ [99] => 1, [100] => 0, [102] => 2, [103] => 2 }");
	A3.Unset (103);
	check_output (A3, "{ [99] => 1, [100] => 0, [102] => 2 }");
	A3.Unset (99).Unset (100);
	check_output (A3, "{ [102] => 2 }");
	A3.Unset (102);
	check_output (A3, "{ }");

	CSparseArray<string>  S;
	S.Set (25, "first");
	S.Set (36, "third");
	S.Set (30, "second");
	check_output (S, "{ [25] => first, [30] => second, [36] => third }");

	CSparseArray<string>  k (S);
	k = S;
	check_output (k, "{ [25] => first, [30] => second, [36] => third }");

	/* Brute force tests, very CPU-intensive. */
	#define ALEN 200
	#define AMOV -100
	#define INVL 99999

	CInt ref[200] = INVL;
	CTestArray<CInt>  *l = new CTestArray<CInt>;
	for (int i = -10000; i <= 10000; i++)
	{
		int idx = rand () % ALEN + AMOV;
		ref[idx - AMOV] = i;
		l->Set (idx, CInt (i));
		l->self_check ();
		assert (l->IsSet (idx));

		CInt tmp = (*l)[idx];
		assert (tmp == i);

		int size = l->Size ();
		idx = rand () % ALEN + AMOV;
		ref[idx - AMOV] = INVL;

		try
		{
			(*l)[idx];
			size--;
		}
		catch (const InvalidIndexException &ex)
		{
		}

		l->Unset (idx);
		l->self_check ();
		assert (!l->IsSet (idx));
		assert (l->Size () == size);

		CTestArray<CInt> *copy = new CTestArray<CInt>;

		*copy = *l;
		copy->self_check ();
		*copy = *copy;
		copy->self_check ();
		assert (*l == *copy);
		assert (!(*l != *copy));
		delete l;
		l = copy;
	}

	for (int i = 0; i < ALEN; i++)
	{
		if (ref[i] == INVL)
			assert (!l->IsSet (i + AMOV));
		else
			assert (ref[i] == (*l)[i + AMOV]);
	}

	cout << *l << endl;
	delete l;
}
#endif /* __PROGTEST__ */

