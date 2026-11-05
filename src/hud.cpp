#include "hud.h"
#include <GL/freeglut.h>
#include <iomanip>
#include <sstream>

HUD::HUD() : showHelp(false) {}

void HUD::toggleHelp() {
    showHelp = !showHelp;
}

void HUD::drawText(float x, float y, const std::string& text, void* font) {
    if (!font) font = GLUT_BITMAP_HELVETICA_12;
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

void HUD::drawPanel(float x, float y, float w, float h, float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void HUD::render(int windowWidth, int windowHeight,
                const Camera& camera,
                const LightingManager& lighting,
                const Scene& scene) {
    // Configurar projeção ortográfica 2D sobre a janela
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, windowHeight, 0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Salvar e desativar estados 3D para desenho do HUD 2D
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Painel Superior Esquerdo: Informações de Estado
    float panelX = 15.0f;
    float panelY = 15.0f;
    float panelW = 430.0f;
    float panelH = showHelp ? 365.0f : 165.0f;

    // Fundo semitransparente escuro
    drawPanel(panelX, panelY, panelW, panelH, 0.08f, 0.09f, 0.12f, 0.85f);

    // Borda do painel
    glColor4f(0.3f, 0.45f, 0.65f, 0.9f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(panelX, panelY);
    glVertex2f(panelX + panelW, panelY);
    glVertex2f(panelX + panelW, panelY + panelH);
    glVertex2f(panelX, panelY + panelH);
    glEnd();

    // Título Principal
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

    // Estatísticas da Entidade Selecionada
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

    // Modo de Projeção
    currentY += 22.0f;
    std::string projStr = std::string("Projecao [P]: ") + camera.getProjectionName();
    glColor3f(0.9f, 0.7f, 1.0f);
    drawText(panelX + 15.0f, currentY, projStr);

    // Status da Iluminação (F1 a F5)
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

    // 2. Seção de Guia de Teclas / Controles
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

    // Restaurar matrizes e atributos
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
