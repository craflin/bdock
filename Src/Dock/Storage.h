
#pragma once

class Storage
{
public:
  Storage();
  ~Storage();

  Storage* getCurrentStorage();
  void_t setCurrentStorage(Storage* storage);

  bool_t enterSection(const String& name);
  bool_t enterNumSection(uint_t pos);
  Storage* getSection(const String& name); 
  Storage* getNumSection(uint_t pos);
  
  bool_t deleteSection(const String& name);
  bool_t deleteNumSection(uint_t pos);

  bool_t swapNumSections(uint_t pos1, uint_t pos2);

  uint_t getNumSectionCount() const;
  bool_t setNumSectionCount(uint_t size);

  bool_t leave();

  const String& getStr(const String& name, const String& default = String()) const;
  int_t getInt(const String& name, int default) const;
  uint_t getUInt(const String& name, uint_t default) const;
  bool_t getData(const String& name, const void_t*& data, uint_t& length, const void_t* defaultData, uint_t defaultLength) const;
  bool_t setStr(const String& name, const String& value);
  bool_t setInt(const String& name, int_t value);
  bool_t setUInt(const String& name, uint_t value);
  bool_t setData(const String& name, const void_t* data, uint_t length);
  bool_t deleteEntry(const String& name);

  bool_t save();
  bool_t load(const String& file);

private:
  class Data
  {
  public:
    void_t* data;
    uint_t length;
    Data(const void_t* data, uint_t length);
    Data() : data(0), length(0) {};
    ~Data();
  };

  class Variant
  {
  public:
    enum Type
    {
      Null,
      Str,
      Int,
      UInt,
      Data,
    };

    Type type;
    union
    {
      int_t _int;
      uint_t _uint;
      String* str;
      Storage::Data* data;
    };

    Variant();
    ~Variant();
    void free();
    Variant& operator=(const Variant& other);
  };

  String filename;
  HashMap<String, Variant> entries;
  HashMap<String, Storage*> storages;
  Array<Storage*> array;
  Storage* current;
  Storage* parent;

  Storage(Storage* parent);

  void_t clear();

  bool_t load(File& file);
  bool_t save(File& file) const;

  static bool_t read(File& file, void_t* buffer, uint_t size);
  static bool_t write(File& file, const void_t* buffer, uint_t size);
};
