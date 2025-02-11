#ifndef VIEWER_H
#define VIEWER_H

#include <map>
#include <string>
#include "camera.h"
#include "../math/math.h"

class Shader;

class Viewer
{
public:
  Viewer();
  ~Viewer();

  void init();

  void addModel(std::string name, std::string path);

  void update(float ratio, float elapsed);
  void renderCurrModel();

  Camera *camera;

  std::string currModel;

  Vector3f lightDir;

private:
  Shader *phongStatic;
  Shader *phongAnimated;

  Shader *pbrStatic;
  Shader *pbrAnimated;

  std::map<std::string, class Model *> models;
};

#endif