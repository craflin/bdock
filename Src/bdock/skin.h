

#ifndef Skin_H
#define Skin_H

class Skin 
{
public:
  class Bitmap
  {
  public:
    HBITMAP bmp;
    SIZE size;

    void draw(HDC dest, int x, int y) const;

  private:
    Bitmap();
    ~Bitmap();

    bool load(const wchar* file);

    friend class Skin;
  };

  // dock backround
  Bitmap leftBg;
  Bitmap rightBg;
  Bitmap midBg;

  // icon background
  Bitmap activeBg;
  Bitmap defaultBg;
  Bitmap fullBg;
  Bitmap halfBg;

  Skin();
  ~Skin();

  bool init(const wchar* name);

  void draw(HDC dest, const SIZE& size, const RECT& update);

private:
  HBRUSH bgBrush;
  HBITMAP bg;
  SIZE bgSize;
};

#endif
