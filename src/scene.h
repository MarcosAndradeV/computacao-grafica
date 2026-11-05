#pragma once
#include <vector>
#include <raylib.h>
#include "object.h"
#include "lighting.h"

class Scene {
public:
    std::vector<ArtObject> objects;
    int selectedEntity; // 0 = Luz Posicional, 1..4 = Objetos 1..4

    Model cubeModel; // Modelo compartilhado para renderizar sala, piso e pedestais com iluminação

    Scene();

    void init();
    void unload();
    void reset();

    void selectEntity(int entityIndex);
    void translateSelected(float dx, float dy, float dz, LightingManager& lighting);
    void rotateSelected(float rx, float ry, float rz);
    void scaleSelected(float deltaScale);

    void render(LightingManager& lighting);

private:
    void renderRoom(LightingManager& lighting);
    void renderPedestal(float posX, float posZ, LightingManager& lighting);
    void drawIlluminatedBox(Vector3 pos, Vector3 size, const MaterialData& mat, LightingManager& lighting);
};
