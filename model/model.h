#ifndef MODEL_H
#define MODEL_H

#include "../math/mat4.h"
#include "../math/quaternion.h"
#include "../math/vec3.h"

#include "renderer/renderer.h"
#include "animation/animation.h"
#include "foreign/gltf.h"

#include <vector>

enum ModelType
{
  ModelGLTF,
  ModelOBJ,
};

class Model
{
public:
  Model();
  ~Model() {}

  void orient(Quat);
  void scale(Vector3f);
  void translate(Vector3f);

  void render();
  void clean();

  Mat4x4 get_transform();

  std::vector<Mesh> meshes;
  std::vector<Texture> textures;
  std::vector<Clip> clips;
  
  Color3f color;

private:
  class Transform *transform;
};

#endif
