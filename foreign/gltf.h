#ifndef GLTF_H
#define GLTF_H

#include <iostream>
#include <vector>

struct Mesh;

class GLTFFile {
public:
  GLTFFile(std::string path);
  ~GLTFFile() {}

  std::vector<Mesh> meshes;
};

#endif
