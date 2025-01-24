#ifndef MATERIAL_H
#define MATERIAL_H

#include "../../math/vec3.h"

class Texture;

struct Material
{
  float ao;
  float roughness;
  float metallicness;
  Vector3f baseCol;
  Texture *baseTex;
  Texture *metallicMap;

  void configShader(class Shader &);
};

#endif