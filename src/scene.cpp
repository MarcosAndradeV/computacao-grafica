#include "scene.h"

Scene::Scene() : selectedEntity(1) {
    init();
}

void Scene::init() {
    objects.clear();

    // Pedestal 1: Superior Esquerdo (-4.0, 2.2, -4.0) -> Utah Teapot em Ouro
    Transform3D t1;
    t1.posX = -4.0f; t1.posY = 2.2f; t1.posZ = -4.0f;
    t1.scale = 1.0f;
    objects.emplace_back(1, "Utah Teapot", "Ouro", ArtObjectType::TEAPOT, Materials::Gold(), t1);

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
        // Luz Posicional selecionada
        lighting.translatePositionalLight(dx, dy, dz);
    } else if (selectedEntity >= 1 && selectedEntity <= 4) {
        // Um dos 4 objetos de arte
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

void Scene::renderPedestal(float posX, float posZ, bool enableAmbient, bool enableDiffuse, bool enableSpecular) {
    glPushMatrix();
    glTranslatef(posX, 0.65f, posZ);

    Materials::apply(Materials::Pedestal(), enableAmbient, enableDiffuse, enableSpecular);

    // Corpo principal do pedestal (cubo escalado)
    glPushMatrix();
    glScalef(1.8f, 1.3f, 1.8f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Moldura superior/tampo do pedestal
    glPushMatrix();
    glTranslatef(0.0f, 0.66f, 0.0f);
    glScalef(1.95f, 0.06f, 1.95f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Rodapé do pedestal
    glPushMatrix();
    glTranslatef(0.0f, -0.63f, 0.0f);
    glScalef(1.95f, 0.06f, 1.95f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Plaquinha decorativa de identificação frontal (madeira dourada/latão)
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.91f);
    glScalef(0.8f, 0.25f, 0.02f);
    Materials::apply(Materials::WoodTrim(), enableAmbient, enableDiffuse, enableSpecular);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

void Scene::renderRoom(bool enableAmbient, bool enableDiffuse, bool enableSpecular) {
    const float roomSize = 9.5f;
    const float wallHeight = 5.5f;
    const int tiles = 18;
    const float step = (roomSize * 2.0f) / float(tiles);

    // 1. Piso Ladrilhado da Galeria com normais para cima
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < tiles; ++i) {
        float x1 = -roomSize + float(i) * step;
        float x2 = x1 + step;
        for (int j = 0; j < tiles; ++j) {
            float z1 = -roomSize + float(j) * step;
            float z2 = z1 + step;

            if ((i + j) % 2 == 0) {
                Materials::apply(Materials::FloorWhite(), enableAmbient, enableDiffuse, enableSpecular);
            } else {
                Materials::apply(Materials::FloorDark(), enableAmbient, enableDiffuse, enableSpecular);
            }

            glBegin(GL_QUADS);
            glVertex3f(x1, 0.0f, z1);
            glVertex3f(x1, 0.0f, z2);
            glVertex3f(x2, 0.0f, z2);
            glVertex3f(x2, 0.0f, z1);
            glEnd();
        }
    }

    Materials::apply(Materials::Wall(), enableAmbient, enableDiffuse, enableSpecular);

    // 2. Parede do Fundo (Z = -roomSize)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(-roomSize, 0.0f, -roomSize);
    glVertex3f( roomSize, 0.0f, -roomSize);
    glVertex3f( roomSize, wallHeight, -roomSize);
    glVertex3f(-roomSize, wallHeight, -roomSize);
    glEnd();

    // 3. Parede Lateral Esquerda (X = -roomSize)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-roomSize, 0.0f,  roomSize);
    glVertex3f(-roomSize, 0.0f, -roomSize);
    glVertex3f(-roomSize, wallHeight, -roomSize);
    glVertex3f(-roomSize, wallHeight,  roomSize);
    glEnd();

    // 4. Parede Lateral Direita (X = roomSize)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(roomSize, 0.0f, -roomSize);
    glVertex3f(roomSize, 0.0f,  roomSize);
    glVertex3f(roomSize, wallHeight,  roomSize);
    glVertex3f(roomSize, wallHeight, -roomSize);
    glEnd();

    // 5. Rodapés de Madeira nas paredes
    Materials::apply(Materials::WoodTrim(), enableAmbient, enableDiffuse, enableSpecular);

    // Rodapé fundo
    glPushMatrix();
    glTranslatef(0.0f, 0.15f, -roomSize + 0.05f);
    glScalef(roomSize * 2.0f, 0.3f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Rodapé esquerda
    glPushMatrix();
    glTranslatef(-roomSize + 0.05f, 0.15f, 0.0f);
    glScalef(0.1f, 0.3f, roomSize * 2.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Rodapé direita
    glPushMatrix();
    glTranslatef(roomSize - 0.05f, 0.15f, 0.0f);
    glScalef(0.1f, 0.3f, roomSize * 2.0f);
    glutSolidCube(1.0);
    glPopMatrix();
}

void Scene::render(LightingManager& lighting) {
    bool amb = lighting.ambientEnabled;
    bool dif = lighting.diffuseEnabled;
    bool esp = lighting.specularEnabled;

    // 1. Renderizar arquitetura da galeria (sala e paredes)
    renderRoom(amb, dif, esp);

    // 2. Renderizar os 4 pedestais de exposição
    renderPedestal(-4.0f, -4.0f, amb, dif, esp);
    renderPedestal( 4.0f, -4.0f, amb, dif, esp);
    renderPedestal(-4.0f,  4.0f, amb, dif, esp);
    renderPedestal( 4.0f,  4.0f, amb, dif, esp);

    // 3. Renderizar os 4 objetos de arte tridimensionais
    for (size_t i = 0; i < objects.size(); ++i) {
        bool isSelected = (selectedEntity == static_cast<int>(i + 1));
        objects[i].render(isSelected, amb, dif, esp);
    }

    // 4. Renderizar a lâmpada/marcador da luz posicional
    lighting.renderLightMarker(selectedEntity == 0);
}
