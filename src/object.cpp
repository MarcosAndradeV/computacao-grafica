#include "object.h"
#include <cmath>

ArtObject::ArtObject(int id, const std::string& name, const std::string& materialName,
                     ArtObjectType type, const Material& mat, const Transform3D& initialTrans)
    : id(id), name(name), materialName(materialName), type(type), material(mat),
      currentTransform(initialTrans), initialTransform(initialTrans) {}

void ArtObject::translate(float dx, float dy, float dz) {
    currentTransform.posX += dx;
    currentTransform.posY += dy;
    currentTransform.posZ += dz;
}

void ArtObject::rotate(float rx, float ry, float rz) {
    currentTransform.rotX += rx;
    currentTransform.rotY += ry;
    currentTransform.rotZ += rz;

    // Normalizar ângulos para [0, 360)
    currentTransform.rotX = std::fmod(currentTransform.rotX, 360.0f);
    currentTransform.rotY = std::fmod(currentTransform.rotY, 360.0f);
    currentTransform.rotZ = std::fmod(currentTransform.rotZ, 360.0f);
}

void ArtObject::changeScale(float deltaScale) {
    currentTransform.scale += deltaScale;
    if (currentTransform.scale < 0.1f) currentTransform.scale = 0.1f;
    if (currentTransform.scale > 4.0f) currentTransform.scale = 4.0f;
}

void ArtObject::reset() {
    currentTransform = initialTransform;
}

void ArtObject::render(bool isSelected, bool enableAmbient, bool enableDiffuse, bool enableSpecular) {
    glPushMatrix();

    // 1. Aplicação das Transformações Geométricas 3D na ordem: Translação -> Rotação -> Escala
    glTranslatef(currentTransform.posX, currentTransform.posY, currentTransform.posZ);
    glRotatef(currentTransform.rotX, 1.0f, 0.0f, 0.0f);
    glRotatef(currentTransform.rotY, 0.0f, 1.0f, 0.0f);
    glRotatef(currentTransform.rotZ, 0.0f, 0.0f, 1.0f);
    glScalef(currentTransform.scale, currentTransform.scale, currentTransform.scale);

    // 2. Configuração de Materiais na Pipeline Fixa
    Materials::apply(material, enableAmbient, enableDiffuse, enableSpecular);

    // 3. Renderização da Geometria 3D
    drawGeometry();

    // 4. Indicador visual para o objeto selecionado (anel ou caixa aramada discreta)
    if (isSelected) {
        drawSelectionHighlight();
    }

    glPopMatrix();
}

void ArtObject::drawGeometry() {
    switch (type) {
        case ArtObjectType::TEAPOT:
            // FreeGLUT teapot gera normais automáticas
            glFrontFace(GL_CW); // Teapot clássico do GLUT usa orientação horária
            glutSolidTeapot(0.9);
            glFrontFace(GL_CCW);
            break;

        case ArtObjectType::SPHERE:
            glutSolidSphere(0.85, 40, 40);
            break;

        case ArtObjectType::TORUS:
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            glutSolidTorus(0.32, 0.72, 32, 32);
            break;

        case ArtObjectType::CONE:
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, -0.7f);
            glutSolidCone(0.8, 1.5, 36, 36);
            break;
    }
}

void ArtObject::drawSelectionHighlight() {
    // Desabilitar iluminação temporariamente para renderizar o anel indicador brilhante
    glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT);
    glDisable(GL_LIGHTING);

    glColor3f(1.0f, 0.84f, 0.0f); // Dourado brilhante de seleção
    glLineWidth(2.5f);

    // Desenhar um anel ao redor da base do objeto
    glBegin(GL_LINE_LOOP);
    const int segments = 48;
    const float radius = 1.3f;
    for (int i = 0; i < segments; ++i) {
        float theta = 2.0f * 3.1415926f * float(i) / float(segments);
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);
        glVertex3f(x, -0.65f, z);
    }
    glEnd();

    glPopAttrib();
}
