#pragma once
#include <string>
#include <raylib.h>
#include "material.h"

class LightingManager;

enum class ArtObjectType {
    KNOT,    // Nó Escultural (Substitui Teapot mantendo a complexidade artística)
    SPHERE,  // Esfera
    TORUS,   // Toro
    CONE     // Cone
};

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

    ArtObject(int id, const std::string& name, const std::string& materialName,
              ArtObjectType type, const MaterialData& mat, const Transform3D& initialTrans);

    void init();
    void unload();

    void translate(float dx, float dy, float dz);
    void rotate(float rx, float ry, float rz);
    void changeScale(float deltaScale);
    void reset();

    void render(bool isSelected, LightingManager& lighting);

private:
    void drawSelectionHighlight();
};
