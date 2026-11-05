#pragma once
#include <raylib.h>
#include "material.h"

class LightingManager {
public:
    // Toggles de luzes e componentes (F1 - F5)
    bool directionalEnabled;  // F1
    bool positionalEnabled;   // F2
    bool ambientEnabled;      // F3
    bool diffuseEnabled;      // F4
    bool specularEnabled;     // F5

    // Posições
    Vector3 directionalDir; // Luz distante
    Vector3 positionalPos;  // Luz próxima

    // Posição inicial para reset
    const Vector3 defaultPositionalPos = {0.0f, 3.8f, 0.0f};
    const Vector3 defaultDirectionalDir = {1.0f, 2.0f, 1.2f};

    Shader shader;

    // Uniform locations no shader GLSL
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
    void applyMaterial(const MaterialData& mat);

    // Renderiza a lâmpada/marcador da luz posicional no espaço 3D
    void renderLightMarker(bool isSelected);
};
