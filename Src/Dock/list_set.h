
#pragma once

#include <list>

template<typename V> class list_set : public std::list<V>
{
public:
  iterator find(const V& value)
  {
    for(auto i = begin(), _end = end(); i != _end; ++i)
      if(*i == value)
        return i;
    return end();
  }

  void erase(const V& value)
  {
    return remove(value);
  }

  void erase(const iterator& it)
  {
    std::list<V>::erase(it);
  }
};

// TODO: use unordered_set to optimize this
