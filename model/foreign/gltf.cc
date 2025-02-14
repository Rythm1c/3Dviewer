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
      auto it = primitive.attributes.find("POSITION");
      if (it != primitive.attributes.end())
      {
        const float *positions = getData<float>(this->tinyModel, it->second);
        int count = tinyModel.accessors[it->second].count;

        for (size_t i = 0; i < count; ++i)
        {
          Vertex vertex = {
              .pos = Vector3f(
                  positions[i * 3 + 0],
                  positions[i * 3 + 1],
                  positions[i * 3 + 2]),
          };
          tmpmesh.vertices.push_back(vertex);
        }
      }
      else
      {
        std::cout << "no position attribute found in primitive " << j << " of mesh " << m << "\n";
      }

      // normals
      it = primitive.attributes.find("NORMAL");
      if (it != primitive.attributes.end())
      {
        const float *normals = getData<float>(this->tinyModel, it->second);
        int count = tinyModel.accessors[it->second].count;

        for (size_t i = 0; i < count; ++i)
        {

          tmpmesh.vertices[i].norm = Vector3f(
              normals[i * 3 + 0],
              normals[i * 3 + 1],
              normals[i * 3 + 2]);
        }
      }
      else
      {
        std::cout << "no normal attribute found in primitive " << j << " of mesh " << m << "\n";
      }

      // texture coords
      it = primitive.attributes.find("TEXCOORD_0");
      if (it != primitive.attributes.end())
      {
        const float *uvs = getData<float>(this->tinyModel, it->second);
        int count = tinyModel.accessors[it->second].count;

        for (size_t i = 0; i < count; ++i)
        {

          tmpmesh.vertices[i].tc = Vector2f(
              uvs[i * 2 + 0],
              uvs[i * 2 + 1]);
        }
      }
      else
      {
        std::cout << "no texture coordinate attribute found in primitive " << j << " of mesh " << m << "\n";
      }

      it = primitive.attributes.find("JOINTS_0");
      if (it != primitive.attributes.end())
      {
        const unsigned short *joints = getData<unsigned short>(this->tinyModel, it->second);
        int count = tinyModel.accessors[it->second].count;

        std::vector<int> skinjoints;
        skinjoints = tinyModel.skins[0].joints;

        for (size_t i = 0; i < count; ++i)
        {

          tmpmesh.vertices[i].joints[0] = skinjoints[joints[i * 4 + 0]];
          tmpmesh.vertices[i].joints[1] = skinjoints[joints[i * 4 + 1]];
          tmpmesh.vertices[i].joints[2] = skinjoints[joints[i * 4 + 2]];
          tmpmesh.vertices[i].joints[3] = skinjoints[joints[i * 4 + 3]];
        };
      }
      else
      {
        std::cout << "no joints attribute found in primitive " << j << " of mesh " << m << "\n";
      }

      it = primitive.attributes.find("WEIGHTS_0");
      if (it != primitive.attributes.end())
      {
        const float *weights = getData<float>(this->tinyModel, it->second);
        int count = tinyModel.accessors[it->second].count;

        for (size_t i = 0; i < count; ++i)
        {

          tmpmesh.vertices[i].weights[0] = weights[i * 4 + 0];
          tmpmesh.vertices[i].weights[1] = weights[i * 4 + 1];
          tmpmesh.vertices[i].weights[2] = weights[i * 4 + 2];
          tmpmesh.vertices[i].weights[3] = weights[i * 4 + 3];
        };
      }
      else
      {
        std::cout << "no weights attributes found in primitive " << j << " of mesh " << m << "\n";
      }

      if (primitive.indices >= 0)
      {
        const uint *indices = getData<uint>(tinyModel, primitive.indices);
        int count = tinyModel.accessors[primitive.indices].count;
        for (int i = 0; i < count; ++i)
        {
          tmpmesh.indices.push_back(indices[i]);
        }
      }

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
      const std::vector<double> &m = node.matrix;
      finalTransform = transformFromMat(
          Mat4x4(
              m[0], m[1], m[2], m[3],
              m[4], m[5], m[6], m[7],
              m[8], m[9], m[10], m[11],
              m[12], m[13], m[14], m[15])
              .transpose());
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
          node.rotation[0],
          node.rotation[1],
          node.rotation[2],
          node.rotation[3]);
    }

    result.setLocalTransform(i, finalTransform);
    /* Mat4x4 mat = finalTransform.get();
     std::cout << "joint index: " << i << "\n";
     std::cout << mat.rc[0][0] << " " << mat.rc[0][1] << " " << mat.rc[0][2] << " " << mat.rc[0][3] << "\n";

     std::cout << mat.rc[1][0] << " " << mat.rc[1][1] << " " << mat.rc[1][2] << " " << mat.rc[1][3] << "\n";

     std::cout << mat.rc[2][0] << " " << mat.rc[2][1] << " " << mat.rc[2][2] << " " << mat.rc[2][3] << "\n";

     std::cout << mat.rc[3][0] << " " << mat.rc[3][1] << " " << mat.rc[3][2] << " " << mat.rc[3][3] << "\n"; */

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
  inverseMats.resize(tinyModel.nodes.size(), identity());

  const tinygltf::Skin &skin = tinyModel.skins[0];

  if (skin.inverseBindMatrices < 1)
  {
    std::cout << "no inverse bind matrices found!\n";
    return inverseMats;
  }
  const float *data = getData<float>(tinyModel, skin.inverseBindMatrices);

  for (int j = 0; j < skin.joints.size(); j++)
  {
    int index = skin.joints[j];
    /*   inverseMats[index] = tmpMatrixData[i].transpose(); */
    inverseMats[index] =
        Mat4x4(
            data[j * 16 + 0], data[j * 16 + 1], data[j * 16 + 2], data[j * 16 + 3],
            data[j * 16 + 4], data[j * 16 + 5], data[j * 16 + 6], data[j * 16 + 7],
            data[j * 16 + 8], data[j * 16 + 9], data[j * 16 + 10], data[j * 16 + 11],
            data[j * 16 + 12], data[j * 16 + 13], data[j * 16 + 14], data[j * 16 + 15])
            .transpose();

    /* std::cout << "joint index: " << index << "\n";
      std::cout << inverseMats[index].rc[0][0] << " " << inverseMats[index].rc[0][1] << " " << inverseMats[index].rc[0][2] << " " << inverseMats[index].rc[0][3] << "\n";

      std::cout << inverseMats[index].rc[1][0] << " " << inverseMats[index].rc[1][1] << " " << inverseMats[index].rc[1][2] << " " << inverseMats[index].rc[1][3] << "\n";

      std::cout << inverseMats[index].rc[2][0] << " " << inverseMats[index].rc[2][1] << " " << inverseMats[index].rc[2][2] << " " << inverseMats[index].rc[2][3] << "\n";

      std::cout << inverseMats[index].rc[3][0] << " " << inverseMats[index].rc[3][1] << " " << inverseMats[index].rc[3][2] << " " << inverseMats[index].rc[3][3] << "\n"; */
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

  int count = tinyModel.accessors[animSampler.input].count;

  // std::cout << "channel target: " << channel.target_node << "\n";

  for (int j = 0; j < count; j++)
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
              valueData[j * 4 + 0],
              valueData[j * 4 + 1],
              valueData[j * 4 + 2],
              valueData[j * 4 + 3]},
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
}

Clip getClip(const tinygltf::Model &tinyModel, const tinygltf::Animation &animation)
{
  Clip clip;

  for (int i = 0; i < animation.channels.size(); i++)
  {
    const tinygltf::AnimationChannel &channel = animation.channels[i];
    const tinygltf::AnimationSampler &animSampler = animation.samplers[channel.sampler];

    if (channel.target_node < 0)
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

  // std::cout << "number of tracks: " << clip.size() << std::endl;
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

  return reinterpret_cast<const T *>(&dataBuffer.data[dataBufferview.byteOffset + dataAccessor.byteOffset]);
}