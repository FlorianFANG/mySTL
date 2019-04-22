/* Double linked list.
 *
 * Each list node has two pointers: p_next, p_prev
 * 		- p_next points to the p_prev of the next node
 *  	- p_prev points to the p_prev of the previous node
 *
 * An empty node is placed between list head and tail, so form a double ll
 */

#ifndef _MY_LIST_
#define _MY_LIST_

#include <stddef.h>
#include "my_alloc.h"  
#include "my_algo.h" 
#include "my_construct.h" 
#include "my_uninitialized.h"
#include "my_iterator.h"

namespace fyj
{

// List Node: the struct is different from the List itself
template <class T>
struct __list_node
{
	__list_node* prev;
	__list_node* next;
	T data;
};

// Iterator: BidrectionalIterator
template <class T>
struct __list_iterator
{
	typedef __list_iterator<T> 		    iterator;
	typedef __list_iterator<T> 			self;

	typedef bidirectional_iterator_tag 	iterator_category;
	typedef T 							value_type;
	typedef T*						    pointer;
	typedef T& 							reference;
	typedef __list_node<T>* 			link_type;
	typedef size_t  					size_type;
	typedef ptrdiff_t  		  			difference_type;		

	// iterator as normal pointer, pointing at list_node
	link_type node;

	// constructor
	__list_iterator(link_type x) : node(x) {}
	__list_iterator() {}
	__list_iterator(const iterator& x) : node(x.node) {}

	//overload operator
	bool operator==(const self& x) const {return node == x.node;}
	bool operator!=(const self& x) const {return node != x.node;}
	reference operator*() const {return (*node).data;}
	pointer operator->() const {return &(operator*());}

	// "++i"
	self& operator++()
	{
		node = (link_type)((*node).next);
		return *this;
	}
	// "i++"
	self& operator++(int)
	{
		self temp = *this;
		++*this;
		return temp;
	}
	// "--i"
	self& operator--()
	{
		node = (link_type)((*node).prev);
		return *this;
	}
	// "i--"
	self& operator--(int)
	{
		self temp = *this;
		--*this;
		return temp;
	}
};

template <class T, class Alloc = alloc>
class list
{
protected:
	typedef __list_node<T> list_node;
	typedef size_t size_type;
	typedef T& reference;
	typedef list_node* link_type;

	typedef simple_alloc<list_node, Alloc> list_node_allocator;

public:
	typedef __list_iterator<T> iterator;
	

protected:
	link_type node;

	link_type get_node()
	{
		return list_node_allocator::allocate();
	}
	void put_node(link_type p)
	{
		list_node_allocator::deallocate(p);
	}
	link_type create_node(const T& x)
	{
		link_type p = get_node();   // allocate
		construct(&p->data, x);     // construct
		return p;
	}
	void destroy_node(link_type p)
	{
		destroy(&p->data);          // destruct
		put_node(p);                // deallocate
	}
	void empty_initialize() // for constructor list()
	{
		node = get_node();
		node->next = node;
		node->prev = node;
	}
	void insert(iterator position, const T& x) //for push_back(),push_front()
	{
		link_type temp = create_node(x);
		temp->next = position.node;
		temp->prev = position.node->prev;
		(link_type(position.node->prev))->next = temp;
		position.node->prev = temp;
	}
	// transfer the [first, last) range to the front of position
	// used for splice(), sort(), merge(), reverse()
	void transfer(iterator position, iterator first, iterator last)
	{
		if(position != last)
		{
			(*(link_type((*last.node).prev))).next = position.node;
			(*(link_type((*first.node).prev))).next = last.node;
			(*(link_type((*position.node).prev))).next = first.node;
			link_type temp = link_type((*position.node).prev);
			(*position.node).prev = (*last.node).prev;
			(*last.node).prev = (*first.node).prev;
			(*first.node).prev = temp;
		}
	}

public:
	list()
	{
		empty_initialize();
	}
	iterator begin()
	{
		return (link_type)((*node).next);
	}
	iterator end()
	{
		return node;
	}
	bool empty() const
	{
		return node->next == node;
	}
	size_type size() const
	{
		size_type result = 0;
		distance(begin(), end(), result);
		return result;
	}
	reference front()
	{
		return *begin();
	}
	reference back()
	{
		return *(--end());
	}
	void push_back(const T& x)
	{
		insert(end(), x);
	}
	void push_front(const T& x)
	{
		insert(begin(), x);
	}
	void erase(iterator position)
	{
		link_type next_node = link_type(position.node->next);
		link_type prev_node = link_type(position.node->prev);
		prev_node->next = next_node;
		next_node->prev = prev_node;
		destroy(position.node);
		//return iterator(next_node);
	}
	void pop_front()
	{
		erase(begin());
	}
	void pop_back()
	{
		iterator temp = end();
		erase(--temp);
	}
	void clear()
	{
		link_type curr = (link_type)node->next; // beginning node
		while (curr != node)
		{
			link_type temp = curr;
			curr = (link_type)curr->next;
			destroy(temp);
		}
		node->next = node;
		node->prev = node;
	}
	void remove(const T& x)
	{
		iterator first = begin();
		iterator last = end();
		while(first != last)
		{
			iterator next = first;
			++next;
			if(*first == x)
				erase(first);
			first = next;
		}
	}
	// if there are continuous same element, erase the duplicates
	void unique() 
	{
		iterator first = begin();
		iterator last = end();
		if (first == last) return;
		iterator next = first;
		while(++next != last)
		{
			if(*first == *next)
				erase(next);
			else
				first = next;
			next = first;
		}
	}
	void splice(iterator position, list& x)
	{
		if(!x.empty())
			transfer(position, x.begin(), x.end());
	}
	void splice(iterator position, iterator i)
	{
		iterator j = i;
		++j;
		if(position == i || position == j)
			return;
		transfer(position, i, j);
	}
	void splice(iterator position, iterator first, iterator last)
	{
		if(first != last)
			transfer(position, first, last);
	}
	// Merge the list x to this list
	// Pre-conditon: both lists are sorted (increase order)
	void merge(list<T, Alloc>& x)
	{
		iterator first1 = begin();
		iterator last1 = end();
		iterator first2= x.begin();
		iterator last2 = x.end();

		while(first1 != last1 && first2 != last2)
		{
			if(*first2 < *first1)
			{
				iterator next = first2;
				transfer(first1, first2, ++next);
				first2 = next;
			}
			else
				++first1;
		}

		if(first2 != last2) 
			transfer(last1, first2, last2);
	}
	void reverse()
	{
		//if empty list or only one node, return
		if(node->next == node || link_type(node->next)->next == node)
			return;
		iterator first = begin();
		++first;
		while(first != end())
		{
			iterator old = first;
			++first;
			transfer(begin(), old, first);
		}
	}

	// Sort: using quick sort algo
	// sort() in "my_algo.h" is not applicable for linked list...
	// ...because sort() accept only RandomAccessIterator
	void sort()
	{
		//if empty list or only one node, return
		if(node->next == node || link_type(node->next)->next == node)
			return;
		list<T, Alloc> carry;
		list<T, Alloc> counter[64];
		int fill = 0;
		while(!empty())
		{
			carry.splice(carry.begin(), *this, begin());
			int i = 0;
			while(i < fill && !counter[i].empty())
			{
				counter[i].merge(carry);
				carry.swap(counter[i++]);
			}
			carry.swap(counter[i]);
			if(i == fill)
				++fill;
		}	
		for(int i = 1; i < fill; ++i)
			counter[i].merge(counter[i-1]);
		swap(counter[fill-1]);
	}

};



} //end of namespace


#endif