// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/render/hud.cpp
//
// Descrição:
//   Implementa a projeção de elementos 3D na tela (`GetWorldToScreen`) e os
//   painéis de telemetria e explicação didática das transformações de CG.
// ============================================================================

#include "render/hud.h"
#include "raymath.h"

void RenderizarHUD(const JogoEstado &jogo) {
    const int larguraTela = GetScreenWidth();
    const int alturaTela  = GetScreenHeight();

    // 1. Retículo de Mira 3D Projetado para Coordenadas de Tela (Viewport Transform)
    Vector3 fwd, up, right;
    CalcularOrientacaoNave(jogo.nave, fwd, up, right);
    Vector3 pontoMira3D = Vector3Add(jogo.nave.pos, Vector3Scale(fwd, 22.0f));
    Vector2 miraTela    = GetWorldToScreen(pontoMira3D, jogo.camera);

    if (miraTela.x > 0 && miraTela.x < larguraTela && miraTela.y > 0 && miraTela.y < alturaTela) {
        int mx = (int)miraTela.x;
        int my = (int)miraTela.y;
        DrawCircleLines(mx, my, 14, (Color){0, 240, 255, 200});
        DrawCircleLines(mx, my, 4, (Color){255, 255, 255, 230});
        DrawLine(mx - 24, my, mx - 8, my, (Color){0, 240, 255, 180});
        DrawLine(mx + 8, my, mx + 24, my, (Color){0, 240, 255, 180});
        DrawLine(mx, my - 24, mx, my - 8, (Color){0, 240, 255, 180});
        DrawLine(mx, my + 8, mx, my + 24, (Color){0, 240, 255, 180});
    }

    // 2. Caixas Holográficas de Rastreamento de Alvos (Bounding Brackets)
    Vector3 dirCam = Vector3Subtract(jogo.camera.target, jogo.camera.position);
    for (const auto &a : jogo.alvos) {
        if (!a.ativo)
            continue;

        // Teste de visibilidade frontal via Produto Escalar (evita desenhar alvos atrás da câmera)
        Vector3 dirAlvo = Vector3Subtract(a.pos, jogo.camera.position);
        if (Vector3DotProduct(dirCam, dirAlvo) > 0.0f) {
            Vector2 posTela = GetWorldToScreen(a.pos, jogo.camera);
            float dist      = Vector3Distance(jogo.nave.pos, a.pos);

            int bx     = (int)posTela.x;
            int by     = (int)posTela.y;
            int tamBox = (int)Clamp(400.0f / dist, 12.0f, 50.0f);

            DrawLine(bx - tamBox, by - tamBox, bx - tamBox + 6, by - tamBox, a.cor);
            DrawLine(bx - tamBox, by - tamBox, bx - tamBox, by - tamBox + 6, a.cor);
            DrawLine(bx + tamBox, by - tamBox, bx + tamBox - 6, by - tamBox, a.cor);
            DrawLine(bx + tamBox, by - tamBox, bx + tamBox, by - tamBox + 6, a.cor);
            DrawLine(bx - tamBox, by + tamBox, bx - tamBox + 6, by + tamBox, a.cor);
            DrawLine(bx - tamBox, by + tamBox, bx - tamBox, by + tamBox - 6, a.cor);
            DrawLine(bx + tamBox, by + tamBox, bx + tamBox - 6, by + tamBox, a.cor);
            DrawLine(bx + tamBox, by + tamBox, bx + tamBox, by + tamBox - 6, a.cor);

            DrawText(TextFormat("%.0fm", dist), bx - 14, by + tamBox + 4, 11, a.cor);
        }
    }

    // 3. Barra Superior de Status e Pontuação
    DrawRectangle(0, 0, larguraTela, 48, (Color){10, 15, 30, 220});
    DrawLine(0, 48, larguraTela, 48, (Color){0, 180, 255, 150});

    DrawText("COMPUTACAO GRAFICA 3D - SPACE COMBAT", 20, 14, 18, (Color){0, 220, 255, 255});
    DrawText(TextFormat("PONTUACAO: %05d", jogo.pontuacao), larguraTela - 240, 14, 18, YELLOW);
    DrawText(TextFormat("ONDA: %d", jogo.ondaAtual), larguraTela - 360, 14, 18, LIME);

    // 4. Banner Central de Eventos (Novas Ondas e Checkpoints)
    if (jogo.alertaTemporizador > 0.0f) {
        int tamTxt = MeasureText(jogo.alertaTexto.c_str(), 20);
        int ax     = (larguraTela - tamTxt) / 2;
        DrawRectangle(ax - 20, 60, tamTxt + 40, 36, (Color){0, 20, 40, 210});
        DrawRectangleLines(ax - 20, 60, tamTxt + 40, 36, (Color){0, 240, 255, 230});
        DrawText(jogo.alertaTexto.c_str(), ax, 68, 20, (Color){0, 255, 220, 255});
    }

    // 5. Painel Inferior Esquerdo: Telemetria de Navegação e Câmera
    DrawRectangle(15, alturaTela - 80, 290, 65, (Color){10, 15, 30, 200});
    DrawRectangleLines(15, alturaTela - 80, 290, 65, (Color){0, 180, 255, 120});
    DrawText(TextFormat("VELOCIDADE: %.1f km/s", jogo.nave.velocidadeAtual * 10.0f), 25, alturaTela - 72, 13, WHITE);
    DrawText(TextFormat("POSICAO: (X:%.1f, Y:%.1f, Z:%.1f)", jogo.nave.pos.x, jogo.nave.pos.y, jogo.nave.pos.z), 25, alturaTela - 54, 12, LIGHTGRAY);

    const char *strCam = (jogo.modoCamera == ModoCamera::Chase)
                             ? "3a PESSOA (CHASE)"
                             : (jogo.modoCamera == ModoCamera::Cockpit ? "1a PESSOA (COCKPIT)" : "TATICA (ORBITAL)");
    DrawText(TextFormat("CAMERA [C]: %s", strCam), 25, alturaTela - 36, 12, (Color){0, 240, 255, 255});

    // 6. Painel Inferior Direito: Resumo Didático de Computação Gráfica (Alternável com [H])
    if (jogo.mostrarHUD_CG) {
        int pw = 360;
        int ph = 175;
        int px = larguraTela - pw - 15;
        int py = alturaTela - ph - 15;

        DrawRectangle(px, py, pw, ph, (Color){8, 12, 24, 230});
        DrawRectangleLines(px, py, pw, ph, (Color){0, 200, 255, 180});

        DrawText("TRANSFORMACOES GEOMETRICAS 3D (CG)", px + 15, py + 10, 13, (Color){0, 240, 255, 255});
        DrawText("Pipeline: M = T(x,y,z) * Ry(yaw) * Rx(pitch) * Rz(roll) * S", px + 15, py + 30, 11, YELLOW);
        DrawText(TextFormat("- Yaw (Guinada Y):   %.1f deg", jogo.nave.yaw), px + 15, py + 48, 11, LIGHTGRAY);
        DrawText(TextFormat("- Pitch (Arfagem X): %.1f deg (Eixo Y %s [I])", jogo.nave.pitch, jogo.inverterPitch ? "Invertido" : "Direto"),
                 px + 15, py + 64, 11, LIGHTGRAY);
        DrawText(TextFormat("- Roll (Rolagem Z):  %.1f deg (Banking Dinamico)", jogo.nave.roll), px + 15, py + 80, 11, LIGHTGRAY);
        DrawText("- Projecao: Perspectiva 3D (FOV 60 deg)", px + 15, py + 96, 11, SKYBLUE);

        DrawText("CONTROLES: W/S/A/D ou Setas = Voo | Q/E = Roll", px + 15, py + 118, 11, RAYWHITE);
        DrawText("ESPACO = Laser | SHIFT = Turbo | C = Camera", px + 15, py + 134, 11, RAYWHITE);
        DrawText("R = Reiniciar | H = Ocultar Painel CG", px + 15, py + 150, 11, (Color){160, 200, 255, 200});
    } else {
        DrawText("Pressione [H] para exibir detalhes de CG e Controles", larguraTela - 380, alturaTela - 25, 12, GRAY);
    }
}
