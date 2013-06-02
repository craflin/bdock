
#ifndef Storage_H
#define Storage_H

class Storage
{
public:
  Storage();
  ~Storage();

  Storage* getCurrentStorage();
  void setCurrentStorage(Storage* storage);

  bool enterSection(const char* name);
  bool enterNumSection(uint pos);
  Storage* getSection(const char* name); 
  Storage* getNumSection(uint pos);
  
  bool deleteSection(const char* name);
  bool deleteNumSection(uint pos);

  uint getNumSectionCount() const;
  bool setNumSectionCount(uint size);

  bool leave();

  const wchar* getStr(const char* name, uint* length = 0, const wchar* default = L"", uint defaultLength = 0) const;
  int getInt(const char* name, int default) const;
  uint getUInt(const char* name, uint default) const;
  bool getData(const char* name, char** data, uint* length, const char* defaultData, uint defaultLength) const;
  bool setStr(const char* name, const wchar* value, uint length);
  bool setInt(const char* name, int value);
  bool setUInt(const char* name, uint value);
  bool setData(const char* name, char* data, uint length);
  bool deleteEntry(const char* name);

  bool save(const wchar* file);
  bool load(const wchar* file);

private:
  class Data
  {
  public:
    char* data;
    uint length;
    Data(const char* data, uint length);
    Data() {};
    ~Data();
  };

  class Str
  {
  public:
    wchar* data;
    uint length;
    Str(const wchar* data, uint length);
    Str() {};
    ~Str();
  };

  class Variant
  {
  public:
    enum Type
    {
      Null,
      Str, // special kind of data
      Int,
      UInt,
      Data,
    };

    Type type;
    union
    {
      int _int;
      uint _uint;
      Storage::Str* str;
      Storage::Data* data;
    };

    Variant();
    ~Variant();
    void free();
  };

  std::unordered_map<std::string, Variant> entries;
  std::unordered_map<std::string, Storage*> storages;
  std::vector<Storage*> array;
  Storage* current;
  Storage* parent;

  Storage(Storage* parent);

  void free();

  bool load(HANDLE hFile);
  bool save(HANDLE hFile) const;

  bool read(HANDLE hFile, void* buffer, uint size) const;
  bool write(HANDLE hFile, const void* buffer, uint size) const;

};

#endif 
