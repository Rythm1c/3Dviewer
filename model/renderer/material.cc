#include "material.h"
#include "texture.h"
#include "shader.h"

void Material::configShader(Shader &shader)
{
  shader.updateFloat("roughness", this->roughness);
  shader.updateFloat("metallic", this->metallicness);
  shader.updateVec3("baseColor", this->baseCol);
  shader.updateInt("hasAlbedoMap", (this->baseTex != -1));
  shader.updateInt("hasMetallicMap", (this->metallicMap != -1));
}
