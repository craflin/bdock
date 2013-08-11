
#pragma once

template<typename K, typename V> class list_map : public std::list<std::pair<K, V> >
{
public:
  std::list<std::pair<K, V> >::iterator find(const K& key)
  {
    for(auto i = begin(), _end = end(); i != _end; ++i)
      if(i->first == key)
        return i;
    return end();
  }
};
/*

// TODO:
template<typename K, typename V> class list_map
{
public:
  class iterator : public std::iterator<std::bidirectional_iterator_tag, std::pair<K, V> >
  {
  public:
    iterator() : item(0) {}
    iterator(const iterator& other) : item(other.item) {}
    iterator& operator++() {item = item->next; return *this;}
    iterator operator++(int) {iterator tmp(*this); item = item->next; return tmp;}
    bool operator==(const iterator& other) {return item == other.item;}
    bool operator!=(const iterator& other) {return item != other.item;}
    std::pair<K, V>& operator*() {return item->data;}
  private:
    Item* item;
  };

  void push_back(const std::pair<K, V>& data)
  {
    std::unordered_map<K, Item>::iterator mapIt = map.insert(std::make_pair(item.key, Item(data)));
    Item& item = mapIt->second;
    item.mapIt = mapIt;
    if(!firstItem)


  }
  
  void erase(iterator& it)
  {
    map.erase(it.item->mapIt);
  }


private:
  struct Item
  {
    std::pair<K, V> data;
    std::unordered_map<K, Item>::iterator mapIt;
    Item* prev;
    Item* next;
    
    Item(const std::pair<K, V>& data) : data(data) {}
  };
  
  std::unordered_map<K, Item> map;
  Item* firstItem;
  Item* lastItem;
};
*/