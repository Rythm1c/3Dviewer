
#include "app.h"
#include "window.h"
#include "../viewer/viewer.h"

#include <chrono>

static auto lastFrameDuration = std::chrono::high_resolution_clock::now();

App::App()
    : viewer(nullptr),
      window(nullptr),
      running(true),
      delta(0.0),
      fps(0.0),
      keyboardState(nullptr)
{
}

void App::init()
{

  this->window = new Window();
  this->window->init();

  this->viewer = new Viewer();
  this->viewer->init();
  this->viewer->addModel("astronaut", "models/astronaut/scene.gltf");
  this->viewer->currModel = "astronaut";

  int nkeys;
  this->keyboardState = SDL_GetKeyboardState(&nkeys);
}

void App::run()
{
  this->init();
  while (this->running)
  {
    this->calcFps();
    this->handelInput();

    this->window->clear(0.8, 0.2, 0.2);
    this->viewer->update(this->window->ratio());
    this->viewer->renderCurrModel();
    this->window->swapBuffer();
  }
}
void App::calcFps()
{
  auto now = std::chrono::high_resolution_clock::now();
  this->delta = std::chrono::duration<float, std::chrono::seconds::period>(
                    now - lastFrameDuration)
                    .count();
  lastFrameDuration = now;
  this->fps = 1.0 / this->delta;
}
void App::handelInput()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {

    switch (event.type)
    {
    case SDL_QUIT:
      this->running = false;
      break;

    case SDL_KEYDOWN:
      // handle special key stokes
      switch (event.key.keysym.sym)
      {
      }
      break;

    case SDL_KEYUP:
      break;

    case SDL_MOUSEMOTION:
      if (event.button.button == SDL_BUTTON_LEFT)
      {
        this->viewer->camera->rotation(
            event.motion.xrel, event.motion.yrel);
      }
      break;

    case SDL_WINDOWEVENT:
      switch (event.window.event)
      {
      case SDL_WINDOWEVENT_SIZE_CHANGED:
        this->window->reSize();
        break;

      default:
        break;
      }
      break;

    default:
      break;
    }
  }

  if (this->keyboardState[SDL_SCANCODE_W])
  {
    this->viewer->camera->moveForwards(this->delta);
  }
  if (this->keyboardState[SDL_SCANCODE_S])
  {
    this->viewer->camera->moveBackwards(this->delta);
  }
  if (this->keyboardState[SDL_SCANCODE_D])
  {
    this->viewer->camera->moveRight(this->delta);
  }
  if (this->keyboardState[SDL_SCANCODE_A])
  {
    this->viewer->camera->moveLeft(this->delta);
  }
  // for zooming in and out
  if (this->keyboardState[SDL_SCANCODE_Z])
  {
    this->viewer->camera->fov++;
  }
  else if (this->keyboardState[SDL_SCANCODE_X])
  {
    this->viewer->camera->fov--;
  }
}