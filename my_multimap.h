/**
 * Map data structure where elements are unique and ordered by Key
 * Map is implemented by RB_tree
 */

#ifndef _MY_MULTIMAP_
#define _MY_MULTIMAP_

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

template <class Key, class T, class Compare = less<Key>, class Alloc = alloc>
class multimap
{
public:
	typedef Key 			   key_type;
	typedef T 				   data_type;
	typedef T 				   mapped_type;
	typedef pair<const Key, T> value_type;
	typedef Compare 		   key_compare;

	class value_compare : public binary_functor<value_type, value_type, bool>
	{
	friend class map<Key, T, Compare, Alloc>;
	protected:
		Compare comp;
		value_compare(Compare c) : comp(c){}
	public:
		bool operator()(const value_type& x, const value_type& y) const
		{return comp(x.first, y.first);}
	};

private:
	typedef rb_tree<key_type, value_type, select1st<value_type>,
					key_compare, Alloc> tree_type;
	tree_type t;

public:
	typedef typename tree_type::pointer             pointer;
	typedef typename tree_type::const_pointer 	    const_pointer;
	typedef typename tree_type::reference           reference;
	typedef typename tree_type::const_reference     const_reference;
	typedef typename tree_type::iterator            iterator;
	typedef typename tree_type::const_iterator      const_iterator;
	typedef typename tree_type::reverse_iterator    reverse_iterator;
	typedef typename tree_type::const_reverse_iterator    const_reverse_iterator;
	typedef typename tree_type::size_type 		     size_type;
	typedef typename tree_type::difference_type      difference_type;

	multimap() : t(Compare()) {}
	explicit multimap(const Compare& comp) : t(comp){}

	// template <class InputIterator>
	// map(InputIterator first, InputIterator last)
	// 	: t(Compare()) {t.insert_unique(first, last);}
	// template <class InputIterator>
	// map(InputIterator first, InputIterator last, const Compare& comp)
	// 	: t(comp) {t.insert_unique(first, last);}

	multimap(const multimap<Key, Compare, Alloc>& x) : t(x.t) {}

	multimap<Key, Compare, Alloc>& operator=(const multimap<Key, Compare, Alloc>& x)
	{
		t = x.t;
		return *this;
	}

	key_compare key_comp() const {return t.key_comp;}
	value_compare value_comp() const {return value_compare(t.key_comp());}
	iterator begin() const {return t.begin();}
	iterator end() const {return t.end();}
	bool empty() const {return t.empty();}
	size_type size() const {return t.size();}
	size_type max_size() const {return t.max_size();}

	// Subscript operator
	// Return by reference so that it could be lvalue or rvalue
	T& operator[] (const key_type& k)
	{
		return (*((insert(value_type(k, T()))).first)).second;
	}
	
	void swap(multimap<Key, Compare, Alloc>& x) {t.swap(x.t);}

	pair<iterator, bool> insert(const value_type& x)
	{
		return t.insert_equal(x);
	}

	void clear() {t.clear();}

	iterator find(const key_type& x) const {return t.find();}
	//size_type count(const key_type& x) const {return t.count(x);}

	friend bool operator==(const multimap<Key, T, Compare, Alloc>& x,
						   const multimap<Key, T, Compare, Alloc>& y,)
	{
		return x.t == y.t;
	}

	friend bool operator< (const multimap<Key, T, Compare, Alloc>& x,
						   const multimap<Key, T, Compare, Alloc>& y,)
	{
		return x.t < y.t;
	}
};

}

#endif