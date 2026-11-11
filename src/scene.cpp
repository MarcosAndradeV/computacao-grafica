#include "scene.h"

Scene::Scene() : selectedEntity(1) { init(); }

void Scene::init() {
    objects.clear();

    Transform3D t1;
    t1.posX = -4.0f;
    t1.posY = 2.2f;
    t1.posZ = -4.0f;
    t1.scale = 1.0f;
    objects.emplace_back(1, "Utah Teapot", "Ouro", ArtObjectType::TEAPOT, Materials::Gold(), t1);

    Transform3D t2;
    t2.posX = 4.0f;
    t2.posY = 2.2f;
    t2.posZ = -4.0f;
    t2.scale = 1.0f;
    objects.emplace_back(2, "Esfera Polida", "Rubi", ArtObjectType::SPHERE, Materials::Ruby(), t2);

    Transform3D t3;
    t3.posX = -4.0f;
    t3.posY = 2.2f;
    t3.posZ = 4.0f;
    t3.scale = 1.0f;
    objects.emplace_back(3, "Toro Geometrico", "Esmeralda", ArtObjectType::TORUS, Materials::Emerald(), t3);

    Transform3D t4;
    t4.posX = 4.0f;
    t4.posY = 2.2f;
    t4.posZ = 4.0f;
    t4.scale = 1.0f;
    objects.emplace_back(4, "Cone Escultural", "Cromo", ArtObjectType::CONE, Materials::Chrome(), t4);
}

void Scene::reset() {
    selectedEntity = 1;
    for (auto &obj : objects) {
        obj.reset();
    }
}

void Scene::selectEntity(int entityIndex) {
    if (entityIndex >= 0 && entityIndex <= 4) {
        selectedEntity = entityIndex;
    }
}

void Scene::translateSelected(float dx, float dy, float dz, LightingManager &lighting) {
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

void Scene::renderPedestal(float posX, float posZ, bool enableAmbient, bool enableDiffuse, bool enableSpecular) {
    glPushMatrix();
    glTranslatef(posX, 0.65f, posZ);

    Materials::apply(Materials::Pedestal(), enableAmbient, enableDiffuse, enableSpecular);

    glPushMatrix();
    glScalef(1.8f, 1.3f, 1.8f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.66f, 0.0f);
    glScalef(1.95f, 0.06f, 1.95f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, -0.63f, 0.0f);
    glScalef(1.95f, 0.06f, 1.95f);
    glutSolidCube(1.0);
    glPopMatrix();

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
#ifdef WALLS
    Materials::apply(Materials::Wall(), enableAmbient, enableDiffuse, enableSpecular);

    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(-roomSize, 0.0f, -roomSize);
    glVertex3f(roomSize, 0.0f, -roomSize);
    glVertex3f(roomSize, wallHeight, -roomSize);
    glVertex3f(-roomSize, wallHeight, -roomSize);
    glEnd();

    glNormal3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-roomSize, 0.0f, roomSize);
    glVertex3f(-roomSize, 0.0f, -roomSize);
    glVertex3f(-roomSize, wallHeight, -roomSize);
    glVertex3f(-roomSize, wallHeight, roomSize);
    glEnd();

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(roomSize, 0.0f, -roomSize);
    glVertex3f(roomSize, 0.0f, roomSize);
    glVertex3f(roomSize, wallHeight, roomSize);
    glVertex3f(roomSize, wallHeight, -roomSize);
    glEnd();
#endif
    Materials::apply(Materials::WoodTrim(), enableAmbient, enableDiffuse, enableSpecular);

    glPushMatrix();
    glTranslatef(0.0f, 0.15f, -roomSize + 0.05f);
    glScalef(roomSize * 2.0f, 0.3f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-roomSize + 0.05f, 0.15f, 0.0f);
    glScalef(0.1f, 0.3f, roomSize * 2.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(roomSize - 0.05f, 0.15f, 0.0f);
    glScalef(0.1f, 0.3f, roomSize * 2.0f);
    glutSolidCube(1.0);
    glPopMatrix();
}

void Scene::render(LightingManager &lighting) {
    bool amb = lighting.ambientEnabled;
    bool dif = lighting.diffuseEnabled;
    bool esp = lighting.specularEnabled;

    renderRoom(amb, dif, esp);

    renderPedestal(-4.0f, -4.0f, amb, dif, esp);
    renderPedestal(4.0f, -4.0f, amb, dif, esp);
    renderPedestal(-4.0f, 4.0f, amb, dif, esp);
    renderPedestal(4.0f, 4.0f, amb, dif, esp);

    for (size_t i = 0; i < objects.size(); ++i) {
        bool isSelected = (selectedEntity == static_cast<int>(i + 1));
        objects[i].render(isSelected, amb, dif, esp);
    }

    lighting.renderLightMarker(selectedEntity == 0);
}
