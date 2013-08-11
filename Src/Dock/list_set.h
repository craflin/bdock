
#pragma once

template<typename V> class list_set : public std::list<V>
{
public:
  std::list<V>::iterator find(const V& value)
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

  void erase(const std::list<V>::iterator& it)
  {
    std::list<V>::erase(it);
  }
};

// TODO: use unordered_set to optimize this
