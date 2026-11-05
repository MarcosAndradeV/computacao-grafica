#include "camera.h"
#include <cmath>
#include <algorithm>

namespace {
    constexpr float DEG2RAD = 3.14159265358979323846f / 180.0f;
}

Camera::Camera() {
    projectionMode = ProjectionMode::PERSPECTIVE;
    reset();
}

void Camera::reset() {
    targetX = defaultTargetX;
    targetY = defaultTargetY;
    targetZ = defaultTargetZ;
    azimuthDeg = defaultAzimuth;
    elevationDeg = defaultElevation;
    distance = defaultDistance;
    projectionMode = ProjectionMode::PERSPECTIVE;
}

void Camera::rotate(float deltaAzimuth, float deltaElevation) {
    azimuthDeg += deltaAzimuth;
    elevationDeg += deltaElevation;

    // Manter azimute em [0, 360)
    azimuthDeg = std::fmod(azimuthDeg, 360.0f);
    if (azimuthDeg < 0.0f) azimuthDeg += 360.0f;

    // Limitar elevação para evitar inversão da câmera (gimbal lock no zênite/nadir)
    elevationDeg = std::clamp(elevationDeg, -85.0f, 85.0f);
}

void Camera::zoom(float deltaDistance) {
    distance += deltaDistance;
    distance = std::clamp(distance, 3.0f, 35.0f);
}

void Camera::pan(float deltaX, float deltaY) {
    // Deslocar o ponto de observação no plano perpendicular à linha de visão
    float radAz = azimuthDeg * DEG2RAD;
    float cosAz = std::cos(radAz);
    float sinAz = std::sin(radAz);

    // Vetor lateral (Right) no plano XZ
    float rightX = cosAz;
    float rightZ = -sinAz;

    targetX += rightX * deltaX;
    targetZ += rightZ * deltaX;
    targetY += deltaY;
}

void Camera::toggleProjection() {
    if (projectionMode == ProjectionMode::PERSPECTIVE) {
        projectionMode = ProjectionMode::ORTHOGRAPHIC;
    } else {
        projectionMode = ProjectionMode::PERSPECTIVE;
    }
}

void Camera::applyProjection(int windowWidth, int windowHeight) const {
    if (windowHeight <= 0) windowHeight = 1;
    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (projectionMode == ProjectionMode::PERSPECTIVE) {
        // Campo de visão vertical 45 graus, plano próximo 0.5, distante 100.0
        gluPerspective(45.0, aspect, 0.5, 100.0);
    } else {
        // Projeção Ortográfica dimensionada proporcionalmente à distância (zoom)
        float orthoHeight = distance * 0.45f;
        float orthoWidth = orthoHeight * aspect;
        glOrtho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, -50.0, 100.0);
    }

    glMatrixMode(GL_MODELVIEW);
}

void Camera::applyView() const {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float radAz = azimuthDeg * DEG2RAD;
    float radEl = elevationDeg * DEG2RAD;

    // Conversão de coordenadas esféricas para cartesianas
    float eyeX = targetX + distance * std::cos(radEl) * std::sin(radAz);
    float eyeY = targetY + distance * std::sin(radEl);
    float eyeZ = targetZ + distance * std::cos(radEl) * std::cos(radAz);

    gluLookAt(eyeX, eyeY, eyeZ,
              targetX, targetY, targetZ,
              0.0f, 1.0f, 0.0f);
}

const char* Camera::getProjectionName() const {
    return (projectionMode == ProjectionMode::PERSPECTIVE) ? "PERSPECTIVA (gluPerspective)" : "ORTOGRAFICA (glOrtho)";
}
