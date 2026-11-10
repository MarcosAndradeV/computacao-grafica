#pragma once
#include "material.h"
#include <raylib.h>
#include <string>

class LightingManager;

enum class ArtObjectType { KNOT, SPHERE, TORUS, CONE };

struct Transform3D {
    float posX = 0.0f;
    float posY = 0.0f;
    float posZ = 0.0f;
    float rotX = 0.0f;
    float rotY = 0.0f;
    float rotZ = 0.0f;
    float scale = 1.0f;
};

class ArtObject {
  public:
    int id;
    std::string name;
    std::string materialName;
    ArtObjectType type;
    MaterialData material;

    Transform3D currentTransform;
    Transform3D initialTransform;

    Model model;

    ArtObject(int id,
              const std::string &name,
              const std::string &materialName,
              ArtObjectType type,
              const MaterialData &mat,
              const Transform3D &initialTrans);

    void init();
    void unload();

    void translate(float dx, float dy, float dz);
    void rotate(float rx, float ry, float rz);
    void changeScale(float deltaScale);
    void reset();

    void render(bool isSelected, LightingManager &lighting);

  private:
    void drawSelectionHighlight();
};
