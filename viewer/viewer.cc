#include "viewer.h"
#include "../renderer/renderer.h"
#include "../foreign/gltf.h"
#include "../foreign/obj.h"
#include "../animation/animation.h"
/*
#include <dirent.h> */

Viewer::Viewer()
    : camera(new Camera()),
      currModel("None"),
      lightDir(Vector3f(0.5, -0.5, 0.5)),
      phongStatic(nullptr),
      phongAnimated(nullptr),
      pbrStatic(nullptr),
      pbrAnimated(nullptr) {}

Viewer::~Viewer()
{
  delete this->camera;
  for (auto &model : models)
  {
    model.second->clean();
    delete model.second;
  }
}

void Viewer::init()
{
  this->phongStatic = new Shader("shaders/shader.vs", "shaders/shader.fs");
}

void Viewer::addModel(std::string name, std::string path)
{
  // struct dirent *entry = nullptr;
  GLTFFile file = GLTFFile(path);
  Model *model = new Model();
  model->meshes = file.meshes;
  model->scale(Vector3f(0.5));
  model->orient(Quat(180.0, Vector3f(0.0, 1.0, 0.0)));
  model->translate(Vector3f(0.0, 0.0, 10.0));
  this->models.insert(std::make_pair(name, model));
}

void Viewer::update(float ratio)
{

  this->phongStatic->use();
  this->phongStatic->updateVec3("lightDirection", this->lightDir);
  this->phongStatic->updateVec3("viewPos", this->camera->pos);
  this->phongStatic->updateMat4("view", this->camera->view());
  this->phongStatic->updateMat4("projection", this->camera->projection(ratio));
}

void Viewer::renderCurrModel()
{
  this->phongStatic->use();
  this->phongStatic->updateInt("textured", false);
  if (this->currModel != "None")
  {
    this->phongStatic->updateVec3("inColor", this->models[this->currModel]->color);
    this->phongStatic->updateMat4("transform", this->models[this->currModel]->get_transform());
    this->models[this->currModel]->render();
  }
}