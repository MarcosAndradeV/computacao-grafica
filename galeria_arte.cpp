/**
 * ============================================================================
 * FAINOR - ENGENHARIA DE COMPUTAÇÃO
 * COMPUTAÇÃO GRÁFICA - ATIVIDADE AVALIATIVA BF3
 * ============================================================================
 * Projeto: Galeria de Arte 3D Interativa
 * Docente: Prof. José Humberto Góes Alves
 * Período: 2026.2
 *
 * Integrantes: Marcos Andrade, Tacio Moreira, David Gomes, Kaique Maia e Felipe Gama
 *
 * Tecnologias: C++17, OpenGL, GLU, FreeGLUT
 *
 * COMO COMPILAR:
 *   g++ -Wall -Wextra -O2 galeria_arte.cpp -lglut -lGLU -lGL -lm -o galeria_arte
 *
 * COMO EXECUTAR:
 *   ./galeria_arte
 *
 * RECURSOS IMPLEMENTADOS:
 *   1. Ambiente 3D: Sala expositiva com piso ladrilhado xadrez e 4 pedestais.
 *   2. Transformações 3D: Translação (XYZ), Rotação (XYZ) e Escala por objeto.
 *   3. Viewing 3D: Câmera orbital (Setas), Zoom (PgUp/PgDn), Pan (Shift+Setas)
 *      e alternância dinâmica entre Projeção Perspectiva e Ortográfica ('P').
 *   4. Iluminação Phong: Luz Direcional distante (F1), Luz Posicional móvel (F2),
 *      componentes Ambiente (F3), Difusa (F4) e Especular (F5), materiais nobres
 *      com diferentes coeficientes de reflexão e expoente de brilho (shininess).
 *   5. Seleção e Controle: Teclas 1-4 selecionam as obras; Tecla '0' seleciona
 *      a lâmpada posicional para movimentação no espaço via W/S/A/D/Q/E.
 *   6. HUD 2D: Overlay gráfico desenhado na viewport via gluOrtho2D exibindo
 *      status em tempo real e guia de teclas (alternável com 'H').
 * ============================================================================
 */

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

constexpr float DEG2RAD = 3.14159265358979323846f / 180.0f;

struct Material {
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat emission[4];
    GLfloat shininess;
};

namespace Materials {

// 1. Ouro
inline Material Gold() {
    return {
        {0.24725f, 0.1995f, 0.0745f, 1.0f},
        {0.75164f, 0.60648f, 0.22648f, 1.0f},
        {0.628281f, 0.555802f, 0.366065f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        51.2f
    };
}

// 2. Rubi
inline Material Ruby() {
    return {
        {0.1745f, 0.01175f, 0.01175f, 1.0f},
        {0.61424f, 0.04136f, 0.04136f, 1.0f},
        {0.727811f, 0.626959f, 0.626959f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        76.8f
    };
}

// 3. Esmeralda
inline Material Emerald() {
    return {
        {0.0215f, 0.1745f, 0.0215f, 1.0f},
        {0.07568f, 0.61424f, 0.07568f, 1.0f},
        {0.633f, 0.727811f, 0.633f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        76.8f
    };
}

// 4. Cromo
inline Material Chrome() {
    return {
        {0.25f, 0.25f, 0.25f, 1.0f},
        {0.4f, 0.4f, 0.4f, 1.0f},
        {0.774597f, 0.774597f, 0.774597f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        89.6f
    };
}

// Pedestal de exibição
inline Material Pedestal() {
    return {
        {0.12f, 0.12f, 0.14f, 1.0f},
        {0.35f, 0.35f, 0.38f, 1.0f},
        {0.2f, 0.2f, 0.22f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        20.0f
    };
}

// Piso ladrilhado claro
inline Material FloorWhite() {
    return {
        {0.2f, 0.2f, 0.2f, 1.0f},
        {0.8f, 0.8f, 0.82f, 1.0f},
        {0.15f, 0.15f, 0.15f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        10.0f
    };
}

// Piso ladrilhado escuro
inline Material FloorDark() {
    return {
        {0.05f, 0.05f, 0.06f, 1.0f},
        {0.18f, 0.18f, 0.20f, 1.0f},
        {0.1f, 0.1f, 0.1f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        10.0f
    };
}

// Rodapés e molduras de madeira nobre
inline Material WoodTrim() {
    return {
        {0.12f, 0.06f, 0.02f, 1.0f},
        {0.35f, 0.18f, 0.08f, 1.0f},
        {0.15f, 0.15f, 0.15f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        15.0f
    };
}

// Aplicação de materiais na pipeline fixa respeitando os toggles de iluminação
inline void apply(const Material& mat, bool enableAmbient = true, bool enableDiffuse = true, bool enableSpecular = true) {
    GLfloat black[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, enableAmbient ? mat.ambient : black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, enableDiffuse ? mat.diffuse : black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, enableSpecular ? mat.specular : black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat.emission);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat.shininess);
}

} // namespace Materials

// ============================================================================
// 2. MÓDULO DE VIEWING 3D (CÂMERA E MATRIZES DE PROJEÇÃO)
// ============================================================================

enum class ProjectionMode {
    PERSPECTIVE,
    ORTHOGRAPHIC
};

class Camera {
public:
    ProjectionMode projectionMode;

    // Ponto de observação no espaço 3D (Target)
    float targetX, targetY, targetZ;

    // Coordenadas esféricas relativas ao ponto de foco
    float azimuthDeg;   // Rotação horizontal (eixo Y)
    float elevationDeg; // Rotação vertical (elevação)
    float distance;     // Distância focal (Zoom)

    // Valores padrão de inicialização
    const float defaultTargetX = 0.0f;
    const float defaultTargetY = 1.2f;
    const float defaultTargetZ = 0.0f;
    const float defaultAzimuth = 45.0f;
    const float defaultElevation = 25.0f;
    const float defaultDistance = 14.0f;

    Camera() {
        projectionMode = ProjectionMode::PERSPECTIVE;
        reset();
    }

    void reset() {
        targetX = defaultTargetX;
        targetY = defaultTargetY;
        targetZ = defaultTargetZ;
        azimuthDeg = defaultAzimuth;
        elevationDeg = defaultElevation;
        distance = defaultDistance;
        projectionMode = ProjectionMode::PERSPECTIVE;
    }

    void rotate(float deltaAzimuth, float deltaElevation) {
        azimuthDeg += deltaAzimuth;
        elevationDeg += deltaElevation;

        azimuthDeg = std::fmod(azimuthDeg, 360.0f);
        if (azimuthDeg < 0.0f) azimuthDeg += 360.0f;

        // Limita a elevação para evitar efeito gimbal lock
        elevationDeg = std::clamp(elevationDeg, -85.0f, 85.0f);
    }

    void zoom(float deltaDistance) {
        distance += deltaDistance;
        distance = std::clamp(distance, 3.0f, 35.0f);
    }

    void pan(float deltaX, float deltaY) {
        float radAz = azimuthDeg * DEG2RAD;
        float cosAz = std::cos(radAz);
        float sinAz = std::sin(radAz);

        float rightX = cosAz;
        float rightZ = -sinAz;

        targetX += rightX * deltaX;
        targetZ += rightZ * deltaX;
        targetY += deltaY;
    }

    void toggleProjection() {
        if (projectionMode == ProjectionMode::PERSPECTIVE) {
            projectionMode = ProjectionMode::ORTHOGRAPHIC;
        } else {
            projectionMode = ProjectionMode::PERSPECTIVE;
        }
    }

    void applyProjection(int windowWidth, int windowHeight) const {
        if (windowHeight <= 0) windowHeight = 1;
        float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        if (projectionMode == ProjectionMode::PERSPECTIVE) {
            gluPerspective(45.0, aspect, 0.5, 100.0);
        } else {
            float orthoHeight = distance * 0.45f;
            float orthoWidth = orthoHeight * aspect;
            glOrtho(-orthoWidth, orthoWidth, -orthoHeight, orthoHeight, -50.0, 100.0);
        }

        glMatrixMode(GL_MODELVIEW);
    }

    void applyView() const {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        float radAz = azimuthDeg * DEG2RAD;
        float radEl = elevationDeg * DEG2RAD;

        float eyeX = targetX + distance * std::cos(radEl) * std::sin(radAz);
        float eyeY = targetY + distance * std::sin(radEl);
        float eyeZ = targetZ + distance * std::cos(radEl) * std::cos(radAz);

        gluLookAt(eyeX, eyeY, eyeZ, targetX, targetY, targetZ, 0.0f, 1.0f, 0.0f);
    }

    const char* getProjectionName() const {
        return (projectionMode == ProjectionMode::PERSPECTIVE)
            ? "PERSPECTIVA (gluPerspective)"
            : "ORTOGRAFICA (glOrtho)";
    }
};

// ============================================================================
// 3. MÓDULO DE ILUMINAÇÃO (FONTES DE LUZ E COMPONENTES PHONG)
// ============================================================================

class LightingManager {
public:
    // Toggles de luzes e componentes (F1 - F5)
    bool directionalEnabled;  // F1
    bool positionalEnabled;   // F2
    bool ambientEnabled;      // F3
    bool diffuseEnabled;      // F4
    bool specularEnabled;     // F5

    // Posições das fontes luminosas
    GLfloat directionalDir[4]; // Luz distante (w = 0.0)
    GLfloat positionalPos[4];  // Luz próxima (w = 1.0)

    const GLfloat defaultPositionalPos[4]  = {0.0f, 3.8f, 0.0f, 1.0f};
    const GLfloat defaultDirectionalDir[4] = {1.0f, 2.0f, 1.2f, 0.0f};

    LightingManager() { reset(); }

    void reset() {
        directionalEnabled = true;
        positionalEnabled = true;
        ambientEnabled = true;
        diffuseEnabled = true;
        specularEnabled = true;

        for (int i = 0; i < 4; ++i) {
            positionalPos[i] = defaultPositionalPos[i];
            directionalDir[i] = defaultDirectionalDir[i];
        }
    }

    void init() {
        glEnable(GL_LIGHTING);
        glEnable(GL_NORMALIZE);
        glShadeModel(GL_SMOOTH);

        // Atenuação física realista para a fonte posicional próxima
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.05f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.01f);
    }

    void toggleDirectional() { directionalEnabled = !directionalEnabled; }
    void togglePositional()  { positionalEnabled = !positionalEnabled; }
    void toggleAmbient()     { ambientEnabled = !ambientEnabled; }
    void toggleDiffuse()     { diffuseEnabled = !diffuseEnabled; }
    void toggleSpecular()    { specularEnabled = !specularEnabled; }

    void translatePositionalLight(float dx, float dy, float dz) {
        positionalPos[0] += dx;
        positionalPos[1] += dy;
        positionalPos[2] += dz;

        // Limita a lâmpada dentro da área visível da galeria
        if (positionalPos[1] < 0.3f) positionalPos[1] = 0.3f;
        if (positionalPos[1] > 6.0f) positionalPos[1] = 6.0f;
    }

    void applyLighting() {
        // Componente Ambiente Global
        GLfloat globalAmbient[4] = {0.2f, 0.2f, 0.2f, 1.0f};
        GLfloat noAmbient[4]     = {0.0f, 0.0f, 0.0f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientEnabled ? globalAmbient : noAmbient);

        // Fonte Direcional Distante (GL_LIGHT0, w = 0.0)
        if (directionalEnabled) {
            glEnable(GL_LIGHT0);

            GLfloat dirAmbient[4]  = {0.1f, 0.1f, 0.1f, 1.0f};
            GLfloat dirDiffuse[4]  = {0.7f, 0.7f, 0.65f, 1.0f};
            GLfloat dirSpecular[4] = {0.6f, 0.6f, 0.6f, 1.0f};
            GLfloat black[4]       = {0.0f, 0.0f, 0.0f, 1.0f};

            glLightfv(GL_LIGHT0, GL_POSITION, directionalDir);
            glLightfv(GL_LIGHT0, GL_AMBIENT, ambientEnabled ? dirAmbient : black);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseEnabled ? dirDiffuse : black);
            glLightfv(GL_LIGHT0, GL_SPECULAR, specularEnabled ? dirSpecular : black);
        } else {
            glDisable(GL_LIGHT0);
        }

        // Fonte Posicional Próxima (GL_LIGHT1, w = 1.0)
        if (positionalEnabled) {
            glEnable(GL_LIGHT1);

            GLfloat posAmbient[4]  = {0.12f, 0.12f, 0.08f, 1.0f};
            GLfloat posDiffuse[4]  = {1.0f, 0.95f, 0.8f, 1.0f};
            GLfloat posSpecular[4] = {1.0f, 1.0f, 0.9f, 1.0f};
            GLfloat black[4]       = {0.0f, 0.0f, 0.0f, 1.0f};

            glLightfv(GL_LIGHT1, GL_POSITION, positionalPos);
            glLightfv(GL_LIGHT1, GL_AMBIENT, ambientEnabled ? posAmbient : black);
            glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseEnabled ? posDiffuse : black);
            glLightfv(GL_LIGHT1, GL_SPECULAR, specularEnabled ? posSpecular : black);
        } else {
            glDisable(GL_LIGHT1);
        }
    }

    void renderLightMarker(bool isSelected) {
        if (!positionalEnabled) return;

        glPushMatrix();
        glTranslatef(positionalPos[0], positionalPos[1], positionalPos[2]);

        // Marcador visual esférico emissivo
        GLfloat emissionColor[4] = {1.0f, 0.95f, 0.4f, 1.0f};
        GLfloat noEmission[4]    = {0.0f, 0.0f, 0.0f, 1.0f};

        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissionColor);
        glutSolidSphere(0.22, 24, 24);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);

        // Indicador quando a lâmpada estiver selecionada para mover (tecla '0')
        if (isSelected) {
            glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT);
            glDisable(GL_LIGHTING);
            glColor3f(1.0f, 0.85f, 0.0f);
            glLineWidth(2.0f);

            glBegin(GL_LINE_LOOP);
            for (int i = 0; i < 36; ++i) {
                float rad = 2.0f * 3.1415926f * float(i) / 36.0f;
                glVertex3f(0.38f * std::cos(rad), 0.0f, 0.38f * std::sin(rad));
            }
            glEnd();

            glBegin(GL_LINE_LOOP);
            for (int i = 0; i < 36; ++i) {
                float rad = 2.0f * 3.1415926f * float(i) / 36.0f;
                glVertex3f(0.38f * std::cos(rad), 0.38f * std::sin(rad), 0.0f);
            }
            glEnd();

            glPopAttrib();
        }

        glPopMatrix();
    }
};

// ============================================================================
// 4. MÓDULO DE OBJETOS E TRANSFORMAÇÕES GEOMÉTRICAS 3D
// ============================================================================

enum class ArtObjectType {
    TEAPOT,
    SPHERE,
    TORUS,
    CONE
};

struct Transform3D {
    float posX = 0.0f;
    float posY = 0.0f;
    float posZ = 0.0f;
    float rotX = 0.0f;
    float rotY = 0.0f;
    float rotZ = 0.0f;
    float scale = 1.0f;
};

class ArtObject {
public:
    int id;
    std::string name;
    std::string materialName;
    ArtObjectType type;
    Material material;

    Transform3D currentTransform;
    Transform3D initialTransform;

    ArtObject(int id, const std::string& name, const std::string& materialName,
              ArtObjectType type, const Material& mat, const Transform3D& initialTrans)
        : id(id), name(name), materialName(materialName), type(type), material(mat),
          currentTransform(initialTrans), initialTransform(initialTrans) {}

    void translate(float dx, float dy, float dz) {
        currentTransform.posX += dx;
        currentTransform.posY += dy;
        currentTransform.posZ += dz;
    }

    void rotate(float rx, float ry, float rz) {
        currentTransform.rotX += rx;
        currentTransform.rotY += ry;
        currentTransform.rotZ += rz;

        currentTransform.rotX = std::fmod(currentTransform.rotX, 360.0f);
        currentTransform.rotY = std::fmod(currentTransform.rotY, 360.0f);
        currentTransform.rotZ = std::fmod(currentTransform.rotZ, 360.0f);
    }

    void changeScale(float deltaScale) {
        currentTransform.scale += deltaScale;
        if (currentTransform.scale < 0.1f) currentTransform.scale = 0.1f;
        if (currentTransform.scale > 4.0f) currentTransform.scale = 4.0f;
    }

    void reset() { currentTransform = initialTransform; }

    void render(bool isSelected, bool enableAmbient, bool enableDiffuse, bool enableSpecular) {
        glPushMatrix();

        // Composição de Transformações Geométricas 3D: Translação -> Rotação -> Escala
        glTranslatef(currentTransform.posX, currentTransform.posY, currentTransform.posZ);
        glRotatef(currentTransform.rotX, 1.0f, 0.0f, 0.0f);
        glRotatef(currentTransform.rotY, 0.0f, 1.0f, 0.0f);
        glRotatef(currentTransform.rotZ, 0.0f, 0.0f, 1.0f);
        glScalef(currentTransform.scale, currentTransform.scale, currentTransform.scale);

        // Aplica o material óptico
        Materials::apply(material, enableAmbient, enableDiffuse, enableSpecular);

        // Desenha a geometria com normais apropriadas
        drawGeometry();

        // Anel indicador de seleção ao redor do objeto ativo
        if (isSelected) {
            drawSelectionHighlight();
        }

        glPopMatrix();
    }

private:
    void drawGeometry() {
        switch (type) {
            case ArtObjectType::TEAPOT:
                glFrontFace(GL_CW);
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

    void drawSelectionHighlight() {
        glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT);
        glDisable(GL_LIGHTING);

        glColor3f(1.0f, 0.84f, 0.0f);
        glLineWidth(2.5f);

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
};

// ============================================================================
// 5. MÓDULO DA CENA 3D (SALA EXPOSITIVA E PEDESTAIS)
// ============================================================================

class Scene {
public:
    std::vector<ArtObject> objects;
    int selectedEntity; // 0 = Luz Posicional, 1..4 = Obras de arte 1..4

    Scene() : selectedEntity(1) { init(); }

    void init() {
        objects.clear();

        // Objeto 1: Utah Teapot em Ouro
        Transform3D t1;
        t1.posX = -4.0f; t1.posY = 2.2f; t1.posZ = -4.0f; t1.scale = 1.0f;
        objects.emplace_back(1, "Utah Teapot", "Ouro", ArtObjectType::TEAPOT, Materials::Gold(), t1);

        // Objeto 2: Esfera Polida em Rubi
        Transform3D t2;
        t2.posX = 4.0f; t2.posY = 2.2f; t2.posZ = -4.0f; t2.scale = 1.0f;
        objects.emplace_back(2, "Esfera Polida", "Rubi", ArtObjectType::SPHERE, Materials::Ruby(), t2);

        // Objeto 3: Toro Geométrico em Esmeralda
        Transform3D t3;
        t3.posX = -4.0f; t3.posY = 2.2f; t3.posZ = 4.0f; t3.scale = 1.0f;
        objects.emplace_back(3, "Toro Geometrico", "Esmeralda", ArtObjectType::TORUS, Materials::Emerald(), t3);

        // Objeto 4: Cone Escultural em Cromo
        Transform3D t4;
        t4.posX = 4.0f; t4.posY = 2.2f; t4.posZ = 4.0f; t4.scale = 1.0f;
        objects.emplace_back(4, "Cone Escultural", "Cromo", ArtObjectType::CONE, Materials::Chrome(), t4);
    }

    void reset() {
        selectedEntity = 1;
        for (auto& obj : objects) {
            obj.reset();
        }
    }

    void selectEntity(int entityIndex) {
        if (entityIndex >= 0 && entityIndex <= 4) {
            selectedEntity = entityIndex;
        }
    }

    void translateSelected(float dx, float dy, float dz, LightingManager& lighting) {
        if (selectedEntity == 0) {
            lighting.translatePositionalLight(dx, dy, dz);
        } else if (selectedEntity >= 1 && selectedEntity <= 4) {
            objects[selectedEntity - 1].translate(dx, dy, dz);
        }
    }

    void rotateSelected(float rx, float ry, float rz) {
        if (selectedEntity >= 1 && selectedEntity <= 4) {
            objects[selectedEntity - 1].rotate(rx, ry, rz);
        }
    }

    void scaleSelected(float deltaScale) {
        if (selectedEntity >= 1 && selectedEntity <= 4) {
            objects[selectedEntity - 1].changeScale(deltaScale);
        }
    }

    void renderPedestal(float posX, float posZ, bool enableAmbient, bool enableDiffuse, bool enableSpecular) {
        glPushMatrix();
        glTranslatef(posX, 0.65f, posZ);

        Materials::apply(Materials::Pedestal(), enableAmbient, enableDiffuse, enableSpecular);

        // Bloco principal
        glPushMatrix();
        glScalef(1.8f, 1.3f, 1.8f);
        glutSolidCube(1.0);
        glPopMatrix();

        // Tampo superior
        glPushMatrix();
        glTranslatef(0.0f, 0.66f, 0.0f);
        glScalef(1.95f, 0.06f, 1.95f);
        glutSolidCube(1.0);
        glPopMatrix();

        // Base inferior
        glPushMatrix();
        glTranslatef(0.0f, -0.63f, 0.0f);
        glScalef(1.95f, 0.06f, 1.95f);
        glutSolidCube(1.0);
        glPopMatrix();

        // Placa frontal decorativa
        glPushMatrix();
        glTranslatef(0.0f, 0.2f, 0.91f);
        glScalef(0.8f, 0.25f, 0.02f);
        Materials::apply(Materials::WoodTrim(), enableAmbient, enableDiffuse, enableSpecular);
        glutSolidCube(1.0);
        glPopMatrix();

        glPopMatrix();
    }

    void renderRoom(bool enableAmbient, bool enableDiffuse, bool enableSpecular) {
        const float roomSize = 9.5f;
        const int tiles = 18;
        const float step = (roomSize * 2.0f) / float(tiles);

        // Piso xadrez com cálculo correto de normais para cima
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

        Materials::apply(Materials::WoodTrim(), enableAmbient, enableDiffuse, enableSpecular);

        // Rodapés de contorno
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

    void render(LightingManager& lighting) {
        bool amb = lighting.ambientEnabled;
        bool dif = lighting.diffuseEnabled;
        bool esp = lighting.specularEnabled;

        renderRoom(amb, dif, esp);

        // Renderiza os pedestais
        renderPedestal(-4.0f, -4.0f, amb, dif, esp);
        renderPedestal( 4.0f, -4.0f, amb, dif, esp);
        renderPedestal(-4.0f,  4.0f, amb, dif, esp);
        renderPedestal( 4.0f,  4.0f, amb, dif, esp);

        // Renderiza as obras de arte
        for (size_t i = 0; i < objects.size(); ++i) {
            bool isSelected = (selectedEntity == static_cast<int>(i + 1));
            objects[i].render(isSelected, amb, dif, esp);
        }

        // Renderiza a lâmpada indicadora
        lighting.renderLightMarker(selectedEntity == 0);
    }
};

// ============================================================================
// 6. MÓDULO DE INTERFACE VISUAL (HUD 2D SOBRE A JANELA OPENGL)
// ============================================================================

class HUD {
public:
    bool showHelp;

    HUD() : showHelp(false) {}

    void toggleHelp() { showHelp = !showHelp; }

    void drawText(float x, float y, const std::string& text, void* font = nullptr) {
        if (!font) font = GLUT_BITMAP_HELVETICA_12;
        glRasterPos2f(x, y);
        for (char c : text) {
            glutBitmapCharacter(font, c);
        }
    }

    void drawPanel(float x, float y, float w, float h, float r, float g, float b, float a) {
        glColor4f(r, g, b, a);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }

    void render(int windowWidth, int windowHeight,
                const Camera& camera,
                const LightingManager& lighting,
                const Scene& scene) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, windowWidth, windowHeight, 0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float panelX = 15.0f;
        float panelY = 15.0f;
        float panelW = 430.0f;
        float panelH = showHelp ? 365.0f : 165.0f;

        drawPanel(panelX, panelY, panelW, panelH, 0.08f, 0.09f, 0.12f, 0.85f);

        glColor4f(0.3f, 0.45f, 0.65f, 0.9f);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(panelX, panelY);
        glVertex2f(panelX + panelW, panelY);
        glVertex2f(panelX + panelW, panelY + panelH);
        glVertex2f(panelX, panelY + panelH);
        glEnd();

        // Título
        glColor3f(1.0f, 0.85f, 0.3f);
        drawText(panelX + 15.0f, panelY + 24.0f, "GALERIA DE ARTE 3D - COMPUTACAO GRAFICA", GLUT_BITMAP_HELVETICA_12);

        // Entidade Selecionada
        float currentY = panelY + 48.0f;
        std::ostringstream ssEnt;
        if (scene.selectedEntity == 0) {
            ssEnt << "Entidade Ativa: [0] LUZ POSICIONAL (Movel)";
            glColor3f(1.0f, 0.95f, 0.2f);
        } else {
            const auto& obj = scene.objects[scene.selectedEntity - 1];
            ssEnt << "Entidade Ativa: [" << obj.id << "] " << obj.name << " (" << obj.materialName << ")";
            glColor3f(0.4f, 0.9f, 1.0f);
        }
        drawText(panelX + 15.0f, currentY, ssEnt.str());

        // Estatísticas
        currentY += 20.0f;
        std::ostringstream ssStats;
        ssStats << std::fixed << std::setprecision(2);
        if (scene.selectedEntity == 0) {
            ssStats << "Posicao Luz (X,Y,Z): ("
                    << lighting.positionalPos[0] << ", "
                    << lighting.positionalPos[1] << ", "
                    << lighting.positionalPos[2] << ")";
            glColor3f(0.85f, 0.85f, 0.85f);
        } else {
            const auto& obj = scene.objects[scene.selectedEntity - 1];
            ssStats << "Pos: (" << obj.currentTransform.posX << ", "
                                << obj.currentTransform.posY << ", "
                                << obj.currentTransform.posZ << ") "
                    << "Rot: (" << int(obj.currentTransform.rotX) << ", "
                                << int(obj.currentTransform.rotY) << ", "
                                << int(obj.currentTransform.rotZ) << ") "
                    << "Esc: "  << obj.currentTransform.scale << "x";
            glColor3f(0.85f, 0.85f, 0.85f);
        }
        drawText(panelX + 15.0f, currentY, ssStats.str());

        // Projeção
        currentY += 22.0f;
        std::string projStr = std::string("Projecao [P]: ") + camera.getProjectionName();
        glColor3f(0.9f, 0.7f, 1.0f);
        drawText(panelX + 15.0f, currentY, projStr);

        // Status Iluminação
        currentY += 22.0f;
        glColor3f(0.7f, 1.0f, 0.7f);
        std::ostringstream ssLight;
        ssLight << "Luzes: [F1] Direcional: " << (lighting.directionalEnabled ? "LIG" : "DES")
                << " | [F2] Posicional: "     << (lighting.positionalEnabled ? "LIG" : "DES");
        drawText(panelX + 15.0f, currentY, ssLight.str());

        currentY += 18.0f;
        std::ostringstream ssComp;
        ssComp << "Componentes: [F3] Amb: " << (lighting.ambientEnabled ? "ON " : "OFF")
               << "  [F4] Dif: "           << (lighting.diffuseEnabled ? "ON " : "OFF")
               << "  [F5] Esp: "           << (lighting.specularEnabled ? "ON" : "OFF");
        drawText(panelX + 15.0f, currentY, ssComp.str());

        // Guia de Teclas
        if (showHelp) {
            currentY += 26.0f;
            glColor4f(0.35f, 0.45f, 0.55f, 0.8f);
            glBegin(GL_LINES);
            glVertex2f(panelX + 15.0f, currentY - 8.0f);
            glVertex2f(panelX + panelW - 15.0f, currentY - 8.0f);
            glEnd();

            glColor3f(1.0f, 0.85f, 0.3f);
            drawText(panelX + 15.0f, currentY, "GUIA DE TECLAS E INTERACAO:");

            glColor3f(0.85f, 0.85f, 0.9f);
            currentY += 18.0f;
            drawText(panelX + 15.0f, currentY, "1, 2, 3, 4: Selecionar Obras | 0: Selecionar Luz Posicional");
            currentY += 16.0f;
            drawText(panelX + 15.0f, currentY, "W / S: Transladar no Eixo Z | A / D: Transladar no Eixo X");
            currentY += 16.0f;
            drawText(panelX + 15.0f, currentY, "Q / E: Transladar no Eixo Y | + / -: Aumentar/Diminuir Escala");
            currentY += 16.0f;
            drawText(panelX + 15.0f, currentY, "I / K: Rotacionar Eixo X   | J / L: Rotacionar Eixo Y");
            currentY += 16.0f;
            drawText(panelX + 15.0f, currentY, "U / O: Rotacionar Eixo Z   | R: Restaurar Padroes (Reset)");
            currentY += 16.0f;
            drawText(panelX + 15.0f, currentY, "Setas: Rotacionar Camera   | PgUp / PgDn: Zoom In/Out");
            currentY += 16.0f;
            drawText(panelX + 15.0f, currentY, "Shift + Setas: Deslocar Ponto de Observacao (Pan)");
            currentY += 16.0f;
            drawText(panelX + 15.0f, currentY, "F1-F5: Controle Iluminacao | H: Ocultar/Exibir Ajuda");
        } else {
            currentY += 22.0f;
            glColor3f(0.6f, 0.6f, 0.6f);
            drawText(panelX + 15.0f, currentY, "Pressione [H] para expandir o guia de atalhos.");
        }

        glPopAttrib();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
};

// ============================================================================
// 7. VARIÁVEIS GLOBAIS E CALLBACKS DO FREEGLUT
// ============================================================================

int windowWidth = 1280;
int windowHeight = 720;

Camera camera;
LightingManager lighting;
Scene scene;
HUD hud;

const float TRANSLATE_STEP   = 0.25f;
const float ROTATE_STEP      = 5.0f;
const float SCALE_STEP       = 0.08f;
const float CAMERA_ROT_STEP  = 3.0f;
const float CAMERA_ZOOM_STEP = 0.5f;
const float CAMERA_PAN_STEP  = 0.3f;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.applyProjection(windowWidth, windowHeight);
    camera.applyView();
    lighting.applyLighting();
    scene.render(lighting);
    hud.render(windowWidth, windowHeight, camera, lighting, scene);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = (h > 0) ? h : 1;
    glViewport(0, 0, windowWidth, windowHeight);
    glutPostRedisplay();
}

void keyboard(unsigned char key, int /*x*/, int /*y*/) {
    switch (key) {
        // Seleção de Objetos e Luz Posicional
        case '1': scene.selectEntity(1); break;
        case '2': scene.selectEntity(2); break;
        case '3': scene.selectEntity(3); break;
        case '4': scene.selectEntity(4); break;
        case '0': scene.selectEntity(0); break;

        // Translação no Eixo Z (W / S)
        case 'w': case 'W': scene.translateSelected(0.0f, 0.0f, -TRANSLATE_STEP, lighting); break;
        case 's': case 'S': scene.translateSelected(0.0f, 0.0f,  TRANSLATE_STEP, lighting); break;

        // Translação no Eixo X (A / D)
        case 'a': case 'A': scene.translateSelected(-TRANSLATE_STEP, 0.0f, 0.0f, lighting); break;
        case 'd': case 'D': scene.translateSelected( TRANSLATE_STEP, 0.0f, 0.0f, lighting); break;

        // Translação no Eixo Y (Q / E)
        case 'q': case 'Q': scene.translateSelected(0.0f, -TRANSLATE_STEP, 0.0f, lighting); break;
        case 'e': case 'E': scene.translateSelected(0.0f,  TRANSLATE_STEP, 0.0f, lighting); break;

        // Rotação no Eixo X (I / K)
        case 'i': case 'I': scene.rotateSelected(-ROTATE_STEP, 0.0f, 0.0f); break;
        case 'k': case 'K': scene.rotateSelected( ROTATE_STEP, 0.0f, 0.0f); break;

        // Rotação no Eixo Y (J / L)
        case 'j': case 'J': scene.rotateSelected(0.0f, -ROTATE_STEP, 0.0f); break;
        case 'l': case 'L': scene.rotateSelected(0.0f,  ROTATE_STEP, 0.0f); break;

        // Rotação no Eixo Z (U / O)
        case 'u': case 'U': scene.rotateSelected(0.0f, 0.0f, -ROTATE_STEP); break;
        case 'o': case 'O': scene.rotateSelected(0.0f, 0.0f,  ROTATE_STEP); break;

        // Escala (+ / -)
        case '+': case '=': scene.scaleSelected( SCALE_STEP); break;
        case '-': case '_': scene.scaleSelected(-SCALE_STEP); break;

        // Alternar Projeção (P)
        case 'p': case 'P': camera.toggleProjection(); break;

        // Restaurar Configurações Iniciais (R)
        case 'r': case 'R':
            camera.reset();
            lighting.reset();
            scene.reset();
            break;

        // Alternar Ajuda no HUD (H)
        case 'h': case 'H': hud.toggleHelp(); break;

        // Sair (Esc)
        case 27:
            glutLeaveMainLoop();
            return;

        default:
            break;
    }

    glutPostRedisplay();
}

void specialKeys(int key, int /*x*/, int /*y*/) {
    int modifiers = glutGetModifiers();
    bool shiftPressed = (modifiers & GLUT_ACTIVE_SHIFT) != 0;

    switch (key) {
        // Câmera: Rotacionar ou Deslocar Ponto de Observação (Shift + Setas)
        case GLUT_KEY_LEFT:
            if (shiftPressed) camera.pan(-CAMERA_PAN_STEP, 0.0f);
            else camera.rotate(-CAMERA_ROT_STEP, 0.0f);
            break;

        case GLUT_KEY_RIGHT:
            if (shiftPressed) camera.pan(CAMERA_PAN_STEP, 0.0f);
            else camera.rotate(CAMERA_ROT_STEP, 0.0f);
            break;

        case GLUT_KEY_UP:
            if (shiftPressed) camera.pan(0.0f, CAMERA_PAN_STEP);
            else camera.rotate(0.0f, CAMERA_ROT_STEP);
            break;

        case GLUT_KEY_DOWN:
            if (shiftPressed) camera.pan(0.0f, -CAMERA_PAN_STEP);
            else camera.rotate(0.0f, -CAMERA_ROT_STEP);
            break;

        // Zoom (Page Up / Page Down)
        case GLUT_KEY_PAGE_UP:
            camera.zoom(-CAMERA_ZOOM_STEP);
            break;

        case GLUT_KEY_PAGE_DOWN:
            camera.zoom(CAMERA_ZOOM_STEP);
            break;

        // Controle de Iluminação (F1 - F5)
        case GLUT_KEY_F1: lighting.toggleDirectional(); break;
        case GLUT_KEY_F2: lighting.togglePositional();  break;
        case GLUT_KEY_F3: lighting.toggleAmbient();     break;
        case GLUT_KEY_F4: lighting.toggleDiffuse();     break;
        case GLUT_KEY_F5: lighting.toggleSpecular();    break;

        default:
            break;
    }

    glutPostRedisplay();
}

void initGL() {
    glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    lighting.init();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 80);

    glutCreateWindow("Galeria de Arte 3D Interativa - Computacao Grafica (FreeGLUT / OpenGL)");

    initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();
    return 0;
}
