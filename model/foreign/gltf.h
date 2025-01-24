#ifndef GLTF_H
#define GLTF_H

#include <iostream>
#include <vector>
#include "tiny_gltf.h"

struct Mesh;
class Texture;

class GLTFFile
{
public:
  GLTFFile(std::string path);
  ~GLTFFile() {}

  std::vector<Mesh> getMeshes();
  std::vector<Texture> getTextures();

  void populateModel(class Model &model);

private:
  tinygltf::Model tinyModel;
};

#endif
