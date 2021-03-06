
#pragma once

class Plugin;

class Timer : public API::Timer
{
public:
  Plugin* plugin;
  uint_t id;

  Timer(uint_t interval, Plugin* plugin) : plugin(plugin), id(0) { this->interval = interval; }
};
