/**
 * Set data structure where elements are unique and ordered by Key
 * Set is implemented by RB_tree
 */

#ifndef _MY_SET_
#define _MY_SET_

#include "my_alloc.h"
#include <stddef.h>  
#include "my_algo.h" 
#include "my_construct.h" 
#include "my_uninitialized.h"
#include "my_iterator.h"
#include "my_functors.h"
#include "my_rb_tree.h"

namespace fyj
{

template <class Key, class Compare = less<Key>, class Alloc = alloc>
class set
{
public:
	typedef Key 	key_type;
	typedef Key 	value_type;
	typedef Compare key_compare;
	typedef Compare value_compare;

private:
	typedef rb_tree<key_type, value_type, identity<value_type>,
					key_compare, Alloc> tree_type;
	tree_type t;

public:
	// The key is not allowed to be modified in a set, so "const"
	typedef typename tree_type::const_pointer     pointer;
	typedef typename tree_type::const_pointer 	  const_pointer;
	typedef typename tree_type::const_reference   reference;
	typedef typename tree_type::const_reference   const_reference;
	typedef typename tree_type::const_iterator    iterator;
	typedef typename tree_type::const_iterator    const_iterator;
	typedef typename tree_type::const_reverse_iterator    reverse_iterator;
	typedef typename tree_type::const_reverse_iterator    const_reverse_iterator;
	typedef typename tree_type::size_type 		   size_type;
	typedef typename tree_type::difference_type    difference_type;

	set() : t(Compare()) {}
	explicit set(const Compare& comp) : t(comp){}

	// template <class InputIterator>
	// set(InputIterator first, InputIterator last)
	// 	: t(Compare()) {t.insert_unique(first, last);}
	// template <class InputIterator>
	// set(InputIterator first, InputIterator last, const Compare& comp)
	// 	: t(comp) {t.insert_unique(first, last);}

	set(const set<Key, Compare, Alloc>& x) : t(x.t) {}

	set<Key, Compare, Alloc>& operator=(const set<Key, Compare, Alloc>& x)
	{
		t = x.t;
		return *this;
	}

	key_compare key_comp() const {return t.key_comp;}
	value_compare value_comp() const {return t.key_comp;}
	iterator begin() const {return t.begin();}
	iterator end() const {return t.end();}
	bool empty() const {return t.empty();}
	size_type size() const {return t.size();}
	size_type max_size() const {return t.max_size();}
	void swap(set<Key, Compare, Alloc>& x) {t.swap(x.t);}

	pair<iterator, bool> insert(const value_type& x)
	{
		pair<typename tree_type::iterator, bool> p = t.insert_unique(x);
		return pair<iterator, bool>(p.first, p.second);
	}

	void clear() {t.clear();}

	iterator find(const key_type& x) const {return t.find();}
	//size_type count(const key_type& x) const {return t.count(x);}

	friend bool operator==(const set<Key, Compare, Alloc>& x,
						   const set<Key, Compare, Alloc>& y,)
	{
		return x.t == y.t;
	}

	friend bool operator< (const set<Key, Compare, Alloc>& x,
						   const set<Key, Compare, Alloc>& y,)
	{
		return x.t < y.t;
	}
};


}


#endif