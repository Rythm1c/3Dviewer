
class App
{
public:
  App();
  ~App() {}

  void run();

private:
  class Viewer *viewer;
  class Window *window;
  bool running;

  float delta;
  float fps;

  const unsigned char *keyboardState;

  void init();
  void handelInput();
  void calcFps();
};