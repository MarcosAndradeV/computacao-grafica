#pragma once
#include "camera.h"
#include "lighting.h"
#include "scene.h"
#include <raylib.h>
#include <string>

class HUD {
  public:
    bool showHelp;

    HUD();

    void toggleHelp();

    void render(int windowWidth, int windowHeight, const GalleryCamera &camera, const LightingManager &lighting, const Scene &scene);
};
