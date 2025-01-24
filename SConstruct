env = Environment(
    CC=["g++"],
    CCFLAGS=[
        "-std=c++23",
        "-O2",
    ],
)

# env.ParseConfig("pkg-config --cflags --libs sdl2 gl glew")

# Progress("Evaluating $TARGET\n")

env.Program(
    LIBS=[
        "SDL2",
        "GL",
        "GLEW",
    ],
    source=[
        "main.cc",
        Glob("app/*.cc"),
        Glob("math/*.cc"),
        Glob("viewer/*.cc"),
        # Glob("imgui/*.cpp"),
        Glob("model/.cc"),
        Glob("model/renderer/*.cc"),
        Glob("model/animation/*.cc"),
        Glob("model/foreign/*.cc"),
        Glob("imgui/*.cpp"),
    ],
    target="model_viewer",
)
