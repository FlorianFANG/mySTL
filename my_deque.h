/* Deque container
 * 
 *
 *
 */

#ifndef _MY_DEQUE_
#define _MY_DEQUE_

#include "my_alloc.h"
#include <stddef.h>  
#include "my_algo.h" 
#include "my_construct.h" 
#include "my_uninitialized.h"
#include "my_iterator.h"
#include "my_functors.h"
#include "my_vector.h"


namespace fyj
{

// @sz: size of element of type T
// @n : input by user
// if n != 0: there are n element in each buffer
// otherwise:
//   	if sz < 512: return 512/sz (num of element in each buffer)
//      otherwise  : return 1 (only 1 element in each buffer)
inline size_t __deque_buf_size(size_t n, size_t sz)
{
	return n != 0 ? n : (sz < 512 ? size_t(512 / sz) : size_t(1));
}

template <class T, size_t bufSize>
struct __deque_iterator
{
	typedef __deque_iterator<T, bufSize> iterator;
	typedef __deque_iterator<T, bufSize> const_iterator;
	static size_t buffer_size()
	{return __deque_buf_size(bufSize, sizeof(T));}

	typedef random_access_iterator_tag iterator_category;
	typedef T value_type;
	typedef T& reference;
	typedef T* pointer;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T** map_pointer;

	typedef __deque_iterator self;

	T* cur;
	T* first;
	T* last;
	map_pointer node;

	void set_node(map_pointer new_node)
	{
		node = new_node;
		first = *new_node;
		last = first + difference_type(buffer_size());
	}

	reference operator*() const {return *cur;}
	pointer operator->() const {return &(operator*());}
	difference_type operator-(const self& x) const
	{
		return difference_type(buffer_size()) * (node - x.node - 1)
			   + (cur - first) + (x.last - x.cur);
	}

	self& operator++()
	{
		++cur;
		if (cur == last)
		{
			set_node(node + 1);
			cur = first;
		}
		return *this;
	}

	self operator++(int)
	{
		self temp = *this;
		++*this;
		return temp;
	}

	self& operator--()
	{
		if(cur == first)
		{
			set_node(node - 1);
			cur = last;
		}
		--cur;
		return *this;
	}

	self operator--(int)
	{
		self temp = *this;
		--*this;
		return temp;
	}

	self& operator+=(difference_type n)
	{
		difference_type offset = n + (cur - first);

		//if in the same buffer
		if(offset >= 0 && offset < difference_type(buffer_size()))
			cur += n;
		//if in the difference buffer
		else
		{
			difference_type node_offset = 
				offset > 0 ? offset / difference_type(buffer_size())
							: -difference_type((-offset-1) / 
								buffer_size())-1;
			set_node(node + node_offset);
			cur = first + (offset - node_offset * 
						   difference_type(buffer_size()));
		}
		return *this;
	}

	self& operator-=(difference_type n)
	{return *this += -n;}

	self operator+(difference_type n) const
	{
		self temp = *this;
		return temp += n;
	}

	self operator-(difference_type n) const
	{
		self temp = *this;
		return temp -= n;
	}

	reference operator[](difference_type n) const
	{return *(*this + n);}

	bool operator==(const self& x) const {return cur == x.cur;}
	bool operator!=(const self& x) const {return !(*this == x);}
	bool operator<(const self& x) const
	{return (node == x.node) ? (cur < x.cur) : (node < x.node);}
}; 

// bufSize = 0 means 512 bytes as buffer
const size_t initial_map_size = 8;
template <class T, class Alloc = alloc, size_t bufSize = 0>
class deque
{
public:
	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

public:
	typedef __deque_iterator<T, bufSize> iterator;

protected:
	typedef pointer* map_pointer;

protected:
	// map is T**, pointing at T*, which points to buffer containing T
	map_pointer map;
	size_type map_size;

	iterator start;
	iterator finish;

protected:
	typedef simple_alloc<value_type, Alloc> data_allocator;
	typedef simple_alloc<pointer, Alloc> map_allocator;

	pointer allocate_node()
	{
		return data_allocator::allocate(iterator::buffer_size());
	}

	void create_map_and_node(size_type num_elements)
	{
		size_type num_nodes = num_elements / iterator::buffer_size() + 1;
		map_size = max(initial_map_size, num_nodes + 2);
		map = map_allocator::allocate(map_size);

		map_pointer nstart = map + (map_size - num_nodes) / 2;
		map_pointer nfinish = nstart + num_nodes - 1;

		map_pointer cur;
		for(cur = nstart; cur < nfinish; ++cur)
			*cur = allocate_node();

		start.set_node(nstart);
		finish.set_node(nfinish);
		start.cur = start.first;
		finish.cur = finish.first + num_elements % iterator::buffer_size();
	}

	void fill_initialize(size_type n, const value_type& value)
	{
		create_map_and_node(n);
		map_pointer cur;
		for(cur = start.node; cur < finish.node; ++cur)
			uninitialized_fill(*cur, *cur+iterator::buffer_size(), value);
		uninitialized_fill(finish.first, finish.cur, value);
	}

	void reallocate_map(size_type nodes_to_add, bool add_at_front)
	{
		size_type old_num_nodes = finish.node - start.node + 1;
		size_type new_num_nodes = old_num_nodes + nodes_to_add;

		map_pointer new_nstart;
		if(map_size > 2*new_num_nodes)
		{
			new_nstart = map + (map_size - new_num_nodes) / 2
						 + (add_at_front ? nodes_to_add : 0);
			if (new_nstart < start.node)
				copy(start.node, finish.node+1, new_nstart);
			else
				copy_backward(start.node, finish.node+1,
							  new_nstart+old_num_nodes);
		}
		else
		{
			size_type new_map_size = map_size + 
									 max(map_size, nodes_to_add) + 2;
			map_pointer new_map = map_allocator::allocate(new_map_size);
			new_nstart = map + (map_size - new_num_nodes) / 2
						 + (add_at_front ? nodes_to_add : 0);
			copy(start.node, finish.node+1, new_nstart);
			map_allocator::deallocate(map, map_size);
			map = new_map;
			map_size =new_map_size;									 
		}

		start.set_node(new_nstart);
		finish.set_node(new_nstart + old_num_nodes - 1);
	}

	void reserve_map_at_back(size_type nodes_to_add = 1)
	{
		if(nodes_to_add + 1 > map_size - (finish.node - map))
			reallocate_map(nodes_to_add, false);
	}

	void reserve_map_at_front(size_type nodes_to_add = 1)
	{
		if(nodes_to_add + 1 > start.node - map)
			reallocate_map(nodes_to_add, true);
	}

	void push_back_aux(const value_type& t)
	{
		value_type t_copy = t;
		reserve_map_at_back();
		*(finish.node + 1) = allocate_node();
		construct(finish.cur, t_copy);
		finish.set_node(finish.node + 1);
		finish.cur = finish.first;
	}

	void push_front_aux(const value_type& t)
	{
		value_type t_copy = t;
		reserve_map_at_front();
		*(start.node + 1) = allocate_node();
		start.set_node(start.node - 1);
		start.cur = start.last - 1;
		construct(start.cur, t_copy);
	}

	void pop_back_aux()
	{
		deallocate_node(finish.first);
		finish.set_node(finish.node - 1);
		finish.cur = finish.last - 1;
		destroy(finish.cur);
	}

	void pop_front_aux()
	{
		destroy(start.cur);
		deallocate_node(start.first);
		start.set_node(start.node + 1);
		start.cur = start.first;
	}

	iterator insert_aux(iterator position, const value_type& x)
	{
		difference_type index = position - start;
		value_type x_copy = x;
		if(index < size()/2)
		{
			push_front(front());
			iterator front1 = start;
			++front1;
			iterator front2 = front1;
			++front2;
			position = start + index;
			iterator pos1 = position;
			++pos1;
			copy(front2, pos1, front1);
		}
		else
		{
			push_back(back());
			iterator back1 = finish;
			--back1;
			iterator back2 = back1;
			--back2;
			position = start + index;
			copy_backward(position, back2, back1);
		}
		*position = x_copy;
		return position;
	}

public:
	deque(int n, const value_type& value)
		: start(), finish(), map(0), map_size(0)
	{
		fill_initialize(n, value);
	}

	void push_back(const value_type& t)
	{
		if(finish.cur != finish.last - 1)
		{
			construct(finish.cur, t);
			++finish.cur;
		}
		else
			push_back_aux(t);
	}

	void push_front(const value_type& t)
	{
		if(start.cur != start.last - 1)
		{
			construct(start.cur, t);
			--start.cur;
		}
		else
			push_front_aux(t);
	}

	void pop_back()
	{
		if(finish.cur != finish.start)
		{
			--finish.cur;
			destroy(finish.cur);
		}
		else
			pop_back_aux();
	}

	void pop_front()
	{
		if(start.cur != start.last - 1)
		{
			destroy(start.cur);
			++start.cur;
		}
		else
			pop_front_aux();
	}

	void clear()
	{
		for(map_pointer node = start.node+1;
			node < finish.node; ++node)
		{
			destroy(*node, *node+iterator::buffer_size());
			data_allocator::deallocate(*node, iterator::buffer_size());
		}
		if(start.node != finish.node)
		{
			destroy(start.cur, start.last);
			destroy(finish.first, finish.cur);
			data_allocator::deallocate(finish.first, iterator::buffer_size());
		}
		else
			destroy(start.cur, finish.cur);

		finish = start;
	}

public:
	iterator begin() const {return start;}
	iterator end() const {return finish;}

	reference operator[](size_type n)
	{return start[difference_type(n)];}

	reference front() const {return *start;}
	reference back()
	{
		iterator temp = finish;
		--temp;
		return *temp;
	}

	size_type size() const {return finish - start;}
	size_type max_size() const {return size_type(-1);}
	bool empty() const {return finish == start;}

	iterator insert(iterator position, const value_type& x)
	{
		if(position.cur == start.cur)
		{
			push_front(x);
			return start;
		}
		else if (position.cur == finish.cur)
		{
			push_back(x);
			iterator temp = finish;
			--temp;
			return temp;
		}
		else
			return insert_aux(position,x);
	}
};

}

#endif