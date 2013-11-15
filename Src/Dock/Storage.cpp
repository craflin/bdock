
#include "stdafx.h"

Storage::Variant::Variant() : type(Null), data(0) {}

Storage::Variant::~Variant()
{
  free();
}

void Storage::Variant::free()
{
  switch(type)
  {
  case Str:
    delete str;
    str = 0;
    break;
  case Data:
    delete data;
    data = 0;
    break;
  }
}

Storage::Variant& Storage::Variant::operator=(const Variant& other)
{
  free();
  type = other.type;
  switch (type)
  {
  case Str:
    if (other.str)
      str = new String(*other.str);
    break;
  case Data:
    if (other.data)
      data = new Storage::Data(other.data->data, other.data->length);
    break;
  }
  return *this;
}

Storage::Data::Data(const void_t* data, uint length)
{
  this->data = Memory::alloc(length);
  this->length = length;
  Memory::copy(this->data, data, length);
}

Storage::Data::~Data()
{
  Memory::free(data);
}

Storage::Storage() : parent(0)
{
  current = this;
}

Storage::Storage(Storage* parent) : parent(parent)
{
  current = this;
}

Storage::~Storage()
{
  clear();
}

void Storage::clear()
{
  for (HashMap<String, Storage*>::Iterator i = storages.begin(), end = storages.end(); i != end; ++i)
    delete *i;
  for(Array<Storage*>::Iterator i = array.begin(), end = array.end(); i != end; ++i)
    delete *i;
  entries.clear();
  storages.clear();
  array.clear();
}

Storage* Storage::getCurrentStorage()
{
  current->current = current;
  return current;
}

void Storage::setCurrentStorage(Storage* storage)
{
  current = storage ? storage : this;
}

bool Storage::enterSection(const String& name)
{
  HashMap<String, Storage*>::Iterator i = current->storages.find(name);
  if(i == current->storages.end())
  {
    Storage* storage = new Storage(current);
    current->storages.append(name, storage);
    current = storage;
    return true;
  }
  else
  {
    current = *i;
    return true;
  }
}

Storage* Storage::getSection(const String& name)
{
  if(!enterSection(name))
    return 0;
  Storage* storage = getCurrentStorage();
  leave();
  return storage;
}

bool_t Storage::leave()
{
  if(!current->parent)
    return false;
  current = current->parent;
  return true;
}

bool Storage::enterNumSection(uint pos)
{
  if(pos >= current->array.size())
    setNumSectionCount(pos + 1);
  current = current->array[pos];
  return true;
}

Storage* Storage::getNumSection(uint pos)
{
  if(!enterNumSection(pos))
    return 0;
  Storage* storage = getCurrentStorage();
  leave();
  return storage;
}

bool_t Storage::deleteSection(const String& name)
{
  HashMap<String, Storage*>::Iterator i = current->storages.find(name);
  if(i == current->storages.end())
    return false;
  else  
  {
    delete *i;
    current->storages.remove(i);
    return true;
  }
}

bool_t Storage::deleteNumSection(uint_t pos)
{
  if(pos >= current->array.size())
    return false;
  delete current->array[pos];
  current->array.remove(pos);
  return true;
}

bool_t Storage::swapNumSections(uint_t pos1, uint_t pos2)
{
  uint size = (uint) current->array.size();
  if(pos1 >= size || pos2 >= size)
    return false;
  Storage* tmp = current->array[pos1];
  current->array[pos1] = current->array[pos2];
  current->array[pos2] = tmp;
  return true;
}

uint_t Storage::getNumSectionCount() const
{
  return (uint_t)current->array.size();
}

bool_t Storage::setNumSectionCount(uint_t size)
{
  if(size < current->array.size())
  {
    for(uint i = size, count = (uint)current->array.size(); i < count; ++i)
      delete current->array[i];
    current->array.resize(size);
  }
  else if(size > current->array.size())
  {
    uint i = (uint)current->array.size();
    current->array.resize(size);
    for(; i < size; ++i)
      current->array[i] = new Storage(current);
  }
  return true;
}

const String& Storage::getStr(const String& name, const String& default) const
{
  HashMap<String, Variant>::Iterator i = current->entries.find(name);
  if(i == current->entries.end())
    return default;
  const Variant& val = *i;
  if (val.type != Variant::Str)
    return default;
  return *val.str;
}

int_t Storage::getInt(const String& name, int_t default) const
{
  HashMap<String, Variant>::Iterator i = current->entries.find(name);
  if(i == current->entries.end())
    return default;
  const Variant& val = *i;
  if(val.type != Variant::Int)
    return default;
  return val._int;
}

uint_t Storage::getUInt(const String& name, uint default) const
{
  HashMap<String, Variant>::Iterator i = current->entries.find(name);
  if(i == current->entries.end())
    return default;
  const Variant& val = *i;
  if(val.type != Variant::UInt)
    return default;
  return val._uint;
}

bool_t Storage::getData(const String& name, const void_t*& data, uint& length, const void_t* defaultData, uint_t defaultLength) const
{
  HashMap<String, Variant>::Iterator i = current->entries.find(name);
  if(i == current->entries.end())
  {
    data = defaultData;
    length = defaultLength;
    return false;
  }
  const Variant& val = *i;
  if(val.type != Variant::Data)
  {
    data = defaultData;
    length = defaultLength;
    return false;
  }
  data = val.data->data;
  length = val.data->length;
  return true;
}

bool_t Storage::setStr(const String& name, const String& value)
{
  Variant& val = current->entries.append(name, Variant());
  val.type = Variant::Str;
  val.str = new String(value);
  return true;
}

bool_t Storage::setInt(const String& name, int_t value)
{
  Variant& val = current->entries.append(name, Variant());
  val.type = Variant::Int;
  val._int = value;
  return true;
}

bool_t Storage::setUInt(const String& name, uint_t value)
{
  Variant& val = current->entries.append(name, Variant());
  val.type = Variant::UInt;
  val._uint = value;
  return true;
}

bool_t Storage::setData(const String& name, const void_t* data, uint length)
{
  Variant& val = current->entries.append(name, Variant());
  val.type = Variant::Data;
  val.data = new Data(data, length);
  return true;
}

bool_t Storage::deleteEntry(const String& name)
{
  HashMap<String, Variant>::Iterator i = current->entries.find(name);
  if(i == current->entries.end())
    return false;
  current->entries.remove(i);
  return true;
}

bool_t Storage::save()
{
  if(filename.isEmpty())
    return false;
  File file;
  if (!file.open(filename, File::writeFlag))
    return false;
  uint_t header = 1; // minimal header
  return write(file, &header, sizeof(header)) && save(file);
}

bool_t Storage::load(const String& filename)
{
  this->filename = filename;
  File file;
  if (!file.open(filename, File::readFlag))
    return false;
  uint header;
  return read(file, &header, sizeof(header)) && header == 1 && load(file);
}

bool_t Storage::read(File& file, void_t* buffer, uint_t size)
{
  return file.read(buffer, size) == size;
}

bool_t Storage::write(File& file, const void_t* buffer, uint_t size)
{
  return file.write(buffer, size) == size;
}

bool_t Storage::load(File& file)
{
  clear();

  // read all entries
  uint_t size;
  if(!read(file, &size, sizeof(size)))
    return false;
  String keybuffer;
  uint_t keysize;
  for(uint i = 0; i < size; ++i)
  {
    if(!read(file, &keysize, sizeof(keysize)))
      return false;
    keybuffer.clear();
    keybuffer.reserve(keysize);
    if(!read(file, &keybuffer[0], keysize * sizeof(tchar_t)))
      return false;
    keybuffer.resize(keysize);
    Variant& var = entries.append(keybuffer, Variant());
    if(!read(file, &var.type, sizeof(var.type)))
      return false;
    switch(var.type)
    {
    case Variant::Int:
    case Variant::UInt:
      if(!read(file, &var._int, sizeof(int_t)))
        return false;
      break;
    case Variant::Str:
      {
        var.str = new String;
        uint_t length;
        if(!read(file, &length, sizeof(uint_t)))
          return false;
        var.str->reserve(length);
        if (!read(file, &(*var.str)[0], length * sizeof(tchar_t)))
          return false;
        var.str->resize(length);
      }
      break;
    case Variant::Data:
      var.data = new Data;
      if(!read(file, &var.data->length, sizeof(var.data->length)))
        return false;
      var.data->data = Memory::alloc(var.data->length);
      if(!read(file, var.data->data, var.data->length))
        return false;
      break;
    }
  }

  // read substorages
  if(!read(file, &size, sizeof(size)))
    return false;
  for(uint_t i = 0; i < size; ++i)
  {
    if (!read(file, &keysize, sizeof(keysize)))
      return false;
    keybuffer.clear();
    keybuffer.reserve(keysize);
    if (!read(file, &keybuffer[0], keysize * sizeof(tchar_t)))
      return false;
    keybuffer.resize(keysize);
    
    HashMap<String, Storage*>::Iterator it = storages.find(keybuffer);
    Storage** storage;
    if (it == storages.end())
      storage = &storages.append(keybuffer, 0);
    else
    {
      delete *it;
      storage = &*it;
    }
    *storage = new Storage(this);
    if(!(*storage)->load(file))
      return false;
  }

  // read array
  if(!read(file, &size, sizeof(size)))
    return false;
  array.reserve(size);
  for(uint i = 0; i < size; ++i)
  {
    Storage* storage = array.append(new Storage(this));
    if(!storage->load(file))
      return false;
  }

  return true;
}

bool Storage::save(File& file) const
{
  // save all entries
  uint_t size = (uint_t)entries.size();
  if(!write(file, &size, sizeof(size)))
    return false;
  for(HashMap<String, Variant>::Iterator i = entries.begin(), end = entries.end(); i != end; ++i)
  {
    const String& key(i.key());
    const Variant& var(*i);
    uint_t keysize = (uint_t)key.length();
    if(!write(file, &keysize, sizeof(keysize)) ||
       !write(file, &key[0], keysize * sizeof(tchar_t)) ||
       !write(file, &var.type, sizeof(var.type)) )
      return false;
    switch(var.type)
    {
    case Variant::Int:
    case Variant::UInt:
      if(!write(file, &var._int, sizeof(int)))
        return false;
      break;
    case Variant::Str:
    {
      uint_t length = var.str->length();
      if (!write(file, &length, sizeof(uint_t)) ||
        !write(file, &(*var.str)[0], length * sizeof(tchar_t)))
        return false;
      break;
    }
    case Variant::Data:
      if(!write(file, &var.data->length, sizeof(var.data->length)) ||
         !write(file, var.data->data, var.data->length))
        return false;
      break;
    }
  }

  // save substorages
  size = (uint_t)storages.size();
  if(!write(file, &size, sizeof(size)))
    return false;
  for(HashMap<String, Storage*>::Iterator i = storages.begin(), end = storages.end(); i != end; ++i)
  {
    const String& key = i.key();
    Storage* storage = *i;
    uint_t keysize = key.length();
    if(!write(file, &keysize, sizeof(keysize)) ||
       !write(file, &key[0], keysize * sizeof(tchar_t)) ||
       !storage->save(file))
      return false;
  }

  // save array
  size = (uint_t)array.size();
  if(!write(file, &size, sizeof(size)))
    return false;
  for(Array<Storage*>::Iterator i = array.begin(), end = array.end(); i != end; ++i)
    if(!(*i)->save(file))
      return false;

  return true;
}
