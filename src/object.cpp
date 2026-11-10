#include "object.h"
#include "lighting.h"
#include <cmath>
#include <raymath.h>

ArtObject::ArtObject(
    int id, const std::string &name, const std::string &materialName, ArtObjectType type, const MaterialData &mat, const Transform3D &initialTrans)
    : id(id), name(name), materialName(materialName), type(type), material(mat), currentTransform(initialTrans), initialTransform(initialTrans) {}

void ArtObject::init() {
    Mesh mesh;
    switch (type) {
    case ArtObjectType::KNOT:
        mesh = GenMeshKnot(0.6f, 1.4f, 32, 32);
        break;
    case ArtObjectType::SPHERE:
        mesh = GenMeshSphere(0.85f, 32, 32);
        break;
    case ArtObjectType::TORUS:
        mesh = GenMeshTorus(0.35f, 0.72f, 32, 32);
        break;
    case ArtObjectType::CONE:
        mesh = GenMeshCone(0.8f, 1.5f, 32);
        break;
    }
    model = LoadModelFromMesh(mesh);
}

void ArtObject::unload() { UnloadModel(model); }

void ArtObject::translate(float dx, float dy, float dz) {
    currentTransform.posX += dx;
    currentTransform.posY += dy;
    currentTransform.posZ += dz;
}

void ArtObject::rotate(float rx, float ry, float rz) {
    currentTransform.rotX += rx;
    currentTransform.rotY += ry;
    currentTransform.rotZ += rz;

    currentTransform.rotX = std::fmod(currentTransform.rotX, 360.0f);
    currentTransform.rotY = std::fmod(currentTransform.rotY, 360.0f);
    currentTransform.rotZ = std::fmod(currentTransform.rotZ, 360.0f);
}

void ArtObject::changeScale(float deltaScale) {
    currentTransform.scale += deltaScale;
    if (currentTransform.scale < 0.1f)
        currentTransform.scale = 0.1f;
    if (currentTransform.scale > 4.0f)
        currentTransform.scale = 4.0f;
}

void ArtObject::reset() { currentTransform = initialTransform; }

void ArtObject::render(bool isSelected, LightingManager &lighting) {
    // 1. Configurar Shader e Material
    model.materials[0].shader = lighting.shader;
    lighting.applyMaterial(material);

    // 2. Matriz de Transformação Geométrica Composta: Escala -> Rotação -> Translação
    Matrix matScale = MatrixScale(currentTransform.scale, currentTransform.scale, currentTransform.scale);
    Matrix matRot = MatrixRotateXYZ({currentTransform.rotX * DEG2RAD, currentTransform.rotY * DEG2RAD, currentTransform.rotZ * DEG2RAD});
    Matrix matTrans = MatrixTranslate(currentTransform.posX, currentTransform.posY, currentTransform.posZ);

    model.transform = MatrixMultiply(MatrixMultiply(matScale, matRot), matTrans);

    // 3. Renderizar Modelo 3D
    DrawModel(model, {0.0f, 0.0f, 0.0f}, 1.0f, material.baseColor);

    // 4. Indicador de Seleção
    if (isSelected) {
        drawSelectionHighlight();
    }
}

void ArtObject::drawSelectionHighlight() {
    Vector3 center = {currentTransform.posX, currentTransform.posY - 0.65f * currentTransform.scale, currentTransform.posZ};
    DrawCircle3D(center, 1.35f * currentTransform.scale, {0.0f, 1.0f, 0.0f}, 90.0f, GOLD);
}
