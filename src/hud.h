#pragma once
#include <string>
#include "camera.h"
#include "lighting.h"
#include "scene.h"

class HUD {
public:
    bool showHelp;

    HUD();

    void toggleHelp();

    void render(int windowWidth, int windowHeight,
                const Camera& camera,
                const LightingManager& lighting,
                const Scene& scene);

private:
    void drawText(float x, float y, const std::string& text, void* font = nullptr);
    void drawPanel(float x, float y, float w, float h, float r, float g, float b, float a);
};
