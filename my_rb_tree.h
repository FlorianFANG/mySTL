/**
 * RB_tree (kind of Binary Search Tree) such that:
 * 		1). each node is either red or black;
 * 		2). root node is always black
 * 		3). red node's child must be black
 * 		4). number of black node of each path (from root to leaf) must be same
 * By 4) ==> new node must be red; and by 3) ==> parent of new node must be black
 * RB_tree only used for implementation of map and set, not for outside
 */ 

#ifndef _MY_RB_TREE_
#define _MY_RB_TREE_

#include "my_alloc.h"
#include <stddef.h>  
#include "my_algo.h" 
#include "my_construct.h" 
#include "my_uninitialized.h"
#include "my_iterator.h"


namespace fyj
{

typedef bool __rb_tree_color_type;
const __rb_tree_color_type __rb_tree_red   = false;  // red = 0
const __rb_tree_color_type __rb_tree_black = true;   // black = 1

struct __rb_tree_node_base
{
	typedef __rb_tree_color_type color_type;
	typedef __rb_tree_node_base* base_ptr;

	color_type color;
	base_ptr parent;
	base_ptr left;  //point to left child
	base_ptr right; //point to right child

	static base_ptr minNode(base_ptr root)
	{
		while(root)
			root = root->left;
		return root;
	}

	static base_ptr maxNode(base_ptr root)
	{
		while(root)
			root = root->right;
		return root;
	}
};

template <class T>
struct __rb_tree_node : public __rb_tree_node_base
{
	typedef __rb_tree_node<T>* link_type;
	T value;
};

struct __rb_tree_iterator_base
{
	typedef __rb_tree_node_base::base_ptr base_ptr;
	typedef bidirectional_iterator_tag iterator_category;
	typedef ptrdiff_t difference_type;

	base_ptr node;

	// Find the next node (minimun of all nodes that larger than this node)
	// For operator++
	void increment()
	{
		// Case 1: right sub-tree not null and ...
		//         ...next node in the most left of the right sub-tree
		if(node->right)
		{
			node = node->right;
			while(node->left)
				node = node->left;
		}
		// Case 2: right sub-tree is null
		else
		{
			base_ptr p = node->parent;
			while(node == p->right)
			{
				node = p;
				p = p->parent;
			}
			if(node->right != p)
				node = p;
		}
	}

	// Find the prev node (max of all nodes that less than this node)
	// For operator--
	void decrement()
	{
		// Case 1: the node is the head of the tree
		if (node->color == __rb_tree_red && 
			node->parent->parent == node)
			node = node->right;
		// Case 2: the node has left child
		else if (node->left)
		{
			base_ptr l = node->left;
			while(l->right)
				l = l->right;
			node = l;
		}
		// Case 3
		else
		{
			base_ptr p = p->parent;
			while(node == p->left)
			{
				node = p;
				p = p->parent;
			}
			node = p;
		}
	}
};

template <class T, class Ref,class Pointer>
struct __rb_tree_iterator : public __rb_tree_iterator_base
{
	typedef T 					value_type;
	typedef Ref   				reference;
	typedef Pointer 		  	pointer;
	typedef __rb_tree_iterator<T, T&, T*>  iterator;
	typedef __rb_tree_iterator<const T, const T&, const T*>  const_iterator;
	typedef __rb_tree_iterator<T, Ref, Pointer> self;
	typedef __rb_tree_node<T>* link_type;

	__rb_tree_iterator(){}
	__rb_tree_iterator(link_type x) {node = x;}
	__rb_tree_iterator(const iterator& it){node = it.node;}

	reference operator*() const { return link_type(node)->value; }
	pointer operator->() const { return &(operator*()); }

	self& operator++()
	{
		increment();
		return *this;
	}
	self operator++(int)
	{
		self temp = *this;
		increment();
		return temp;
	}

	self& operator--()
	{
		decrement();
		return *this;
	}
	self operator--(int)
	{
		self temp = *this;
		decrement();
		return temp;
	} 

};

// Each node of RB_tree has a Key-Value pair
// The tree is ordered by Key, which is determined by Compare
template <class Key, class Value, class KeyOfValue, class Compare,
		  class Alloc = alloc>
class rb_tree
{
protected:
	typedef void* 					  		  void_pointer;
	typedef __rb_tree_node_base*  			  base_ptr;
	typedef __rb_tree_node<Value> 			  rb_tree_node;
	typedef simple_alloc<rb_tree_node, Alloc> rb_tree_node_allocator;
	typedef __rb_tree_color_type   			  color_type;

public:
	typedef Key                   key_type;
	typedef Value                 value_type;
	typedef value_type* 		  pointer;
	typedef const value_type* 	  const_pointer;
	typedef value_type& 	      reference;
	typedef const value_type&     const_reference;
	typedef rb_tree_node*         link_type;
	typedef size_t                size_type;
	typedef ptrdiff_t             difference_type;

protected:
	link_type get_node() { return rb_tree_node_allocator::allocate(); }
	void put_node(link_type p){ rb_tree_node_allocator::deallocate(p);}

	link_type create_node(const value_type& x)
	{
		link_type temp = get_node();
		construct(&temp->value, x);
		return temp;
	}

	link_type clone_node(link_type x)
	{
		link_type temp = create_node(x->value);
		temp->color = x->color;
		temp->left = 0;
		temp->right = 0;
		return temp;
	}

	void destroy_node(link_type p)
	{
		destroy(&p->value);
		put_node(p);
	}

protected:
	size_type node_count;

	// A trick: header
	// header is the parent of the root, and header's parent is also root
	// header's left is the left most node and ...
	//       ...right is the right most node
	link_type header;
	Compare key_compare;

	link_type& root() const {return (link_type&)header->parent;}
	link_type& leftmost() const {return (link_type&)header->left;}
	link_type& rightmost() const {return (link_type&)header->right;}

	static link_type& left(link_type x){return (link_type&)(x->left);}
	static link_type& right(link_type x){return (link_type&)(x->right);}
	static link_type& parent(link_type x){return (link_type&)(x->parent);}
	static reference value(link_type x){return x->value;}
	static const Key& key(link_type x){return KeyOfValue()(value(x));}
	static color_type& color(link_type x){return (color_type&)(x->color);}

	// base_ptr = __rb_tree_node_base*
	// link_ptr = __rb_tree_node*
	static link_type& left(base_ptr x){return (link_type&)(x->left);}
	static link_type& right(base_ptr x){return (link_type&)(x->right);}
	static link_type& parent(base_ptr x){return (link_type&)(x->parent);}
	static reference value(base_ptr x){return ((link_type)x)->value;}
	static const Key& key(base_ptr x){return KeyOfValue()(value(x));}
	static color_type& color(base_ptr x){return (color_type&)(x->color);}

	static link_type minNode(link_type x)
	{
		return (link_type)__rb_tree_node_base::minNode(x);
	}
	static link_type maxNode(link_type x)
	{
		return (link_type)__rb_tree_node_base::maxNode(x);
	}

public:
	typedef __rb_tree_iterator<value_type, reference, pointer> iterator;

private:
	// left rotate
	void __rb_tree_rotate_left(base_ptr x, base_ptr& root)
	{
		base_ptr y = x->right;
		x->right = y->left;
		if(y->left)
			y->left->parent = x;
		y->parent = x->parent;

		if(x == root)
			root = y;
		else if(x == x->parent->left)
			x->parent->left = y;
		else
			x->parent->right = y;
		y->left = x;
		x->parent = y;
	}

	// right rotate
	void __rb_tree_rotate_right(base_ptr x, base_ptr& root)
	{
		base_ptr y = x->left;
		x->left = y->right;
		if(y->right)
			y->right->parent = x;
		y->parent = x->parent;

		if(x == root)
			root = y;
		else if(x == x->parent->right)
			x->parent->right = y;
		else
			x->parent->left = y;
		y->right = x;
		x->parent = y;
	}


	// Rebalance the BST 
	// Used when insert new node x to the tree
	void __rb_tree_rebalance(base_ptr x, base_ptr& root)
	{
		// new node x must be red
		x->color = __rb_tree_red;

		// parent of new node x must be black
		while(x != root && x->parent->color == __rb_tree_red)
		{
			if(x->parent == x->parent->parent->left)
			{
				// y is uncle of x
				base_ptr y = x->parent->parent->right;
				if(y && y->color == __rb_tree_red)
				{
					// parent and uncle must both be black
					x->parent->color = __rb_tree_black;
					y->color = __rb_tree_black;
					// then grand parent should be red
					x->parent->parent->color = __rb_tree_red;
					x = x->parent->parent;
				}
				else
				{
					if(x == x->parent->right)
					{
						x = x->parent;
						__rb_tree_rotate_left(x, root);
					}
					x->parent->color = __rb_tree_black;
					x->parent->parent->color = __rb_tree_red;
					__rb_tree_rotate_right(x->parent->parent, root);
				}
			}
			else
			{
				// y is uncle of x
				base_ptr y = x->parent->parent->left;
				if(y && y->color == __rb_tree_red)
				{
					// parent and uncle must both be black
					x->parent->color = __rb_tree_black;
					y->color = __rb_tree_black;
					// then grand parent should be red
					x->parent->parent->color = __rb_tree_red;
					x = x->parent->parent;
				}
				else
				{
					if(x == x->parent->left)
					{
						x = x->parent;
						__rb_tree_rotate_right(x, root);
					}
					x->parent->color = __rb_tree_black;
					x->parent->parent->color = __rb_tree_red;
					__rb_tree_rotate_left(x->parent->parent, root);
				}
			}
		}
		root->color = __rb_tree_black;
	}

	// insert the node with value v, to the place of x ...
	//        ... whose parent is y
	// Used for insert_unique and insert_equal
	iterator __insert(base_ptr x_, base_ptr y_, const value_type& v)
	{
		link_type x = (link_type)x_;
		link_type y = (link_type)y_;
		link_type z;
		z = create_node(v);

		if(y == header || x || key_compare(KeyOfValue()(v),key(y)))
		{
			left(y) = z; // leftmost is z
			if(y == header)
			{
				root() = z;
				rightmost() = z;
			}
			else if(y == leftmost())
				leftmost() = z;
		}
		else
		{
			right(y) = z;
			if(y == rightmost())
				rightmost() = z;
		}

		parent(z) = y;
		left(z) = 0;
		right(z) = 0;

		__rb_tree_rebalance(z, header->parent);
		++node_count;
		return iterator(z);
	}

	link_type __copy(link_type x, link_type p);

	void __erase(link_type x);

	void init()
	{
		header = get_node();
		color(header) = __rb_tree_red;
		root() = 0;
		leftmost() = header;
		rightmost() = header;
	}

public:
	rb_tree(const Compare& comp = Compare())
		: node_count(0), key_compare(comp) {init();}
	// ~rb_tree()
	// {
	// 	clear();
	// 	put_node(header);
	// }
	rb_tree<Key, Value, KeyOfValue, Compare, Alloc>&
		operator=(const rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& x);

	Compare key_com() const {return key_compare;}
	iterator begin() const {return leftmost();}
	iterator end() const {return header;}
	bool empty() const {return node_count == 0;}
	size_type size() const {return node_count;}
	size_type max_size() const {return size_type(-1);}
	void swap();

	// insert so that any node is unique
	// used for map / set
	pair<iterator, bool> insert_unique(const value_type& v)
	{
		link_type y = header;
		link_type x = root();
		bool comp = true;
		while(x)
		{
			y = x;
			comp = key_compare(KeyOfValue()(v), key(x));
			x = comp ? left(x) : right(x);
		}
		// after the loop, x = null where v to be inserted and ...
		//              ...y is the leap, parent of x (or say v)
		
		iterator j = iterator(y);

		if (comp)
			if(j == begin())
				return pair<iterator, bool>(__insert(x, y, v), true);
			else
				--j;
		if(key_compare(key(j.node), KeyOfValue()(v)))
			return pair<iterator, bool>(__insert(x, y, v), true);

		return pair<iterator, bool>(j, false);

	}

	// insert and allow duplicate node
	// used for multimap / multiset
	iterator insert_equal(const value_type& v)
	{
		link_type y = header;
		link_type x = root();
		while(x)
		{
			y = x;
			x = key_compare(KeyOfValue()(v), key(x)) ? left(x) : right(x);
		}
		// after the loop, x = null where v to be inserted and ...
		//              ...y is the leap, parent of x (or say v)
		return __insert(x, y, v);
	}

	// find
	iterator find(const Key& k)
	{
		link_type y = header;
		link_type x = root();

		while(x)
		{
			//if x > k
			if(!key_compare(key(x), k))
			{
				y = x;
				x = left(x);
			}
			else
				x = right;
		}
		iterator j = iterator(y);
		return (j == end() || key_compare(k, key(j.node))) ? end() : j;
	}
};

}// end of namespace



#endif