
#pragma once

class Plugin;
class Dock;

class Icon : public API::Icon
{
public:
  Plugin* plugin;
  RECT rect;

  Icon(HBITMAP icon, uint flags, Plugin* plugin);

  void draw(HDC dest, const Settings& settings);
};
