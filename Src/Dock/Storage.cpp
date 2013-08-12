
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
    if(str)
      delete str;
    break;
  case Data:
    if(data)
      delete data;
    break;
  }
}

Storage::Data::Data(const char* data, uint length) : data((char*)malloc(length)), length(length)
{
  memcpy(this->data, data, length);
}

Storage::Data::~Data()
{
  if(data)
    ::free(data);
}

Storage::Str::Str(const wchar* data, uint length) : length(length)
{
  uint size = (length + 1) * sizeof(wchar);
  this->data = (wchar*)malloc(size);
  memcpy(this->data, data, size);
  this->data[length] = L'\0';
}

Storage::Str::~Str()
{
  if(data)
    ::free(data);
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
  free();
}

void Storage::free()
{
  for(std::unordered_map<std::string, Storage*>::iterator i = storages.begin(), end = storages.end(); i != end; ++i)
    delete i->second;
  for(std::vector<Storage*>::iterator i = array.begin(), end = array.end(); i != end; ++i)
    delete *i;
  entries.clear();
  storages.clear();
  array.resize(0);
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

bool Storage::enterSection(const char* name)
{
  std::string key(name);
  std::unordered_map<std::string, Storage*>::iterator i = current->storages.find(key);
  if(i == current->storages.end())
  {
    Storage* storage = new Storage(current);
    current->storages[key] = storage;
    current = storage;
    return true;
  }
  else
  {
    current = i->second;
    return true;
  }
}

Storage* Storage::getSection(const char* name)
{
  if(!enterSection(name))
    return 0;
  Storage* storage = getCurrentStorage();
  leave();
  return storage;
}

bool Storage::leave()
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

bool Storage::deleteSection(const char* name)
{
  std::string key(name);
  std::unordered_map<std::string, Storage*>::iterator i = current->storages.find(key);
  if(i == current->storages.end())
    return false;
  else  
  {
    delete i->second;
    current->storages.erase(i);
    return true;
  }
}

bool Storage::deleteNumSection(uint pos)
{
  if(pos >= current->array.size())
    return false;
  delete current->array[pos];
  current->array.erase(current->array.begin() + pos);
  return true;
}

bool Storage::swapNumSections(uint pos1, uint pos2)
{
  uint size = (uint) current->array.size();
  if(pos1 >= size || pos2 >= size)
    return false;
  Storage* tmp = current->array[pos1];
  current->array[pos1] = current->array[pos2];
  current->array[pos2] = tmp;
  return true;
}

uint Storage::getNumSectionCount() const
{
  return (uint)current->array.size();
}

bool Storage::setNumSectionCount(uint size)
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

const wchar* Storage::getStr(const char* name, uint* length, const wchar* default, uint defaultLength) const
{
  std::unordered_map<std::string, Variant>::const_iterator i = current->entries.find(std::string(name));
  if(i == current->entries.end())
  {
    if(length)
      *length = defaultLength;
    return default;
  }
  const Variant& val(i->second);
  if(val.type != Variant::Str)
  {
    if(length)
      *length = defaultLength;
    return default;
  }
  if(length)
    *length = val.str->length;
  return val.str->data;
}

int Storage::getInt(const char* name, int default) const
{
  std::unordered_map<std::string, Variant>::const_iterator i = current->entries.find(std::string(name));
  if(i == current->entries.end())
    return default;
  const Variant& val(i->second);
  if(val.type != Variant::Int)
    return default;
  return val._int;
}

uint Storage::getUInt(const char* name, uint default) const
{
  std::unordered_map<std::string, Variant>::const_iterator i = current->entries.find(std::string(name));
  if(i == current->entries.end())
    return default;
  const Variant& val(i->second);
  if(val.type != Variant::UInt)
    return default;
  return val._uint;
}

bool Storage::getData(const char* name, char** data, uint* length, const char* defaultData, uint defaultLength) const
{
  std::unordered_map<std::string, Variant>::iterator i = current->entries.find(std::string(name));
  if(i == current->entries.end())
  {
    *data = (char*)defaultData;
    if(length)
      *length = defaultLength; 
    return false;
  }
  const Variant& val(i->second);
  if(val.type != Variant::Data)
  {
    *data = (char*)defaultData;
    if(length)
      *length = defaultLength; 
    return false;
  }
  *data = (char*)val.data->data;
  if(length)
    *length = val.data->length;
  return true;
}


bool Storage::setStr(const char* name, const wchar* value, uint length)
{
  Variant& val(current->entries[std::string(name)]);
  val.free();
  val.type = Variant::Str;
  val.str = new Str(value, length ? length : (uint)wcslen(value));
  return true;
}

bool Storage::setInt(const char* name, int value)
{
  Variant& val(current->entries[std::string(name)]);
  val.free();
  val.type = Variant::Int;
  val._int = value;
  return true;
}

bool Storage::setUInt(const char* name, uint value)
{
  Variant& val(current->entries[std::string(name)]);
  val.free();
  val.type = Variant::UInt;
  val._uint = value;
  return true;
}

bool Storage::setData(const char* name, char* data, uint length)
{
  Variant& val(current->entries[std::string(name)]);
  val.free();
  val.type = Variant::Data;
  val.data = new Data(data, length);
  return true;
}

bool Storage::deleteEntry(const char* name)
{
  std::unordered_map<std::string, Variant>::iterator i = current->entries.find(std::string(name));
  if(i == current->entries.end())
    return false;
  current->entries.erase(i);
  return true;
}

bool Storage::save()
{
  if(filename.empty())
    return false;
  HANDLE hFile = CreateFile(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
    return false;    
  uint header = 1; // minimal header
  bool ret = write(hFile, &header, sizeof(header)) && save(hFile);
  CloseHandle(hFile);
  return ret;
}

bool Storage::load(const wchar* file)
{
  HANDLE hFile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
  if (hFile == INVALID_HANDLE_VALUE)
    return false;
  uint header;
  bool ret = read(hFile, &header, sizeof(header)) && header == 1 && load(hFile);
  CloseHandle(hFile);
  if(ret)
    filename = file;
  return ret;
}

bool Storage::read(HANDLE hFile, void* buffer, unsigned size) const
{
  DWORD bi;
  return ReadFile(hFile, buffer, size, &bi, NULL) && bi == size;
}

bool Storage::write(HANDLE hFile, const void* buffer, unsigned size) const
{
  DWORD bi;
  return WriteFile(hFile, buffer, size, &bi, NULL) ? true : false;
}

bool Storage::load(HANDLE hFile)
{
  free();  

  // read all entries
  uint size;
  if(!read(hFile, &size, sizeof(size)))
    return false;
  std::vector<char> keybuffer;
  uint keysize;  
  for(uint i = 0; i < size; ++i)
  {
    if(!read(hFile, &keysize, sizeof(keysize)))
      return false;
    keybuffer.resize(keysize + 1);
    if(!read(hFile, &keybuffer[0], keysize))
      return false;
    keybuffer[keysize] = '\0';
    Variant& var(entries[std::string(&keybuffer[0])]);
    if(!read(hFile, &var.type, sizeof(var.type)))
      return false;
    switch(var.type)
    {
    case Variant::Int:
    case Variant::UInt:
      if(!read(hFile, &var._int, sizeof(int)))
        return false;
      break;
    case Variant::Str:
      {
        var.str = new Str;
        if(!read(hFile, &var.str->length, sizeof(var.str->length)))
          return false;
        uint size = var.str->length * sizeof(wchar);
        var.str->data = (wchar*)malloc(size + sizeof(wchar));
        if(!read(hFile, var.str->data, size) )
          return false;
        var.str->data[var.str->length] = L'\0';
      }
      break;
    case Variant::Data:
      var.data = new Data;
      if(!read(hFile, &var.data->length, sizeof(var.data->length)))
        return false;
      var.data->data = (char*)malloc(var.data->length);
      if(!read(hFile, var.data->data, var.data->length) )
        return false;
      break;
    }
  }

  // read substorages
  if(!read(hFile, &size, sizeof(size)))
    return false;
  for(uint i = 0; i < size; ++i)
  {
    if(!read(hFile, &keysize, sizeof(keysize)))
      return false;
    keybuffer.resize(keysize + 1);
    if(!read(hFile, &keybuffer[0], keysize))
      return false;
    keybuffer[keysize] = '\0';
    
    Storage*& storage = storages[std::string(&keybuffer[0])];
    storage = new Storage(this);
    if(!storage->load(hFile))
      return false;
  }

  // read array
  if(!read(hFile, &size, sizeof(size)))
    return false;
  array.resize(size);
  for(uint i = 0; i < size; ++i)
  {
    Storage*& storage = array[i];
    storage = new Storage(this);
    if(!storage->load(hFile))
      return false;
  }

  return true;
}

bool Storage::save(HANDLE hFile) const
{
  // save all entries
  uint size = (uint)entries.size();
  if(!write(hFile, &size, sizeof(size)))
    return false;
  for(std::unordered_map<std::string, Variant>::const_iterator i = entries.begin(), end = entries.end(); i != end; ++i)
  {
    const std::string& key(i->first);
    const Variant& var(i->second);
    uint keysize = (uint)key.size();
    if(!write(hFile, &keysize, sizeof(keysize)) ||
       !write(hFile, key.c_str(), keysize) ||
       !write(hFile, &var.type, sizeof(var.type)) )
      return false;    
    switch(var.type)
    {
    case Variant::Int:
    case Variant::UInt:
      if(!write(hFile, &var._int, sizeof(int)))
        return false;
      break;
    case Variant::Str:
      if(!write(hFile, &var.str->length, sizeof(var.str->length)) ||
         !write(hFile, var.str->data, var.str->length * sizeof(wchar)) )
        return false;
      break;
    case Variant::Data:
      if(!write(hFile, &var.data->length, sizeof(var.data->length)) ||
         !write(hFile, var.data->data, var.data->length) )
        return false;
      break;
    }
  }

  // save substorages
  size = (uint)storages.size();
  if(!write(hFile, &size, sizeof(size)))
    return false;
  for(std::unordered_map<std::string, Storage*>::const_iterator i = storages.begin(), end = storages.end(); i != end; ++i)
  {
    const std::string& key(i->first);
    Storage* storage(i->second);
    uint keysize = (uint)key.size();
    if(!write(hFile, &keysize, sizeof(keysize)) ||
       !write(hFile, key.c_str(), keysize) ||
       !storage->save(hFile))
      return false;
  }

  // save array
  size = (uint)array.size();
  if(!write(hFile, &size, sizeof(size)))
    return false;
  for(std::vector<Storage*>::const_iterator i = array.begin(), end = array.end(); i != end; ++i)
    if(!(*i)->save(hFile))
      return false;

  return true;
}
