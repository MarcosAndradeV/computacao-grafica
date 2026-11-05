#pragma once
#include <GL/gl.h>

class LightingManager {
public:
    // Toggles de luzes e componentes (F1 - F5)
    bool directionalEnabled;  // F1
    bool positionalEnabled;   // F2
    bool ambientEnabled;      // F3
    bool diffuseEnabled;      // F4
    bool specularEnabled;     // F5

    // Posições
    GLfloat directionalDir[4]; // Luz distante (w = 0.0)
    GLfloat positionalPos[4];  // Luz próxima (w = 1.0)

    // Posição inicial para reset
    const GLfloat defaultPositionalPos[4] = {0.0f, 3.8f, 0.0f, 1.0f};
    const GLfloat defaultDirectionalDir[4] = {1.0f, 2.0f, 1.2f, 0.0f};

    LightingManager();

    void init();
    void reset();

    void toggleDirectional();
    void togglePositional();
    void toggleAmbient();
    void toggleDiffuse();
    void toggleSpecular();

    void translatePositionalLight(float dx, float dy, float dz);

    // Deve ser chamada logo após a configuração da câmera (gluLookAt) para fixar as luzes no espaço de mundo
    void applyLighting();

    // Renderiza a lâmpada/marcador da luz posicional no espaço 3D
    void renderLightMarker(bool isSelected);
};
