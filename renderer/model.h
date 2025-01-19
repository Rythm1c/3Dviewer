#ifndef MODEL_H
#define MODEL_H

#include "../math/mat4.h"
#include "../math/quaternion.h"
#include "../math/vec3.h"
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

  std::vector<struct Mesh> meshes;
  Color3f color;

private:
  class Transform *transform;
};

#endif
