#ifndef CLIP_H
#define CLIP_H

#include <vector>
#include <string>

class Clip
{
public:
  Clip();
  ~Clip() {}

  uint getIdAtIndex(uint index);
  void seyIdAtIndex(uint idx, uint id);
  uint size();
  float sample(class Pose &outPose, float inTime);
  void ReCalculateDuartion();

  std::string &GetName();
  void SetName(const std::string &inNewName);
  float GetDuration();
  float GetStartTime();
  float GetEndTime();
  bool GetLooping();
  void SetLooping(bool inLooping);

private:
  std::string name;
  float startTime;
  float endTime;
  bool looping;
  std::vector<class TransformTrack> tracks;

  float adjustTimeToFitRange(float time);
};

#endif
