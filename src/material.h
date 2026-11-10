#pragma once
#include <raylib.h>

struct MaterialData {
    Vector4 ambient;
    Vector4 diffuse;
    Vector4 specular;
    float shininess;
    Color baseColor;
};

namespace Materials {

inline MaterialData Gold() {
    return {{0.24725f, 0.1995f, 0.0745f, 1.0f},
            {0.75164f, 0.60648f, 0.22648f, 1.0f},
            {0.628281f, 0.555802f, 0.366065f, 1.0f},
            51.2f,
            {218, 165, 32, 255}};
}

inline MaterialData Ruby() {
    return {{0.1745f, 0.01175f, 0.01175f, 1.0f},
            {0.61424f, 0.04136f, 0.04136f, 1.0f},
            {0.727811f, 0.626959f, 0.626959f, 1.0f},
            76.8f,
            {220, 20, 60, 255}};
}

inline MaterialData Emerald() {
    return {{0.0215f, 0.1745f, 0.0215f, 1.0f}, {0.07568f, 0.61424f, 0.07568f, 1.0f}, {0.633f, 0.727811f, 0.633f, 1.0f}, 76.8f, {0, 201, 87, 255}};
}

inline MaterialData Chrome() {
    return {{0.25f, 0.25f, 0.25f, 1.0f}, {0.4f, 0.4f, 0.4f, 1.0f}, {0.774597f, 0.774597f, 0.774597f, 1.0f}, 89.6f, {220, 220, 225, 255}};
}

inline MaterialData Pedestal() {
    return {{0.12f, 0.12f, 0.14f, 1.0f}, {0.35f, 0.35f, 0.38f, 1.0f}, {0.2f, 0.2f, 0.22f, 1.0f}, 20.0f, {70, 70, 75, 255}};
}

inline MaterialData FloorWhite() {
    return {{0.2f, 0.2f, 0.2f, 1.0f}, {0.8f, 0.8f, 0.82f, 1.0f}, {0.15f, 0.15f, 0.15f, 1.0f}, 10.0f, {225, 225, 230, 255}};
}

inline MaterialData FloorDark() {
    return {{0.05f, 0.05f, 0.06f, 1.0f}, {0.18f, 0.18f, 0.20f, 1.0f}, {0.1f, 0.1f, 0.1f, 1.0f}, 10.0f, {45, 45, 50, 255}};
}

inline MaterialData Wall() {
    return {{0.15f, 0.15f, 0.16f, 1.0f}, {0.65f, 0.65f, 0.68f, 1.0f}, {0.1f, 0.1f, 0.1f, 1.0f}, 5.0f, {180, 180, 185, 255}};
}

inline MaterialData WoodTrim() {
    return {{0.12f, 0.06f, 0.02f, 1.0f}, {0.35f, 0.18f, 0.08f, 1.0f}, {0.15f, 0.15f, 0.15f, 1.0f}, 15.0f, {110, 55, 25, 255}};
}

} // namespace Materials
