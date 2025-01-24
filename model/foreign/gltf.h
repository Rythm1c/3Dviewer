#ifndef GLTF_H
#define GLTF_H

#include <iostream>
#include <vector>
#include "../renderer/texture.h"

struct Mesh;

class GLTFFile
{
public:
  GLTFFile(std::string path, Model &model);
  ~GLTFFile() {}

  std::vector<Mesh> getMeshes(tinygltf::Model &tinyModel);
  std::vector<Texture> getTextures(tinygltf::Model &tinyModel);
};

#endif
