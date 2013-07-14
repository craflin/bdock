
#pragma once

class Plugin;
class Dock;

class Icon : public API::Icon
{
public:
  Plugin* plugin;
  Icon* next;
  Icon* previous;
  RECT rect;

  Icon(HBITMAP icon, uint flags, Plugin* plugin, Icon* insertAfter, Icon* first);
  ~Icon();

  void draw(HDC dest, const Settings& settings);
};
