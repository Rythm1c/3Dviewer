#ifndef GLTF_H
#define GLTF_H

#include <iostream>
#include <vector>
#include "tiny_gltf.h"

class GLTFFile
{
public:
  GLTFFile(std::string path);
  ~GLTFFile() {}

  void populateModel(class Model &model);

private:
  tinygltf::Model tinyModel;

  std::vector<struct Mesh> getMeshes();
  std::vector<class Texture> getTextures();
  std::vector<class Clip> getClips();
  std::vector<class Skeleton> getSkeleton();
};

#endif
