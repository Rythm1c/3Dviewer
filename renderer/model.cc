#include "model.h"
#include "mesh.h"
#include "../math/transform.h"

Model::Model() : color(Color3f(1.0)), transform(new Transform()) {}

void Model::translate(Vector3f pos) { this->transform->translation = pos; }

void Model::scale(Vector3f size) { this->transform->scaling = size; }

void Model::orient(Quat orientation)
{
  this->transform->orientation = orientation;
}

Mat4x4 Model::get_transform() { return this->transform->get(); }

void Model::init()
{
  for (auto &mesh : meshes)
  {
    mesh.init();
  }
}

void Model::render()
{
  for (auto &mesh : meshes)
  {
    mesh.render();
  }
}

void Model::clean()
{
  delete transform;

  for (auto &mesh : meshes)
  {
    mesh.clean();
  }
}
