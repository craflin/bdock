
#ifndef HideTaskBar_H
#define HideTaskBar_H

class HideTaskBar
{
public:
  HideTaskBar();
  ~HideTaskBar();

  bool init();

private:
  static int instances;
  static bool hidden;
  static LPARAM originalState;

  bool showTaskBar(bool show, LPARAM& state) const;

};

#endif //HideTaskBar_H

