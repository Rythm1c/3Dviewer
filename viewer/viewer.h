#ifndef VIEWER_H
#define VIEWER_H

#include <map>
#include <string>

class Viewer
{
public:
  Viewer();
  ~Viewer();

  void addModel();

  class Camera *camera;
  std::map<std::string, class Model *> models;
};

#endif