#include "gltf.h"
#include "../renderer/mesh.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "tiny_gltf.h"

GLTFFile::GLTFFile(std::string path)
{
  tinygltf::Model model;
  tinygltf::TinyGLTF loader;
  std::string err, warn;

  if (!loader.LoadASCIIFromFile(&model, &err, &warn, path))
    throw std::runtime_error(warn + err);

  Vertex vertex = {};
  Mesh tmpmesh = {};
  tmpmesh.mode = TRIANGLES;

  for (size_t m = 0; m < model.meshes.size(); ++m)
  {
    tinygltf::Mesh &mesh = model.meshes[m];

    for (size_t j = 0; j < mesh.primitives.size(); ++j)
    {
      tmpmesh.vertices.clear();
      tmpmesh.indices.clear();

      tinygltf::Primitive &primitive = mesh.primitives[j];
      // positions
      const tinygltf::Accessor &posAccessor =
          model.accessors[primitive.attributes["POSITION"]];
      const tinygltf::BufferView &posBufferview =
          model.bufferViews[posAccessor.bufferView];
      const tinygltf::Buffer &posBuffer = model.buffers[posBufferview.buffer];
      const float *positions = reinterpret_cast<const float *>(
          &posBuffer.data[posBufferview.byteOffset + posAccessor.byteOffset]);

      // normals
      const tinygltf::Accessor &normAccessor =
          model.accessors[primitive.attributes["NORMAL"]];
      const tinygltf::BufferView &normBufferview =
          model.bufferViews[normAccessor.bufferView];
      const tinygltf::Buffer &normBuffer = model.buffers[normBufferview.buffer];
      const float *normals = reinterpret_cast<const float *>(
          &normBuffer
               .data[normBufferview.byteOffset + normAccessor.byteOffset]);

      // indices
      const tinygltf::Accessor &indAccessor =
          model.accessors[primitive.indices];
      const tinygltf::BufferView &indBufferview =
          model.bufferViews[indAccessor.bufferView];
      const tinygltf::Buffer &indBuffer = model.buffers[indBufferview.buffer];
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
}
