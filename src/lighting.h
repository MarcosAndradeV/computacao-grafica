#pragma once
#include "material.h"
#include <raylib.h>

class LightingManager {
  public:
    bool directionalEnabled;
    bool positionalEnabled;
    bool ambientEnabled;
    bool diffuseEnabled;
    bool specularEnabled;

    Vector3 directionalDir;
    Vector3 positionalPos;

    const Vector3 defaultPositionalPos = {0.0f, 3.8f, 0.0f};
    const Vector3 defaultDirectionalDir = {1.0f, 2.0f, 1.2f};

    Shader shader;

    int uDirLightEnabledLoc;
    int uPosLightEnabledLoc;
    int uAmbientEnabledLoc;
    int uDiffuseEnabledLoc;
    int uSpecularEnabledLoc;
    int uDirLightDirLoc;
    int uPosLightPosLoc;
    int uViewPosLoc;
    int uMaterialAmbientLoc;
    int uMaterialSpecularLoc;
    int uMaterialShininessLoc;

    LightingManager();

    void init();
    void unload();
    void reset();

    void toggleDirectional();
    void togglePositional();
    void toggleAmbient();
    void toggleDiffuse();
    void toggleSpecular();

    void translatePositionalLight(float dx, float dy, float dz);

    void update(Vector3 cameraPos);
    void applyMaterial(const MaterialData &mat);

    void renderLightMarker(bool isSelected);
};
