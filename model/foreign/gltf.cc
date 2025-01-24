#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "gltf.h"
#include "../renderer/mesh.h"
#include "../model.h"



GLTFFile::GLTFFile(std::string path)
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
  model.textures = this->getTextures();
}

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
      const tinygltf::Accessor &posAccessor =
          this->tinyModel.accessors[primitive.attributes["POSITION"]];
      const tinygltf::BufferView &posBufferview =
          this->tinyModel.bufferViews[posAccessor.bufferView];
      const tinygltf::Buffer &posBuffer = this->tinyModel.buffers[posBufferview.buffer];
      const float *positions = reinterpret_cast<const float *>(
          &posBuffer.data[posBufferview.byteOffset + posAccessor.byteOffset]);

      // normals
      const tinygltf::Accessor &normAccessor =
          this->tinyModel.accessors[primitive.attributes["NORMAL"]];
      const tinygltf::BufferView &normBufferview =
          this->tinyModel.bufferViews[normAccessor.bufferView];
      const tinygltf::Buffer &normBuffer = this->tinyModel.buffers[normBufferview.buffer];
      const float *normals = reinterpret_cast<const float *>(
          &normBuffer
               .data[normBufferview.byteOffset + normAccessor.byteOffset]);

      // indices
      const tinygltf::Accessor &indAccessor =
          this->tinyModel.accessors[primitive.indices];
      const tinygltf::BufferView &indBufferview =
          this->tinyModel.bufferViews[indAccessor.bufferView];
      const tinygltf::Buffer &indBuffer = this->tinyModel.buffers[indBufferview.buffer];
      const uint *_indices = reinterpret_cast<const uint *>(
          &indBuffer.data[indBufferview.byteOffset + indAccessor.byteOffset]);

      for (size_t i = 0; i < posAccessor.count; ++i)
      {
        vertex.pos = {positions[i * 3 + 0], positions[i * 3 + 1],
                      positions[i * 3 + 2]};
        vertex.norm = {normals[i * 3 + 0], normals[i * 3 + 1],
                       normals[i * 3 + 2]};

        tmpmesh.vertices.push_back(vertex);
      }
      for (size_t i = 0; i < indAccessor.count; ++i)
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
  for (int i = 0; i < this->tinyModel.textures.size(); i++)
  {
    tinygltf::Texture &tex = this->tinyModel.textures[i];
    tinygltf::Image &image = this->tinyModel.images[tex.source];

    textures.push_back(
        Texture(int(image.width),
                int(image.height),
                (void *)image.image.data()));
  }

  return textures;
}