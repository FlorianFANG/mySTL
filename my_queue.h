#ifndef _MY_QUEUE_
#define _MY_QUEUE_

#include "my_alloc.h"
#include <stddef.h>  
#include "my_algo.h" 
#include "my_construct.h" 
#include "my_uninitialized.h"
#include "my_iterator.h"
#include "my_functors.h"
#include "my_deque.h"

namespace fyj
{

template <class T, class Sequence = deque<T> >
class queue
{
public:
	typedef typename Sequence::value_type value_type;
	typedef typename Sequence::size_type size_type;
	typedef typename Sequence::reference reference;
	typedef typename Sequence::const_reference const_reference;

protected:
	Sequence c;

public:
	bool empty() const {return c.empty();}
	size_type size() const {return c.size();}
	reference front() const {return c.front();}
	reference back() const {return c.back();}
	void push(const value_type& x) {c.push_back(x);}
	void pop() {c.pop_front();}

friend bool operator==(const queue<T, Sequence>& x,
					   const queue<T, Sequence>& y)
	{return x.c == y.c;}

friend bool operator<(const queue<T, Sequence>& x,
					  const queue<T, Sequence>& y)
	{return x.c < y.c;}

};

}

#endif