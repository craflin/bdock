
#ifndef Clock_H
#define Clock_H

class Clock : public Plugin
{
public:
  Clock();
  ~Clock();

  bool init();

private:
  HBITMAP bitmap;
  HFONT font;
  HBRUSH bgBrush;
  COLORREF textColor;
  Icon* icon;
  COLORREF* bitmapData;
  Timer* timer;
  SYSTEMTIME st;

  static int timerProc(Plugin* plugin, Timer* timer);

  void update();
  
};

#endif //Clock_H

