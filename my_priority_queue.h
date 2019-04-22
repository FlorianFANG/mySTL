#ifndef _MY_PRIORITY_QUEUE
#define _MY_PRIORITY_QUEUE

#include "my_alloc.h"
#include <stddef.h>  
#include "my_algo.h"          // for heap algorithms
#include "my_construct.h" 
#include "my_uninitialized.h"
#include "my_vector.h"        // priority queue is implemented based on vector


namespace fyj
{

template <class T, class Sequence = vector<T>,
		  class Compare = less<typename Sequence::value_type> >
class _MY_PRIORITY_QUEUE
{
public:
	typedef typename Sequence::value_type   value_type;
	typedef typename Sequence::size_type    size_type;
	typedef typename Sequence::reference 	reference;
	typedef typename Sequence::const_reference const_reference;

protected:
	Sequence data;
	Compare comp;

public:
	priority_queue() : data() {}
	explicit priority_queue(const Compare& x) : data(), comp(x){}

	template <class InputIterator>
	priority_queue(InputIterator first, InputIterator last, const Compare& x):
		data(first, last), comp(x)
	{make_heap(data.begin(), data.end(), comp);}

	template <class InputIterator>
	priority_queue(InputIterator first, InputIterator last):
		data(first, last)
	{make_heap(data.begin(), data.end(), comp);}

	bool empty() const {return data.empty();}

	size_type size() const {return data.size();}

	const_reference top() const {return data.front();}

	void push(const value_type& x)
	{
		data.push_back(x);
		push_heap(data.begin(), data.end(), comp);
	}

	void pop()
	{
		pop_heap(data.begin(), data.end(), comp);
		data.pop_back();
	}
}		  

} // end of namespace


#endif