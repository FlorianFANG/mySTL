/* Hash_table used for unordered_map / set
 * 
 * To solve the hashing collision, here use separate chaining design: 
 * A hash table is a vector of buckets, each bucket is a list
 * When looking for an element:
 *     # first look which bucket number (bkt_num) it store
 *     # then look the position of the list of the bucket
 */

#ifndef _MY_HASH_TABLE_
#define _MY_HASH_TABLE_

#include "my_alloc.h"
#include <stddef.h>  
#include "my_algo.h" 
#include "my_construct.h" 
#include "my_uninitialized.h"
#include "my_iterator.h"
#include "my_functors.h"
#include "my_vector.h"
#include "my_pair.h"

namespace fyj
{

template <class Value>
struct __hashtable_node
{
	__hashtable_node* next;
	Value val;
};

template <class Value, class Key, class HashFun,
  		  class ExtractKey, class EqualKey, class Alloc>
struct __hashtable_iterator
{
	typedef hashtable<Value, Key, HashFun, 
					  ExtractKey, EqualKey, Alloc> hashtable;
    typedef __hashtable_iterator<Value, Key, HashFun, ExtractKey,
    					EqualKey, Alloc> iterator;
    typedef __hashtable_const_iterator<Value, Key, HashFun, ExtractKey,
    					EqualKey, Alloc> const_iterator;   	
    typedef __hashtable_node<Value> node;

    typedef forward_iterator_list iterator_category;
    typedef Value value_type;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef Value& reference;
    typedef Value* pointer;

    node* curr;
    hashtable* ht;

    __hashtable_iterator(){}
    __hashtable_iterator(node* n, hashtable* tab)
    	: node(n), ht(tab) {}

    reference operator*() const {return cur->val;}
    pointer operator->() const {return &(operator*());}

    iterator& operator++()
    {
    	const node* old = cur;
    	cur = cur->next;
    	if(!cur)
    	{
    		size_type bucket = ht->bkt_num(old->val);
    		while(!cur && ++bucket < ht->buckets.size())
    			cur = ht->buckets[bucket];
    	}
    	return *this;
    }    								

    iterator operator++(int)
    {
    	iterator temp = *this;
    	++*this; // operator++()
    	return temp;
    }						  

    bool operator==(const iterator& it) const
    {
    	return it.cur == cur;
    }
    bool operator!=(const iterator& it) const
    {
    	return it.cur != cur;
    }
};  

static const int __num_primes = 28;

// Number of buckets is one of the prime number in the prime list
static const unsigned long __prime_list[__num_primes] =
{
  53ul,         97ul,         193ul,       389ul,       769ul,
  1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
  49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
  1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
  50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul, 
  1610612741ul, 3221225473ul, 4294967291ul
};

unsigned long __next_prime(unsigned long n)
{
	const unsigned long* first = __prime_list;
	const unsigned long* last = first + __num_primes;
	const unsigned long* pos = lower_bound(first, last, n);
	return pos == last ? *(last - 1) : pos;
}

// @Value: used for map; for set, value = key
// @Key: used for map and set
// @HashFun: input_value -> hash_value defined in "my_functors.h"
// @ExtractKey: extract the key of a given value
// @EqualKey: see if two keys are equal
template <class Value, class Key, class HashFun, class ExtractKey,
		  class EqualKey, class Alloc = alloc>
class hashtable
{
public:
	typedef Value value_type;
	typedef Key key_type;
	typedef HashFun hasher;
	typedef EqualKey key_equal;
	typedef size_t size_type;

private:
	hasher hash;
	key_equal equals;
	ExtractKey get_key;

	typedef __hashtable_node<Value> node;
	typedef simple_alloc<node, Alloc> node_allocator;

	vector<node*, Alloc> buckets;
	size_type num_elements;

	node* new_node(const value_type& obj)
	{
		node* n = node_allocator::allocate();
		n->next = 0;
		construct(&n->val, obj);
		return n;
	}

	void delete_node(node* n)
	{
		destroy(&n->val);
		node_allocator::deallocate();
	}

	size_type next_size(size_type n) const
	{return __next_prime; }

	//================= BKT_NUM ===============================
	// v0
	size_type bkt_num_key(const key_type& key, size_t n) const
	{
		return hash(key) % n; // hash defined in "my_functors.h"
	}
	// v1: param are value and num of buckets
	size_type bkt_num(const value_type& obj, size_t n) const
	{
		return bkt_num_key(get_key(obj),n);//v0
	}
	// v2: param is value only
	size_type bkt_num(const value_type& obj) const
	{
		return bkt_num_key(get_key(obj)); //v0
	}
	// v3: param is key only
	size_type bkt_num_key(const key_type& key) const
	{
		return bkt_num_key(key, buckets.size()); //v0
	}




public:
	//================== CONSTRUCTOR ============================
	void initialize_buckets(size_type n)
	{
		const size_type n_buckets = next_size(n);
		buckets.reserve(n_buckets);
		buckets.insert(buckets.end(), n_buckets, (node*)0);
		num_elements = 0;
	}

	hashtable(size_type n, const HashFun& hf, const EqualKey& eql)
		: hash(hf), equals(eql), get_key(ExtractKey()), 
		  num_elements(0)
	{
		initialize_buckets(n);
	}

	//=================== COUNT ================================
	size_type bucket_count() const {return buckets.size();}
	size_type max_bucket_count() const 
	{return __prime_list[__num_primes - 1]; }
	size_type size() const {return num_elements;}
	size_type max_size() const {return 2*max_bucket_count();}
	bool empty() const {return num_elements == 0;}


	//=================== INSERT_UNIQUE ==========================
	void resize(size_type num_elements_hint)
	{
		const size_type old_n = buckets.size();

		if(num_elements_hint > old_n)
		{
			const size_type n = next_size(num_elements_hint);
			if(n > old_n)
			{
				vector<node*, Alloc> temp(n, (node*)0);
				for(size_type bucket = 0; bucket < old_n;
					++bucket)
				{
					node* first = buckets[bucket];
					while(first)
					{
						size_type new_bucket = bkt_num(first->val, n);
						buckets[bucket] = first->next;
						first->next = temp[new_bucket];
						temp[new_bucket] = first;
						first = buckets[bucket];
					}
				}
				buckets.swap(temp);
			}
		}
	}

	pair<iterator, bool> insert_unique_noresize(const value_type& obj)
	{
		const size_type n = bkt_num(obj);
		node* first = buckets[n];

		for(node* cur = first; cur; cur = cur->next)
			if(equals(get_key(cur->val), get_key(obj)))
				return pair<iterator, bool>(iterator(cur, this),
					    					false);
		node* temp = new_node(obj);
		temp->next = first;
		buckets[n] = temp;
		++num_elements;
		return pair<iterator, bool>(iterator(temp, this), true);
	}

	pair<iterator, bool> insert_unique(const value_type& obj)
	{
		resize(num_elements + 1);
		return insert_unique_noresize(obj);
	}

	//=================== INSERT_EQUAL =============================
	iterator insert_equal_noresize(const value_type& obj)
	{
		const size_type n = bkt_num(obj);
		node* first = buckets[n];

		for(node* cur = first; cur; cur = cur->next)
			if(equals(get_key(cur->val), get_key(obj)))
			{
				node* temp = new_node(obj);
				temp->next = cur->next;
				cur->next = temp;
				++num_elements;
				return iterator(temp, this);
			}

		node* temp = new_node(obj);
		temp->next = first;
		buckets[n] = temp;
		++num_elements;
		return iterator(temp, this);
	}

	iterator insert_equal(const value_type& obj)
	{
		resize(num_elements + 1);
		return insert_equal_noresize(obj);
	}

	//===================== CLEAR & COPY ============================
	void clear()
	{
		for(size_type i = 0; i < buckets.size(); ++i)
		{
			node* cur = buckets[i];
			while(cur)
			{
				node* next = cur->next;
				delete_node(cur);
				cur = next;
			}
			buckets[i] = 0;
		}
		num_elements = 0;
	}	

	void copy_from(const hashtable& ht)
	{
		buckets.clear(); // vector::clear()
		buckets.reserve(ht.buckets.size());
		buckets.insert(buckets.end(), ht.buckets.size(),
					   (node*)0);
		for(size_type i = 0; i < ht.buckets.size(); ++i)
		{
			if(const node* cur = ht.buckets[i])
			{
				node* copy = new_node(cur->val);
				buckets[i] = copy;

				for(node* next = cur->next; next; 
					cur = next, next = cur->next)
				{
					copy->next = new_node(next->val);
					copy = copy->next;
				}
			}
		}
		num_elements = ht.num_elements;
	}
};	  	   


} // end of namespace


#endif