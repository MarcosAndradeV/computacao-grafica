#include "hud.h"
#include <iomanip>
#include <sstream>

HUD::HUD() : showHelp(true) {}

void HUD::toggleHelp() { showHelp = !showHelp; }

void HUD::render(int windowWidth, int windowHeight, const GalleryCamera &camera, const LightingManager &lighting, const Scene &scene) {
    (void)windowWidth;
    (void)windowHeight;

    int panelX = 15;
    int panelY = 15;
    int panelW = 440;
    int panelH = showHelp ? 380 : 160;

    // Fundo semitransparente escuro elegante
    DrawRectangle(panelX, panelY, panelW, panelH, Fade(GetColor(0x12141CFF), 0.88f));
    DrawRectangleLines(panelX, panelY, panelW, panelH, Fade(SKYBLUE, 0.6f));

    // Título Principal
    DrawText("GALERIA DE ARTE 3D - COMPUTACAO GRAFICA", panelX + 15, panelY + 12, 14, GOLD);

    // Entidade Selecionada
    int currentY = panelY + 36;
    std::ostringstream ssEnt;
    if (scene.selectedEntity == 0) {
        ssEnt << "Entidade Ativa: [0] LUZ POSICIONAL (Movel)";
        DrawText(ssEnt.str().c_str(), panelX + 15, currentY, 13, YELLOW);
    } else {
        const auto &obj = scene.objects[scene.selectedEntity - 1];
        ssEnt << "Entidade Ativa: [" << obj.id << "] " << obj.name << " (" << obj.materialName << ")";
        DrawText(ssEnt.str().c_str(), panelX + 15, currentY, 13, SKYBLUE);
    }

    // Estatísticas da Entidade Selecionada
    currentY += 18;
    std::ostringstream ssStats;
    ssStats << std::fixed << std::setprecision(2);
    if (scene.selectedEntity == 0) {
        ssStats << "Posicao Luz (X,Y,Z): (" << lighting.positionalPos.x << ", " << lighting.positionalPos.y << ", " << lighting.positionalPos.z
                << ")";
        DrawText(ssStats.str().c_str(), panelX + 15, currentY, 12, LIGHTGRAY);
    } else {
        const auto &obj = scene.objects[scene.selectedEntity - 1];
        ssStats << "Pos: (" << obj.currentTransform.posX << ", " << obj.currentTransform.posY << ", " << obj.currentTransform.posZ << ") "
                << "Rot: (" << int(obj.currentTransform.rotX) << ", " << int(obj.currentTransform.rotY) << ", " << int(obj.currentTransform.rotZ)
                << ") "
                << "Esc: " << obj.currentTransform.scale << "x";
        DrawText(ssStats.str().c_str(), panelX + 15, currentY, 12, LIGHTGRAY);
    }

    // Modo de Projeção
    currentY += 20;
    std::string projStr = std::string("Projecao [P]: ") + camera.getProjectionName();
    DrawText(projStr.c_str(), panelX + 15, currentY, 12, PURPLE);

    // Status da Iluminação (F1 a F5)
    currentY += 20;
    std::ostringstream ssLight;
    ssLight << "Luzes: [F1] Direcional: " << (lighting.directionalEnabled ? "LIG" : "DES")
            << " | [F2] Posicional: " << (lighting.positionalEnabled ? "LIG" : "DES");
    DrawText(ssLight.str().c_str(), panelX + 15, currentY, 12, LIME);

    currentY += 16;
    std::ostringstream ssComp;
    ssComp << "Componentes: [F3] Amb: " << (lighting.ambientEnabled ? "ON " : "OFF") << "  [F4] Dif: " << (lighting.diffuseEnabled ? "ON " : "OFF")
           << "  [F5] Esp: " << (lighting.specularEnabled ? "ON" : "OFF");
    DrawText(ssComp.str().c_str(), panelX + 15, currentY, 12, GREEN);

    // Guia de Teclas / Controles
    if (showHelp) {
        currentY += 22;
        DrawLine(panelX + 15, currentY, panelX + panelW - 15, currentY, Fade(GRAY, 0.4f));

        currentY += 8;
        DrawText("GUIA DE TECLAS E INTERACAO (Conforme PDF):", panelX + 15, currentY, 12, GOLD);

        currentY += 16;
        DrawText("1, 2, 3, 4: Selecionar Obras | 0: Selecionar Luz Posicional", panelX + 15, currentY, 11, RAYWHITE);
        currentY += 14;
        DrawText("W / S: Transladar no Eixo Z | A / D: Transladar no Eixo X", panelX + 15, currentY, 11, RAYWHITE);
        currentY += 14;
        DrawText("Q / E: Transladar no Eixo Y | + / -: Aumentar/Diminuir Escala", panelX + 15, currentY, 11, RAYWHITE);
        currentY += 14;
        DrawText("I / K: Rotacionar Eixo X   | J / L: Rotacionar Eixo Y", panelX + 15, currentY, 11, RAYWHITE);
        currentY += 14;
        DrawText("U / O: Rotacionar Eixo Z   | R: Restaurar Padroes (Reset)", panelX + 15, currentY, 11, RAYWHITE);
        currentY += 14;
        DrawText("Setas: Rotacionar Camera   | PgUp / PgDn: Zoom In/Out", panelX + 15, currentY, 11, RAYWHITE);
        currentY += 14;
        DrawText("Shift + Setas: Deslocar Ponto de Observacao (Pan)", panelX + 15, currentY, 11, RAYWHITE);
        currentY += 14;
        DrawText("F1-F5: Controle Iluminacao | H: Ocultar/Exibir Ajuda", panelX + 15, currentY, 11, RAYWHITE);
    } else {
        currentY += 20;
        DrawText("Pressione [H] para expandir o guia de atalhos.", panelX + 15, currentY, 11, GRAY);
    }
}
