#pragma once
#include <vector>
#include "object.h"
#include "lighting.h"

class Scene {
public:
    std::vector<ArtObject> objects;
    int selectedEntity; // 0 = Luz Posicional, 1..4 = Objetos 1..4

    Scene();

    void init();
    void reset();

    void selectEntity(int entityIndex);
    void translateSelected(float dx, float dy, float dz, LightingManager& lighting);
    void rotateSelected(float rx, float ry, float rz);
    void scaleSelected(float deltaScale);

    void render(LightingManager& lighting);

private:
    void renderRoom(bool enableAmbient, bool enableDiffuse, bool enableSpecular);
    void renderPedestal(float posX, float posZ, bool enableAmbient, bool enableDiffuse, bool enableSpecular);
};
