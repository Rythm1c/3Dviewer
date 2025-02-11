#include "model.h"

Model::Model() : currAnim(-1), pose(Pose()), color(Color3f(1.0)), skeleton(Skeleton()), transform(new Transform()) {}

void Model::translate(Vector3f pos) { this->transform->translation = pos; }

void Model::scale(Vector3f size) { this->transform->scaling = size; }

void Model::orient(Quat orientation)
{
  this->transform->orientation = orientation;
}

Mat4x4 Model::get_transform() { return this->transform->get(); }

void Model::render()
{
  for (auto &mesh : meshes)
  {
    mesh.render();
  }
}

void Model::animate(float elapsed)
{

  if (this->currAnim > -1 && this->clips.size() > 0)
  {
    this->pose = this->skeleton.restPose;
    // this->clips[this->currAnim].sample(this->pose, elapsed);
  }
}

std::vector<Mat4x4> Model::getPose()
{
  std::vector<Mat4x4> result;

  if ((this->currAnim > -1) && (this->clips.size() > 0))
  {
    int len = this->pose.size();
    result.resize(len, Mat4x4());
    for (int i = 0; i < len; ++i)
    {
      Transform world = this->pose.getGlobalTranform(i);
      Mat4x4 invPose = this->skeleton.inversePose[i];

      result[i] = world.get() * invPose;
    }
  }

  return result;
}

void Model::clean()
{
  delete transform;

  for (auto &mesh : meshes)
  {
    mesh.clean();
  }
}
