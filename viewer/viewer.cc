#include "viewer.h"
#include "camera.h"
#include "../renderer/renderer.h"

Viewer::Viewer()
 : camera(new Camera()){}
Viewer::~Viewer()
{
  delete this->camera;
  for (auto &model : models)
  {
    model.second->clean();
    delete model.second;
  }
}