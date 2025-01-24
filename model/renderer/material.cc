#include "material.h"
#include "texture.h"
#include "shader.h"

void Material::configShader(Shader &shader)
{
  shader.updateFloat("roughness", this->roughness);
  shader.updateFloat("metallic", this->metallicness);
  shader.updateVec3("baseColor", this->baseCol);

  shader.updateInt("hasAlbedoMap", this->baseTex != nullptr);
  if (this->baseTex != nullptr)
  {
    // shader.updateInt("hasAlbedoMap", this->baseTex != nullptr);
  }

  shader.updateInt("hasMetallicMap", this->metallicMap != nullptr);
  if (this->metallicMap != nullptr)
  {
    this->metallicMap;
    // shader.updateInt("hasAlbedoMap", this->baseTex != nullptr);
  }
}
