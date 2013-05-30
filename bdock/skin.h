

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

  Bitmap leftBg;
  Bitmap rightBg;
  Bitmap midBg;
  Bitmap activeBg;
  Bitmap fullBg;
  Bitmap halfBg;

  Skin();
  ~Skin();

  bool init(const wchar* name);

  void draw(HDC dest, const SIZE& size, const RECT& update);

private:
  HBRUSH bgBrush;
};

#endif
