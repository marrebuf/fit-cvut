#ifndef __PROGTEST__
class CKey
{
	int *value;
public:
	CKey (int value = 0) : value (new int (value))    {}
	CKey (const CKey &i) : value (new int (*i.value)) {}
	~CKey () {delete value;}

	CKey &operator = (const CKey &i) {
		if (this != &i) *value = *i.value; return *this;}

	bool operator == (const CKey &i) const {return *value == *i.value;}
	bool operator != (const CKey &i) const {return *value != *i.value;}
	bool operator <  (const CKey &i) const {return *value <  *i.value;}
	bool operator >  (const CKey &i) const {return *value >  *i.value;}
	bool operator >= (const CKey &i) const {return *value >= *i.value;}
	bool operator <= (const CKey &i) const {return *value <= *i.value;}

	int getValue () const {return *this->value;}
	void setValue (int v) {*value = v;}
};

class CValue
{
	int *value;
public:
	CValue (int value = 0)   : value (new int (value))    {}
	CValue (const CValue &i) : value (new int (*i.value)) {}
	~CValue () {delete value;}

	CValue &operator = (const CValue &i) {
		if (this != &i) *value = *i.value; return *this;}

	bool operator == (const CValue &i) const {return *value == *i.value;}
	bool operator != (const CValue &i) const {return *value != *i.value;}
	bool operator  < (const CValue &i) const {return *value <  *i.value;}
	bool operator  > (const CValue &i) const {return *value >  *i.value;}
	bool operator >= (const CValue &i) const {return *value >= *i.value;}
	bool operator <= (const CValue &i) const {return *value <= *i.value;}

	int getValue () const {return *this->value;}
	void setValue (int v) {*value = v;}
};
#endif // ! __PROGTEST__

#include <new>

#ifdef nullptr
#undef nullptr
#endif

#define nullptr 0

static int ids = 0;

class CLeafNode;
class CInnerNode;


class CNode
{
	friend class CBTreePlus;
	friend class CInnerNode;
protected:
	int m, gamma, id;
	CInnerNode *parent;
	CKey *keys;

	CNode (const CNode &n) : m (n.m), gamma (n.gamma), id (ids++),
		parent (n.parent), keys (new CKey[n.m - 1])
	{
		for (int i = 0; i < n.gamma; i++)
			this->keys[i] = n.keys[i];
	}
public:
	CNode (int m) : m (m), gamma (0), id (ids++),
		parent (), keys (new CKey[m - 1] ()) {}
	virtual ~CNode () {delete[] keys;}

	virtual bool isLeaf () const {return false;}
	int getGamma () const {return this->gamma;}
	CNode *getParent () const {return (CNode *) (this->parent);}
	CKey &getKey (int index) const {return this->keys[index - 1];}
	int getID () const {return this->id;}
	void setID (int id) {this->id = id;}
};

class CLeafNode : public CNode
{
	friend class CBTreePlus;
	friend class CInnerNode;
protected:
	CLeafNode *next;
	CValue *values;

	CLeafNode (const CLeafNode &leaf);
	CLeafNode &operator = (const CLeafNode &leaf);
public:
	CLeafNode (int m) : CNode (m), next (), values (new CValue[m - 1] ()) {}
	virtual ~CLeafNode () {delete[] values;}

	virtual bool isLeaf () const {return true;}
	CValue &getValue (int index) const {return this->values[index - 1];}
	CLeafNode *getNextLeaf () const {return this->next;}
};

CLeafNode::CLeafNode (const CLeafNode &leaf)
	: CNode (leaf), next (leaf.next), values (new CValue[leaf.m - 1])
{
	for (int i = 0; i < leaf.gamma; i++)
		this->values[i] = leaf.values[i];
}

CLeafNode &
CLeafNode::operator = (const CLeafNode &leaf)
{
	if (this != &leaf)
	{
		this->~CLeafNode ();
		new (this) CLeafNode (leaf);
	}
	return *this;
}

class CInnerNode : public CNode
{
	friend class CBTreePlus;
protected:
	CNode **children;

	void setChild (int index, CNode *child) {this->children[index - 1] = child;}

	CInnerNode (const CInnerNode &inner);
	CInnerNode &operator = (const CInnerNode &inner);
public:
	CInnerNode (int m) : CNode (m), children (new CNode *[m] ()) {}

	virtual ~CInnerNode ()
	{
		for (int i = 0; i <= this->gamma; i++)
			delete this->children[i];
		delete[] this->children;
	}

	CNode *getChild (int index) const {return this->children[index - 1];}
};

CInnerNode::CInnerNode (const CInnerNode &inner)
	: CNode (inner), children (new CNode *[inner.m])
{
	for (int i = 0; i <= inner.gamma; i++)
	{
		CNode *child = inner.children[i];
		CNode *&n    = this->children[i];

		if (child->isLeaf ())
			n = new CLeafNode  (*dynamic_cast<CLeafNode  *> (child));
		else
			n = new CInnerNode (*dynamic_cast<CInnerNode *> (child));

		n->parent = this;
	}
}

CInnerNode &
CInnerNode::operator = (const CInnerNode &inner)
{
	if (this != &inner)
	{
		this->~CInnerNode ();
		new (this) CInnerNode (inner);
	}
	return *this;
}

class CBTreePlus
{
	int m;
	CNode *root;

	void _relink_leafs (CNode *node, CLeafNode *&prev);

	void _insert (const CKey &key, const CValue &val, bool &retval,
		CNode &node, CNode *&new_node, CKey &promoted_key);
	void _insert_into_leaf (const CKey &key, const CValue &val, bool &retval,
		CLeafNode &node, CNode *&new_node);

	bool _delete_from_leaf (const CKey &key, bool &retval, CLeafNode &node);
	inline bool _leaf_rebalance (CInnerNode &node, int i, CLeafNode *child);
	bool _delete_from_inner (const CKey &key, bool &retval, CInnerNode &node);
public:
	CBTreePlus (int m) : m (m), root (new CLeafNode (m)) {} 
	~CBTreePlus () {delete this->root;}

	CBTreePlus (const CBTreePlus &bt);
	CBTreePlus &operator = (const CBTreePlus &bt);

	bool bTreePlusInsert (const CKey &key, const CValue &val);
	int bTreePlusSearch (const CKey &key, CLeafNode **val);
	bool bTreePlusDelete (const CKey &key);
	CNode *getRoot () {return this->root;}
};

CBTreePlus::CBTreePlus (const CBTreePlus &bt)
{
	this->m = bt.m;

	if (bt.root->isLeaf ())
		this->root = new CLeafNode  (*dynamic_cast<CLeafNode  *> (bt.root));
	else
		this->root = new CInnerNode (*dynamic_cast<CInnerNode *> (bt.root));

	CLeafNode *last = nullptr;
	this->_relink_leafs (this->root, last);
	if (last)  last->next = nullptr;
}

void
CBTreePlus::_relink_leafs (CNode *node, CLeafNode *&prev)
{
	if (node->isLeaf ())
	{
		CLeafNode *leaf   = dynamic_cast<CLeafNode  *> (node);
		if (prev)  prev->next = leaf;
		prev = leaf;
	}
	else
	{
		CInnerNode *inner = dynamic_cast<CInnerNode *> (node);
		for (int i = 1; i <= inner->gamma + 1; i++)
			this->_relink_leafs (inner->getChild (i), prev);
	}
}

CBTreePlus &
CBTreePlus::operator = (const CBTreePlus &bt)
{
	if (this != &bt)
	{
		this->~CBTreePlus ();
		new (this) CBTreePlus (bt);
	}
	return *this;
}

int
CBTreePlus::bTreePlusSearch (const CKey &key, CLeafNode **val)
{
	CNode *node = this->root;
	while (!node->isLeaf ())
	{
		int i;
		CInnerNode *inner = dynamic_cast<CInnerNode *> (node);
		for (i = 1; i <= node->gamma; i++)
			if (node->getKey (i) > key)
				break;
		node = inner->getChild (i);
	}

	CLeafNode *leaf;
	*val = leaf = dynamic_cast<CLeafNode *> (node);
	for (int i = 1; i <= node->gamma; i++)
	{
		if (node->getKey (i) > key)
			break;
		else if (node->getKey (i) == key)
			return i;
	}

	*val = nullptr;
	return -1;
}

void
CBTreePlus::_insert_into_leaf (const CKey &key, const CValue &val, bool &retval,
	CLeafNode &node, CNode *&new_node)
{
	int i, j;
	for (i = 1; i <= node.gamma; i++)
		if (node.getKey (i) > key)
			break;
		else if (node.getKey (i) == key)
		{
			node.getValue (i) = val;
			retval = false;
			return;
		}

	if (++node.gamma == this->m)
	{
		CLeafNode *new_leaf;
		new_node = new_leaf = new CLeafNode (this->m);

		new_leaf->next = node.next;
		node.next = new_leaf;

		node.gamma = this->m / 2;
		new_leaf->gamma = this->m - node.gamma;

		for (j = node.gamma + 1; j <= this->m; j++)
		{
			CKey   &new_key   = new_leaf->getKey   (j - node.gamma);
			CValue &new_value = new_leaf->getValue (j - node.gamma);
			if        (j == i) {
				new_key   = key;
				new_value = val;
			} else if (j >  i) {
				new_key   = node.getKey   (j - 1);
				new_value = node.getValue (j - 1);
			} else             {
				new_key   = node.getKey   (j);
				new_value = node.getValue (j);
			}
		}

		if (i > node.gamma)
			return;
	}

	for (j = node.gamma; j > i; j--)
	{
		node.getKey   (j) = node.getKey   (j - 1);
		node.getValue (j) = node.getValue (j - 1);
	}

	node.getKey   (i) = key;
	node.getValue (i) = val;
}

void
CBTreePlus::_insert (const CKey &key, const CValue &val, bool &retval,
	CNode &node, CNode *&new_node, CKey &promoted_key)
{
	if (node.isLeaf ())
	{
		this->_insert_into_leaf (key, val, retval,
			dynamic_cast<CLeafNode &> (node), new_node);
		if (new_node)
			promoted_key = new_node->getKey (1);
		return;
	}

	int i, j;
	for (i = 1; i <= node.getGamma (); i++)
		if (node.getKey (i) > key)
			break;

	CInnerNode &inner = dynamic_cast<CInnerNode &> (node);
	CNode *subtree = inner.getChild (i);

	CNode *new_subnode = nullptr;
	CKey new_key;
	this->_insert (key, val, retval, *subtree, new_subnode, new_key);

	if (!new_subnode)
		return;

	new_subnode->parent = &inner;

	if (++inner.gamma == this->m)
	{
		CInnerNode *new_inner;
		new_node = new_inner = new CInnerNode (this->m);

		inner.gamma = this->m / 2;
		int median = inner.gamma + 1;
		new_inner->gamma = this->m - median;

		if        (median == i) {
			promoted_key          = new_key;
			new_inner->setChild (1, new_subnode);
		} else if (median >  i) {
			promoted_key          = inner.getKey   (median - 1);
			new_inner->setChild (1, inner.getChild (median));
		} else                  {
			promoted_key          = inner.getKey   (median);
			new_inner->setChild (1, inner.getChild (median + 1));
		}

		for (j = 1; j <= new_inner->gamma; j++)
		{
			if        (median + j == i) {
				new_inner->getKey (j)     = new_key;
				new_inner->setChild (j + 1, new_subnode);
			} else if (median + j >  i) {
				new_inner->getKey (j)     = inner.getKey   (median + j - 1);
				new_inner->setChild (j + 1, inner.getChild (median + j));
			} else                      {
				new_inner->getKey (j)     = inner.getKey   (median + j);
				new_inner->setChild (j + 1, inner.getChild (median + j + 1));
			}
		}

		for (j = 1; j <= new_inner->gamma + 1; j++)
			new_inner->getChild (j)->parent = new_inner;

		if (i > inner.gamma)
			return;
	}

	for (j = inner.gamma; j > i; j--)
	{
		inner.getKey   (j)   = inner.getKey   (j - 1);
		inner.setChild (j + 1, inner.getChild (j));
	}

	inner.getKey   (i)   = new_key;
	inner.setChild (i + 1, new_subnode);
}

bool
CBTreePlus::bTreePlusInsert (const CKey &key, const CValue &val)
{
	bool retval = true;
	CNode *new_node = nullptr;
	CKey new_key;
	this->_insert (key, val, retval, *this->root, new_node, new_key);

	if (new_node)
	{
		CInnerNode *root = new CInnerNode (this->m);
		root->gamma = 1;
		root->getKey (1) = new_key;
		root->setChild (1, this->root);
		root->setChild (2, new_node);
		this->root = this->root->parent = new_node->parent = root;
	}

	return retval;
}

bool
CBTreePlus::_delete_from_leaf (const CKey &key, bool &retval, CLeafNode &node)
{
	for (int i = 1; i <= node.gamma; i++)
	{
		if (node.getKey (i) > key)
			return false;
		else if (node.getKey (i) == key)
		{
			for (int j = i; j < node.gamma; j++)
			{
				node.getValue (j) = node.getValue (j + 1);
				node.getKey   (j) = node.getKey   (j + 1);
			}
			retval = true;
			return --node.gamma < this->m / 2;
		}
	}
	return false;
}

inline bool
CBTreePlus::_leaf_rebalance (CInnerNode &node, int i, CLeafNode *child)
{
	CLeafNode *left  = nullptr;
	CLeafNode *right = nullptr;

	// See if we can steal from our left sibling.
	if (i > 1)
	{
		left = dynamic_cast<CLeafNode *> (node.getChild (i - 1));
		if (left->gamma > this->m / 2)
		{
			for (int j = ++child->gamma; j > 1; j--)
			{
				child->getKey   (j) = child->getKey   (j - 1);
				child->getValue (j) = child->getValue (j - 1);
			}
			child->getKey   (1) = left->getKey   (left->gamma);
			child->getValue (1) = left->getValue (left->gamma);

			node.getKey (i - 1) = left->getKey   (left->gamma--);
			return false;
		}
	}

	// See if we can steal from our right sibling.
	if (i <= node.gamma)
	{
		right = dynamic_cast<CLeafNode *> (node.getChild (i + 1));
		if (right->gamma > this->m / 2)
		{
			child->getKey (++child->gamma) = right->getKey   (1);
			child->getValue (child->gamma) = right->getValue (1);

			right->gamma--;
			for (int j = 1; j <= right->gamma; j++)
			{
				right->getKey   (j) = right->getKey   (j + 1);
				right->getValue (j) = right->getValue (j + 1);
			}

			node.getKey (i) = right->getKey (1);
			return false;
		}
	}

	// Convert the case of merging with the left sibling...
	if (left)
	{
		right = child;
		child = left;
		i--;
	}

	// ...to merging with our right sibling.  One of them always exists.
	for (int j = 1; j <= right->gamma; j++)
	{
		child->getKey   (child->gamma + j) = right->getKey   (j);
		child->getValue (child->gamma + j) = right->getValue (j);
	}

	child->gamma += right->gamma;
	child->next   = right->next;
	delete right;

	for (int j = i; j < node.gamma; j++)
	{
		node.getKey   (j)   = node.getKey   (j + 1);
		node.setChild (j + 1, node.getChild (j + 2));
	}

	return node.gamma-- < (this->m + 1) / 2;
}

bool
CBTreePlus::_delete_from_inner (const CKey &key, bool &retval, CInnerNode &node)
{
	int i, j;
	for (i = 1; i <= node.gamma; i++)
		if (node.getKey (i) > key)
			break;

	CNode &child_node = *node.getChild (i);
	if (child_node.isLeaf ())
	{
		CLeafNode *child = dynamic_cast<CLeafNode *> (&child_node);
		if (this->_delete_from_leaf (key, retval, *child))
			return this->_leaf_rebalance (node, i, child);
		return false;
	}
	
	CInnerNode *child = dynamic_cast<CInnerNode *> (&child_node);
	if (!this->_delete_from_inner (key, retval, *child))
		return false;

	CInnerNode *left  = nullptr;
	CInnerNode *right = nullptr;

	// See if we can steal from our left sibling.
	if (i > 1)
	{
		left = dynamic_cast<CInnerNode *> (node.getChild (i - 1));
		if (left->gamma > this->m / 2)
		{
			for (j = ++child->gamma;     j > 1; j--)
				child->getKey   (j) = child->getKey   (j - 1);
			for (j =   child->gamma + 1; j > 1; j--)
				child->setChild (j,   child->getChild (j - 1));

			child->getKey   (1)     = node .getKey   (i - 1);
			CNode *stolen           = left->getChild (left->gamma + 1);
			child->setChild (1,       stolen);
			node  .getKey   (i - 1) = left->getKey   (left->gamma--);
			stolen->parent          = child;
			return false;
		}
	}

	// See if we can steal from our right sibling.
	if (i <= node.gamma)
	{
		right = dynamic_cast<CInnerNode *> (node.getChild (i + 1));
		if (right->gamma > this->m / 2)
		{
			child->getKey   (++child->gamma)    = node  .getKey   (i);
			CNode *stolen                       = right->getChild (1);
			child->setChild   (child->gamma + 1,  stolen);
			node  .getKey     (i)               = right->getKey   (1);
			stolen->parent                      = child;

			for (j = 1; j <  right->gamma; j++)
				right->getKey   (j) = right->getKey   (j + 1);
			for (j = 1; j <= right->gamma; j++)
				right->setChild (j,   right->getChild (j + 1));
			right->gamma--;
			return false;
		}
	}

	// Convert the case of merging with the left sibling...
	if (left)
	{
		right = child;
		child = left;
		i--;
	}

	// ...to merging with our right sibling.  One of them always exists.
	child->getKey (++child->gamma) = node.getKey (i);

	for (int j = 1; j <= right->gamma;     j++)
		child->getKey   (child->gamma + j) = right->getKey (j);
	for (int j = 1; j <= right->gamma + 1; j++)
	{
		CNode *merged = right->getChild (j);
		child->setChild (child->gamma + j,   merged);
		merged->parent = child;
	}

	child->gamma += right->gamma;

	right->gamma = 0;
	right->setChild (1, nullptr);
	delete right;

	for (int j = i; j < node.gamma; j++)
	{
		node.getKey   (j)   = node.getKey   (j + 1);
		node.setChild (j + 1, node.getChild (j + 2));
	}

	return node.gamma-- < (this->m + 1) / 2;
}

bool
CBTreePlus::bTreePlusDelete (const CKey &key)
{
	bool retval = false;
	if (this->root->isLeaf ())
	{
		this->_delete_from_leaf (key, retval,
				*dynamic_cast<CLeafNode *> (this->root));
	}
	else if (this->_delete_from_inner (key, retval,
			*dynamic_cast<CInnerNode *> (this->root))
		&& this->root->getGamma () == 0)
	{
		CInnerNode *inner = dynamic_cast<CInnerNode *> (this->root);
		this->root = inner->getChild (1);
		this->root->parent = nullptr;
		inner->setChild (1, nullptr);
		delete inner;
	}

	return retval;
}

#ifndef __PROGTEST__
#include <cassert>
#include <cstdio>
#include <cstdlib>

#define N_EL(a) (sizeof (a) / sizeof *(a))

static void
visualize_bt_node (CBTreePlus &bt, FILE *dot, const CNode *node)
{
	const CLeafNode  *leaf  = dynamic_cast<const CLeafNode  *> (node);
	const CInnerNode *inner = dynamic_cast<const CInnerNode *> (node);

	CNode *parent;
	if ((parent = node->getParent ()))
		fprintf (dot, "n%d -> n%d;\n", node->getID (), parent->getID ());

	fprintf (dot, "n%d [label = \"{{", node->getID ());

	int i;
	if (node->getGamma () == 0)
		fprintf (dot, "empty");
	else
	{
		for (i = 1; i < node->getGamma (); i++)
			fprintf (dot, "<k%d> %d|", i, node->getKey (i).getValue ());
		fprintf (dot, "<k%d> %d}|{", i, node->getKey (i).getValue ());
	}

	if (node->isLeaf ())
	{
		for (i = 1; i <= leaf->getGamma (); i++)
		{
			if (i != 1)  fputc ('|', dot);
			fprintf (dot, "<o%d> [%d]", i, leaf->getValue (i).getValue ());
		}
	}
	else
	{
		for (i = 1; i <= node->getGamma (); i++)
			fprintf (dot, "<o%d>|", i);
		fprintf (dot, "<o%d>", i);
	}

	fprintf (dot, "}}\"];\n");
	if (node->isLeaf ())
	{
		const CLeafNode *next;
		if ((next = leaf->getNextLeaf ()))
			fprintf (dot, "n%d:e -> n%d:w [constraint = false];\n",
				leaf->getID (), next->getID ());
		return;
	}

	for (i = 1; i <= inner->getGamma () + 1; i++)
	{
		CNode *child = inner->getChild (i);
		if (!child)  continue;
		fprintf (dot, "n%d:o%d:c -> n%d "
			"[arrowtail = dot, dir = both, tailclip = false];\n",
			inner->getID (), i, child->getID ());
		visualize_bt_node (bt, dot, child);
	}
}

static void
visualize_bt (CBTreePlus &bt)
{
	static int ctr = 0;
	char cmdline[80];

	snprintf (cmdline, sizeof cmdline, "dot -T png -o btree_%d.png", ctr++);

	FILE *dot = popen (cmdline, "w");
	fprintf (dot, "digraph BT {\n"
		"node [shape = record, fontsize = 9];\n"
		"edge [arrowsize = 0.5];\n"
		"CBTreePlus;\n");

	CNode *root = bt.getRoot ();
	assert (root != NULL);
	fprintf (dot, "CBTreePlus -> n%d;\n", root->getID ());
	visualize_bt_node (bt, dot, root);

	fprintf (dot, "}\n");
	pclose (dot);
}

int
main (int argc, char *argv[])
{
	CKey   ti[8] = {1,  30, 20, 2,  25, 16, 18, 17},
	       td[8] = {25, 2,  18, 20, 10, 16, 17, 30};
	CValue tv[8] = {8,  7,  6,  5,  4,  3,  2,  1};

	// Basic test
	for (int m = 3; m <= 6; m++)
	{
		CBTreePlus bt (m);

		{
			CBTreePlus bt_copy (bt);
			for (unsigned i = 0; i < N_EL (ti); i++)
			{
				bool inserted = bt_copy.bTreePlusInsert (ti[i], tv[i]);
				assert (inserted == true);
			}

			for (unsigned i = 0; i < N_EL (ti); i++)
			{
				CLeafNode *node;
				int idx = bt_copy.bTreePlusSearch (ti[i], &node);
				assert (idx != -1);
				assert (node->getValue (idx) == tv[i]);
			}

			bt = bt = bt_copy;
		}

		visualize_bt (bt);

		bool inserted = bt.bTreePlusInsert (16, 16);
		assert (inserted == false);

		CLeafNode *node;
		int idx = bt.bTreePlusSearch (-1, &node);
		assert (idx == -1);
		assert (node == nullptr);

		assert (bt.bTreePlusDelete (-2) == false);

		for (unsigned i = 0; i < N_EL (td); i++)
		{
			bool deleted = bt.bTreePlusDelete (td[i]);
			if (!deleted)  assert (td[i].getValue () == 10);
		}

		visualize_bt (bt);

		bool deleted = bt.bTreePlusDelete (1);
		assert (deleted == true);
	}

	// Extended test
	for (int m = 3; m <= 6; m++)
	{
		static const int n = 16;

		CBTreePlus bt (m);
		CKey   *keys   = new CKey  [n];
		CValue *values = new CValue[n];

		for (int i = 0; i < n; i++)
		{
			keys[i]  .setValue (rand () % 10000);
			values[i].setValue (rand () %   100);
			bt.bTreePlusInsert (keys[i], values[i]);
		}

		visualize_bt (bt);

		for (int i = 0; i < n; i++)
			bt.bTreePlusDelete (keys[i]);

		visualize_bt (bt);

		delete[] keys;
		delete[] values;
	}

	return 0;
}
#endif // ! __PROGTEST__

