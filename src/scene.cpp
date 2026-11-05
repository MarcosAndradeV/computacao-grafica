#include "scene.h"
#include <raymath.h>

Scene::Scene() : selectedEntity(1) {}

void Scene::init() {
    objects.clear();

    // Modelo de cubo unitário para renderização com iluminação Phong
    cubeModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));

    // Pedestal 1: Superior Esquerdo (-4.0, 2.2, -4.0) -> Nó Escultural em Ouro
    Transform3D t1;
    t1.posX = -4.0f; t1.posY = 2.2f; t1.posZ = -4.0f;
    t1.scale = 1.0f;
    objects.emplace_back(1, "No Escultural", "Ouro", ArtObjectType::KNOT, Materials::Gold(), t1);

    // Pedestal 2: Superior Direito (4.0, 2.2, -4.0) -> Esfera Polida em Rubi
    Transform3D t2;
    t2.posX = 4.0f; t2.posY = 2.2f; t2.posZ = -4.0f;
    t2.scale = 1.0f;
    objects.emplace_back(2, "Esfera Polida", "Rubi", ArtObjectType::SPHERE, Materials::Ruby(), t2);

    // Pedestal 3: Inferior Esquerdo (-4.0, 2.2, 4.0) -> Toro em Esmeralda
    Transform3D t3;
    t3.posX = -4.0f; t3.posY = 2.2f; t3.posZ = 4.0f;
    t3.scale = 1.0f;
    objects.emplace_back(3, "Toro Geometrico", "Esmeralda", ArtObjectType::TORUS, Materials::Emerald(), t3);

    // Pedestal 4: Inferior Direito (4.0, 2.2, 4.0) -> Cone em Cromo/Prata
    Transform3D t4;
    t4.posX = 4.0f; t4.posY = 2.2f; t4.posZ = 4.0f;
    t4.scale = 1.0f;
    objects.emplace_back(4, "Cone Escultural", "Cromo", ArtObjectType::CONE, Materials::Chrome(), t4);

    for (auto& obj : objects) {
        obj.init();
    }
}

void Scene::unload() {
    UnloadModel(cubeModel);
    for (auto& obj : objects) {
        obj.unload();
    }
}

void Scene::reset() {
    selectedEntity = 1;
    for (auto& obj : objects) {
        obj.reset();
    }
}

void Scene::selectEntity(int entityIndex) {
    if (entityIndex >= 0 && entityIndex <= 4) {
        selectedEntity = entityIndex;
    }
}

void Scene::translateSelected(float dx, float dy, float dz, LightingManager& lighting) {
    if (selectedEntity == 0) {
        lighting.translatePositionalLight(dx, dy, dz);
    } else if (selectedEntity >= 1 && selectedEntity <= 4) {
        objects[selectedEntity - 1].translate(dx, dy, dz);
    }
}

void Scene::rotateSelected(float rx, float ry, float rz) {
    if (selectedEntity >= 1 && selectedEntity <= 4) {
        objects[selectedEntity - 1].rotate(rx, ry, rz);
    }
}

void Scene::scaleSelected(float deltaScale) {
    if (selectedEntity >= 1 && selectedEntity <= 4) {
        objects[selectedEntity - 1].changeScale(deltaScale);
    }
}

void Scene::drawIlluminatedBox(Vector3 pos, Vector3 size, const MaterialData& mat, LightingManager& lighting) {
    cubeModel.materials[0].shader = lighting.shader;
    lighting.applyMaterial(mat);

    Matrix matScale = MatrixScale(size.x, size.y, size.z);
    Matrix matTrans = MatrixTranslate(pos.x, pos.y, pos.z);
    cubeModel.transform = MatrixMultiply(matScale, matTrans);

    DrawModel(cubeModel, {0.0f, 0.0f, 0.0f}, 1.0f, mat.baseColor);
}

void Scene::renderPedestal(float posX, float posZ, LightingManager& lighting) {
    // 1. Corpo principal do pedestal
    drawIlluminatedBox({posX, 0.65f, posZ}, {1.8f, 1.3f, 1.8f}, Materials::Pedestal(), lighting);

    // 2. Tampo superior
    drawIlluminatedBox({posX, 1.33f, posZ}, {1.95f, 0.06f, 1.95f}, Materials::Pedestal(), lighting);

    // 3. Rodapé inferior
    drawIlluminatedBox({posX, 0.03f, posZ}, {1.95f, 0.06f, 1.95f}, Materials::Pedestal(), lighting);

    // 4. Plaquinha decorativa de identificação frontal
    drawIlluminatedBox({posX, 0.85f, posZ + 0.91f}, {0.8f, 0.25f, 0.02f}, Materials::WoodTrim(), lighting);
}

void Scene::renderRoom(LightingManager& lighting) {
    const float roomSize = 9.5f;
    const float wallHeight = 5.5f;
    const int tiles = 14;
    const float step = (roomSize * 2.0f) / float(tiles);

    // 1. Piso Ladrilhado da Galeria
    for (int i = 0; i < tiles; ++i) {
        float x = -roomSize + (float(i) + 0.5f) * step;
        for (int j = 0; j < tiles; ++j) {
            float z = -roomSize + (float(j) + 0.5f) * step;
            const auto& mat = ((i + j) % 2 == 0) ? Materials::FloorWhite() : Materials::FloorDark();
            drawIlluminatedBox({x, -0.05f, z}, {step, 0.1f, step}, mat, lighting);
        }
    }

    // 2. Parede do Fundo (Z = -roomSize)
    drawIlluminatedBox({0.0f, wallHeight * 0.5f, -roomSize}, {roomSize * 2.0f, wallHeight, 0.2f}, Materials::Wall(), lighting);

    // 3. Parede Lateral Esquerda (X = -roomSize)
    drawIlluminatedBox({-roomSize, wallHeight * 0.5f, 0.0f}, {0.2f, wallHeight, roomSize * 2.0f}, Materials::Wall(), lighting);

    // 4. Parede Lateral Direita (X = roomSize)
    drawIlluminatedBox({roomSize, wallHeight * 0.5f, 0.0f}, {0.2f, wallHeight, roomSize * 2.0f}, Materials::Wall(), lighting);

    // 5. Rodapés de Madeira nas paredes
    drawIlluminatedBox({0.0f, 0.15f, -roomSize + 0.1f}, {roomSize * 2.0f, 0.3f, 0.1f}, Materials::WoodTrim(), lighting);
    drawIlluminatedBox({-roomSize + 0.1f, 0.15f, 0.0f}, {0.1f, 0.3f, roomSize * 2.0f}, Materials::WoodTrim(), lighting);
    drawIlluminatedBox({roomSize - 0.1f, 0.15f, 0.0f}, {0.1f, 0.3f, roomSize * 2.0f}, Materials::WoodTrim(), lighting);
}

void Scene::render(LightingManager& lighting) {
    // 1. Renderizar arquitetura da galeria (sala e paredes)
    renderRoom(lighting);

    // 2. Renderizar os 4 pedestais de exposição
    renderPedestal(-4.0f, -4.0f, lighting);
    renderPedestal( 4.0f, -4.0f, lighting);
    renderPedestal(-4.0f,  4.0f, lighting);
    renderPedestal( 4.0f,  4.0f, lighting);

    // 3. Renderizar as 4 obras de arte
    for (size_t i = 0; i < objects.size(); ++i) {
        bool isSelected = (selectedEntity == static_cast<int>(i + 1));
        objects[i].render(isSelected, lighting);
    }

    // 4. Renderizar o marcador visual da lâmpada posicional
    lighting.renderLightMarker(selectedEntity == 0);
}
