
#ifndef Timer_H
#define Timer_H

class Plugin;

class Timer : public API::Timer
{
public:
  Plugin* plugin;
  uint id;

  Timer(uint interval, Plugin* plugin) : plugin(plugin), id(0) { this->interval = interval; }
};

#endif
