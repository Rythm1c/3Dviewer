#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "gltf.h"
#include "../renderer/mesh.h"
#include "../animation/clip.h"
#include "../animation/skeleton.h"
#include "../animation/pose.h"
#include "../animation/frame.h"
#include "../model.h"

#include <filesystem>

namespace fs = std::filesystem;

GLTFFile::GLTFFile(std::string &path)
{
  tinygltf::TinyGLTF loader;
  std::string err, warn;

  if (!loader.LoadASCIIFromFile(&this->tinyModel, &err, &warn, path))
  {
    throw std::runtime_error(warn + err);
  }
}

void GLTFFile::populateModel(Model &model)
{
  model.meshes = this->getMeshes();
  model.clips = this->getClips();
  model.textures = this->getTextures();
  model.skeleton = this->getSkeleton();
}

template <typename T>
const T *getData(const tinygltf::Model &tinyModel, const int index);

std::vector<Mesh> GLTFFile::getMeshes()
{
  std::vector<Mesh> meshes;

  Vertex vertex = {};
  Mesh tmpmesh = {};
  tmpmesh.mode = TRIANGLES;

  for (size_t m = 0; m < this->tinyModel.meshes.size(); ++m)
  {
    tinygltf::Mesh &mesh = this->tinyModel.meshes[m];

    for (size_t j = 0; j < mesh.primitives.size(); ++j)
    {
      tmpmesh.vertices.clear();
      tmpmesh.indices.clear();

      tinygltf::Primitive &primitive = mesh.primitives[j];

      // positions
      const float *positions = getData<float>(
          this->tinyModel,
          primitive.attributes["POSITION"]);
      // normals
      const float *normals = getData<float>(
          this->tinyModel,
          primitive.attributes["NORMAL"]);
      // indices
      const uint *_indices = getData<uint>(
          this->tinyModel,
          primitive.indices);

      size_t vertCount = this->tinyModel.accessors[primitive.attributes["POSITION"]].count;
      size_t indexCount = this->tinyModel.accessors[primitive.indices].count;

      const tinygltf::Material &material = tinyModel.materials[primitive.material];
      const tinygltf::PbrMetallicRoughness &pbr = material.pbrMetallicRoughness;

      Vector3f baseCol = Vector3f(
          pbr.baseColorFactor[0],
          pbr.baseColorFactor[1],
          pbr.baseColorFactor[2]);

      tmpmesh.material = {
          .roughness = float(pbr.roughnessFactor),
          .metallicness = float(pbr.metallicFactor),
          .baseCol = baseCol,
          .baseTex = pbr.baseColorTexture.index,
          .metallicMap = pbr.metallicRoughnessTexture.index,
      };

      /* std::cout << "base texture index:" << pbr.baseColorTexture.index << "\n";
      std::cout << "metallic map index:" << pbr.metallicRoughnessTexture.index << "\n"; */

      tmpmesh.material.baseTex = tinyModel.textures[pbr.baseColorTexture.index].source;

      for (size_t i = 0; i < vertCount; ++i)
      {
        vertex.pos = {
            positions[i * 3 + 0],
            positions[i * 3 + 1],
            positions[i * 3 + 2]};

        vertex.norm = {
            normals[i * 3 + 0],
            normals[i * 3 + 1],
            normals[i * 3 + 2]};

        tmpmesh.vertices.push_back(vertex);
      }
      for (size_t i = 0; i < indexCount; ++i)
      {
        tmpmesh.indices.push_back(_indices[i]);
      }
      tmpmesh.init();
      meshes.push_back(tmpmesh);
    }
  }
  return meshes;
}

std::vector<Texture> GLTFFile::getTextures()
{
  std::vector<Texture> textures;
  textures.resize(this->tinyModel.textures.size());

  for (int i = 0; i < this->tinyModel.textures.size(); i++)
  {
    const tinygltf::Texture &tex = this->tinyModel.textures[i];
    const tinygltf::Image &image = this->tinyModel.images[tex.source];

    // for debugging purposes
    /* for (int y = 0; y < image.height; y++)
    {
      for (int x = 0; x < image.width; x++)
      {

        std::cout << "(";
        for (int p = 0; p < 3; p++)
        {
          std::cout << (uint)image.image.data()[y * x * 3 + 3 * x + p] << ",";
        }
        std::cout << ") ";
      }
      std::cout << "\n";
    } */

    textures[tex.source] = Texture(int(image.width), int(image.height), (void *)image.image.data());
  }

  return textures;
}

std::vector<std::string> getJointNames(const tinygltf::Model &tinyModel)
{

  std::vector<std::string> names;
  names.resize(tinyModel.nodes.size());
  for (int i = 0; i < tinyModel.nodes.size(); i++)
  {
    const tinygltf::Node &node = tinyModel.nodes[i];
    names[i] = node.name;
  }

  return names;
}

Pose getRestPose(const tinygltf::Model &tinyModel)
{
  Pose result;
  result.resize(tinyModel.nodes.size());

  for (int i = 0; i < tinyModel.nodes.size(); i++)
  {

    const tinygltf::Node &node = tinyModel.nodes[i];

    Transform finalTransform;
    if (node.matrix.size() != 0)
    {
      finalTransform = transformFromMat(
          Mat4x4((const float *)node.matrix.data()).transpose());
    }

    if (node.translation.size() != 0)
    {
      finalTransform.translation = Vector3f(
          node.translation[0],
          node.translation[1],
          node.translation[2]);
    }

    if (node.scale.size() != 0)
    {
      finalTransform.scaling = Vector3f(
          node.scale[0],
          node.scale[1],
          node.scale[2]);
    }

    if (node.rotation.size() != 0)
    {
      finalTransform.orientation = Quat(
          node.rotation[3],
          node.rotation[0],
          node.rotation[1],
          node.rotation[2]);
    }

    result.setLocalTransform(i, finalTransform);

    for (int j = 0; j < node.children.size(); j++)
    {
      result.setParent(node.children[j], i);
    }
  }

  return result;
}

std::vector<Mat4x4> getIverseMatrices(const tinygltf::Model &tinyModel)
{
  std::vector<Mat4x4> inverseMats;
  const tinygltf::Skin &skin = tinyModel.skins[0];

  for (int i = 0; i < tinyModel.skins[0].joints.size(); i++)
  {
    if (skin.inverseBindMatrices < 1)
    {
      std::cout << "no inverse bind matrices found\n";
      break;
    }

    const unsigned char *dataPtr = getData<unsigned char>(tinyModel, skin.inverseBindMatrices);

    size_t numMatrices = tinyModel.accessors[skin.inverseBindMatrices].count;

    for (size_t j = 0; j < numMatrices; j++)
    {
      Mat4x4 matrixData;

      std::memcpy(&matrixData.rc[0][0], dataPtr + i * 16 * sizeof(float), 16 * sizeof(float));

      inverseMats.push_back(matrixData);
    }
  }
  return inverseMats;
}

Skeleton GLTFFile::getSkeleton()
{
  Skeleton result;

  result.jointNames = getJointNames(this->tinyModel);
  result.inversePose = getIverseMatrices(this->tinyModel);
  result.restPose = getRestPose(this->tinyModel);

  return result;
}

void editTrack(
    const tinygltf::Model &tinyModel,
    const tinygltf::AnimationSampler &animSampler,
    const tinygltf::AnimationChannel &channel,
    TransformTrack &track)
{

  const std::string path = channel.target_path;

  const float *timeData = getData<float>(tinyModel, animSampler.input);
  const float *valueData = getData<float>(tinyModel, animSampler.output);

  std::cout << "channel target: " << channel.target_node << "\n";

  for (int j = 0; j < tinyModel.accessors[animSampler.input].count; j++)
  {

    if (channel.target_path == "translation")
    {
      Frame<3> frame = {
          .m_value = {
              valueData[j * 3 + 0],
              valueData[j * 3 + 1],
              valueData[j * 3 + 2]},
          .time = timeData[j],
      };

      track.getPosTrack().frames.push_back(frame);
    }
    else if (channel.target_path == "rotation")
    {
      Frame<4> frame = {
          .m_value = {
              valueData[j * 4 + 3],
              valueData[j * 4 + 0],
              valueData[j * 4 + 1],
              valueData[j * 4 + 2]},
          .time = timeData[j],

      };
      track.getRotationTrack().frames.push_back(frame);
    }
    else if (channel.target_path == "scale")
    {
      Frame<3> frame = {
          .m_value = {
              valueData[j * 3 + 0],
              valueData[j * 3 + 1],
              valueData[j * 3 + 2]},
          .time = timeData[j]

      };
      track.getScalingTrack().frames.push_back(frame);
    }
  }

  std::cout << "pos track size: " << track.getPosTrack().size() << "\n";
  std::cout << "pos rotation size: " << track.getRotationTrack().size() << "\n";
  std::cout << "pos scaling size: " << track.getScalingTrack().size() << "\n";
}

Clip getClip(const tinygltf::Model &tinyModel, const tinygltf::Animation &animation)
{
  Clip clip;

  std::cout << "number of raw channels: " << animation.channels.size() << std::endl;

  for (int i = 0; i < animation.channels.size(); i++)
  {
    const tinygltf::AnimationChannel &channel = animation.channels[i];
    const tinygltf::AnimationSampler &animSampler = animation.samplers[channel.sampler];

    if (channel.target_node < -1)
      continue;

    bool exists = false;
    for (int joint = 0; joint < clip.size(); joint++)
    {
      if (clip.getTrack(joint).getId() == channel.target_node)
      {
        editTrack(tinyModel, animSampler, channel, clip.getTrack(joint));
        exists = true;
        break;
      }
    }

    if (!exists)
    {
      TransformTrack jointTrack;
      jointTrack.setId(channel.target_node);
      editTrack(tinyModel, animSampler, channel, jointTrack);
      clip.getTracks().push_back(jointTrack);
    }
  }

  std::cout << "number of tracks: " << clip.size() << std::endl;
  clip.ReCalculateDuartion();
  return clip;
}

std::vector<Clip> GLTFFile::getClips()
{
  std::vector<Clip> clips;
  for (int i = 0; i < this->tinyModel.animations.size(); i++)
  {
    const tinygltf::Animation &animation = this->tinyModel.animations[i];
    clips.push_back(getClip(this->tinyModel, animation));
  }

  return clips;
}

template <typename T>
const T *getData(const tinygltf::Model &tinyModel, const int index)
{
  const tinygltf::Accessor &dataAccessor = tinyModel.accessors[index];
  const tinygltf::BufferView &dataBufferview = tinyModel.bufferViews[dataAccessor.bufferView];
  const tinygltf::Buffer &dataBuffer = tinyModel.buffers[dataBufferview.buffer];

  return reinterpret_cast<const T *>(
      &dataBuffer.data[dataBufferview.byteOffset + dataAccessor.byteOffset]);
}