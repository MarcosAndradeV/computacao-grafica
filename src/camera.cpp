#include "camera.h"
#include <algorithm>
#include <cmath>

constexpr float DEG2RAD_LOCAL = 3.14159265358979323846f / 180.0f;

GalleryCamera::GalleryCamera() {
    projectionMode = ProjectionMode::PERSPECTIVE;
    reset();
}

void GalleryCamera::reset() {
    targetX = defaultTargetX;
    targetY = defaultTargetY;
    targetZ = defaultTargetZ;
    azimuthDeg = defaultAzimuth;
    elevationDeg = defaultElevation;
    distance = defaultDistance;
    projectionMode = ProjectionMode::PERSPECTIVE;
}

void GalleryCamera::rotate(float deltaAzimuth, float deltaElevation) {
    azimuthDeg += deltaAzimuth;
    elevationDeg += deltaElevation;

    // Manter azimute em [0, 360)
    azimuthDeg = std::fmod(azimuthDeg, 360.0f);
    if (azimuthDeg < 0.0f)
        azimuthDeg += 360.0f;

    // Limitar elevação para evitar inversão
    elevationDeg = std::clamp(elevationDeg, -85.0f, 85.0f);
}

void GalleryCamera::zoom(float deltaDistance) {
    distance += deltaDistance;
    distance = std::clamp(distance, 3.0f, 35.0f);
}

void GalleryCamera::pan(float deltaX, float deltaY) {
    float radAz = azimuthDeg * DEG2RAD_LOCAL;
    float cosAz = std::cos(radAz);
    float sinAz = std::sin(radAz);

    // Vetor lateral no plano XZ
    float rightX = cosAz;
    float rightZ = -sinAz;

    targetX += rightX * deltaX;
    targetZ += rightZ * deltaX;
    targetY += deltaY;
}

void GalleryCamera::toggleProjection() {
    if (projectionMode == ProjectionMode::PERSPECTIVE) {
        projectionMode = ProjectionMode::ORTHOGRAPHIC;
    } else {
        projectionMode = ProjectionMode::PERSPECTIVE;
    }
}

Vector3 GalleryCamera::getPosition() const {
    float radAz = azimuthDeg * DEG2RAD_LOCAL;
    float radEl = elevationDeg * DEG2RAD_LOCAL;

    float eyeX = targetX + distance * std::cos(radEl) * std::sin(radAz);
    float eyeY = targetY + distance * std::sin(radEl);
    float eyeZ = targetZ + distance * std::cos(radEl) * std::cos(radAz);

    return {eyeX, eyeY, eyeZ};
}

Camera3D GalleryCamera::getRaylibCamera() const {
    Camera3D cam;
    cam.position = getPosition();
    cam.target = {targetX, targetY, targetZ};
    cam.up = {0.0f, 1.0f, 0.0f};

    if (projectionMode == ProjectionMode::PERSPECTIVE) {
        cam.fovy = 45.0f;
        cam.projection = CAMERA_PERSPECTIVE;
    } else {
        cam.fovy = distance * 0.75f;
        cam.projection = CAMERA_ORTHOGRAPHIC;
    }

    return cam;
}

const char *GalleryCamera::getProjectionName() const {
    return (projectionMode == ProjectionMode::PERSPECTIVE) ? "PERSPECTIVA (Raylib 3D)" : "ORTOGRAFICA (Raylib 3D)";
}
