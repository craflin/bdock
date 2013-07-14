
#pragma once

class Settings
{
public:
  enum Alignment
  {
    center,
    left,
    right,
    top,
    bottom,
  };

  int barHeight;
  int leftMargin;
  int topMargin; // animation and text draw area  
  int rightMargin;
  int bottomMargin;
  int itemWidth;
  int iconWidth;
  int iconHeight;
  Alignment alignment;
  Alignment edge;

  Settings(Storage* storage);
};
