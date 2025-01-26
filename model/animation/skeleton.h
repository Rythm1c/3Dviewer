#ifndef SKELETON_H
#define SKELETON_H

#include "../../math/mat4.h"
#include <vector>

class Skeleton
{
public:

  Skeleton();
  ~Skeleton() {}

  class Pose *restPose;
  std::vector<Mat4x4> inversePose;

  std::vector<Mat4x4> getFinalMat() const;
};

#endif