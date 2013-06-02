
#ifndef Clock_H
#define Clock_H

class Clock
{
public:
  Clock(Dock& dock);
  ~Clock();

  bool init();

private:
  Dock& dock;
  HBITMAP bitmap;
  HFONT font;
  HBRUSH bgBrush;
  COLORREF textColor;
  Icon* icon;
  COLORREF* bitmapData;
  Timer* timer;
  SYSTEMTIME st;

  static int handleTimerEvent(Timer* timer);

  void update();
};

#endif //Clock_H

