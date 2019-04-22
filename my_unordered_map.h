#ifndef _MY_UNORDERED_MAP_
#define _MY_UNORDERED_MAP_


#include "my_alloc.h"
#include <stddef.h>  
#include "my_algo.h" 
#include "my_construct.h" 
#include "my_uninitialized.h"
#include "my_iterator.h"
#include "my_functors.h"
#include "my_hash_table.h"

namespace fyj
{

template <class Key, 
		  class T,
		  class HashFun = hash<Key>,
		  class EqualKey = equal_to<Key>,
		  class Alloc = alloc>
class unordered_map 
{

private:
	typedef hashtable<pair<const Key, T>, Key, HashFun, 
					  select1st<pair<const Key, T>>,
					  EqualKey, Alloc> ht;
    ht rep;

public:
	typedef typename ht::key_type key_type;
	typedef typename ht::T data_type;
	typedef typename ht::T mapped_type;
	typedef typename ht::value_type value_type;
	typedef typename ht::hasher hasher;
	typedef typename ht::key_equal key_equal;
	typedef typename ht::size_type size_type;

	typedef typename ht::difference_type difference_type;
	typedef typename ht::pointer pointer;
	typedef typename ht::const_pointer const_pointer;
	typedef typename ht::reference reference;
	typedef typename ht::const_reference const_reference;
	typedef typename ht::iterator iterator;
	typedef typename ht::const_iterator const_iterator;

	hasher hash_fun() const {return rep.hash_fun();}
	key_equal key_eq() const {return rep.key_eq();}

public:
	unordered_map(): rep(100, hasher(), key_equal()) {}
	explicit unordered_map(size_type n)
				: rep(n, hasher(), key_equal()) {}
	unordered_map(size_type n, const hasher& hf)
	  			: rep(n, hf, key_equal()) {}
	unordered_map(size_type n, const hasher& hf, 
				  const key_equal& keq)
				: rep(n, hf, keq) {}

	template <class InputIterator>
	unordered_map(InputIterator first, InputIterator last)
		: rep(100, hasher(), key_equal())
		{rep.insert_unique(first, last);}

	template <class InputIterator>
	unordered_map(InputIterator first, InputIterator last,
				  size_type n)
		: rep(n, hasher(), key_equal())
		{rep.insert_unique(first, last);}

	template <class InputIterator>
	unordered_map(InputIterator first, InputIterator last,
				  size_type n, const hasher& hf)
		: rep(n, hf, key_equal())
		{rep.insert_unique(first, last);}

	template <class InputIterator>
	unordered_map(InputIterator first, InputIterator last,
				  size_type n, const hasher& hf,
				  const key_equal& keq)
		: rep(n, hf, keq)
		{rep.insert_unique(first, last);}

public:
	size_type size() const {return rep.size();}
	size_type max_size() const {return rep.max_size();}
	bool empty() const {return rep.empty();}

	void swap(unordered_map& s) {rep.swap(s.rep);}

	friend bool operator==(const unordered_map& m1,
						   const unordered_map& m2)
	{return m1.rep == m2.rep; }

	iterator begin() const {return rep.begin();}
	iterator end() const {return rep.end();}

public:
	pair<iterator, bool> insert(const value_type& obj)
	{
		pair<typename ht::iterator, bool> p = rep.insert_unique(obj);
		return pair<iterator, bool>(p.first, p.second);		
	}

	template <class InputIterator>
	void insert(InputIterator first, InputIterator last)
	{rep.insert_unique(first, last);}

	pair<iterator, bool> insert_noresize(const value_type& obj)
	{
		pair<typename ht::iterator, bool> p =
		    rep.insert_unique_noresize(obj);
		return pair<iterator, bool>(p.first, p.second);
	}

	iterator find(const key_type& key) const 
	{return rep.find(key); }

	T& operator[] (const key_type& key)
	{
		return rep.find_or_insert(value_type(key, T())).second;
	}

	size_type count(const key_type& key) const
	{return rep.count(key);}

public:
	void resize(size_type hint) {rep.resize(hint);}
	size_type bucket_count() const 
	{return rep.bucket_count();}
	size_type max_bucket_count() const 
	{return rep.max_bucket_count();}

};		


}

#endif