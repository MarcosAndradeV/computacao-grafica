#pragma once
#include <raylib.h>

enum class ProjectionMode {
    PERSPECTIVE,
    ORTHOGRAPHIC
};

class GalleryCamera {
public:
    ProjectionMode projectionMode;

    // Ponto de observação (Target)
    float targetX, targetY, targetZ;

    // Coordenadas esféricas em relação ao alvo
    float azimuthDeg;   // Rotação horizontal (eixo Y)
    float elevationDeg; // Rotação vertical (elevação)
    float distance;     // Distância (Zoom)

    // Valores padrão para restauração (Reset)
    const float defaultTargetX = 0.0f;
    const float defaultTargetY = 1.2f;
    const float defaultTargetZ = 0.0f;
    const float defaultAzimuth = 45.0f;
    const float defaultElevation = 25.0f;
    const float defaultDistance = 14.0f;

    GalleryCamera();

    void rotate(float deltaAzimuth, float deltaElevation);
    void zoom(float deltaDistance);
    void pan(float deltaX, float deltaY);
    void toggleProjection();
    void reset();

    Camera3D getRaylibCamera() const;
    Vector3 getPosition() const;
    const char* getProjectionName() const;
};
