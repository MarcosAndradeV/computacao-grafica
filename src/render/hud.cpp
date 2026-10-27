// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/render/hud.cpp
//
// Descrição:
//   Interface sobreposta ao mundo 3D. Projeta pontos do espaço do mundo para
//   a tela (`GetWorldToScreen`) para desenhar a mira e os marcadores dos
//   alvos, e exibe telemetria e o painel didático de Computação Gráfica.
// ============================================================================

#include "render/hud.h"
#include "input/input.h"
#include "raymath.h"

namespace {
    const Color CIANO_HUD = {0, 240, 255, 255};

    // Moldura de mira com quatro cantos de `braco` pixels, centrada em (cx, cy)
    void DesenharCantosMira(int cx, int cy, int meio, int braco, Color cor) {
        for (int sx : {-1, 1}) {
            for (int sy : {-1, 1}) {
                int x = cx + sx * meio;
                int y = cy + sy * meio;
                DrawLine(x, y, x - sx * braco, y, cor);  // Braço horizontal, para dentro
                DrawLine(x, y, x, y - sy * braco, cor);  // Braço vertical, para dentro
            }
        }
    }

    // Retículo central: projetado no ponto onde os lasers convergem
    void DesenharMira(const JogoEstado &jogo, int larguraTela, int alturaTela) {
        Vector3 pontoMira3D = Vector3Add(jogo.nave.pos, Vector3Scale(ObterFrenteNave(jogo.nave), Config::DISTANCIA_CONVERGENCIA));
        Vector2 mira = GetWorldToScreen(pontoMira3D, ObterCameraRender(jogo));

        if (mira.x <= 0 || mira.x >= larguraTela || mira.y <= 0 || mira.y >= alturaTela)
            return;

        int mx = (int)mira.x;
        int my = (int)mira.y;
        Color corMira = {0, 240, 255, 180};
        DrawCircleLines(mx, my, 14, Color{0, 240, 255, 200});
        DrawCircleLines(mx, my, 4, Color{255, 255, 255, 230});
        DrawLine(mx - 24, my, mx - 8, my, corMira);
        DrawLine(mx + 8, my, mx + 24, my, corMira);
        DrawLine(mx, my - 24, mx, my - 8, corMira);
        DrawLine(mx, my + 8, mx, my + 24, corMira);
    }

    // Colchetes holográficos sobre cada alvo visível, com a distância
    void DesenharMarcadoresAlvos(const JogoEstado &jogo) {
        Camera3D camera = ObterCameraRender(jogo);
        Vector3 dirCam  = Vector3Subtract(camera.target, camera.position);

        for (const auto &a : jogo.alvos) {
            if (!a.ativo)
                continue;

            // Produto escalar > 0: o alvo está à frente da câmera (evita projetar o que está atrás)
            if (Vector3DotProduct(dirCam, Vector3Subtract(a.pos, camera.position)) <= 0.0f)
                continue;

            Vector2 posTela = GetWorldToScreen(a.pos, camera);
            float dist      = Vector3Distance(jogo.nave.pos, a.pos);
            int tamBox      = (int)Clamp(400.0f / dist, 12.0f, 50.0f);  // Menor quanto mais longe

            DesenharCantosMira((int)posTela.x, (int)posTela.y, tamBox, 6, a.cor);
            DrawText(TextFormat("%.0fm", dist), (int)posTela.x - 14, (int)posTela.y + tamBox + 4, 11, a.cor);
        }
    }

    // Faixa de título e placar
    void DesenharBarraSuperior(const JogoEstado &jogo, int larguraTela) {
        DrawRectangle(0, 0, larguraTela, 48, Color{10, 15, 30, 220});
        DrawLine(0, 48, larguraTela, 48, Color{0, 180, 255, 150});

        DrawText("COMPUTACAO GRAFICA 3D - SPACE COMBAT", 20, 14, 18, Color{0, 220, 255, 255});
        DrawText(TextFormat("PONTUACAO: %05d", jogo.pontuacao), larguraTela - 240, 14, 18, YELLOW);
        DrawText(TextFormat("ONDA: %d", jogo.ondaAtual), larguraTela - 360, 14, 18, LIME);
    }

    // Banner central: aviso de fronteira (prioritário) ou mensagem de evento
    void DesenharBanner(const JogoEstado &jogo, int larguraTela) {
        const char *texto = nullptr;
        Color fundo, borda, corTexto;

        if (FracaoRaioMundo(jogo.nave) > Config::BORDA_ALERTA) {
            texto    = "! ALERTA: APROXIMANDO-SE DO LIMITE DO SETOR !";
            fundo    = Color{60, 10, 15, 230};
            borda    = Color{255, 60, 60, 240};
            corTexto = Color{255, 100, 100, 255};
        } else if (jogo.alertaTemporizador > 0.0f) {
            texto    = jogo.alertaTexto.c_str();
            fundo    = Color{0, 20, 40, 210};
            borda    = Color{0, 240, 255, 230};
            corTexto = Color{0, 255, 220, 255};
        }
        if (texto == nullptr)
            return;

        int tamTxt = MeasureText(texto, 20);
        int ax     = (larguraTela - tamTxt) / 2;
        DrawRectangle(ax - 20, 60, tamTxt + 40, 36, fundo);
        DrawRectangleLines(ax - 20, 60, tamTxt + 40, 36, borda);
        DrawText(texto, ax, 68, 20, corTexto);
    }

    // Telemetria no canto inferior esquerdo
    void DesenharTelemetria(const JogoEstado &jogo, int alturaTela) {
        DrawRectangle(15, alturaTela - 100, 310, 85, Color{10, 15, 30, 200});
        DrawRectangleLines(15, alturaTela - 100, 310, 85, Color{0, 180, 255, 120});
        DrawText(TextFormat("VELOCIDADE: %.1f km/s", jogo.nave.velocidadeAtual * 10.0f), 25, alturaTela - 92, 13, WHITE);
        DrawText(TextFormat("POSICAO: (X:%.1f, Y:%.1f, Z:%.1f)", jogo.nave.pos.x, jogo.nave.pos.y, jogo.nave.pos.z), 25, alturaTela - 74, 12, LIGHTGRAY);

        const char *strCam = "TATICA (ORBITAL)";
        if (jogo.modoCamera == ModoCamera::Chase)
            strCam = "3a PESSOA (CHASE)";
        else if (jogo.modoCamera == ModoCamera::Cockpit)
            strCam = "1a PESSOA (COCKPIT)";
        DrawText(TextFormat("CAMERA [C]: %s", strCam), 25, alturaTela - 56, 12, CIANO_HUD);

        bool gamepad = GamepadConectado();
        const char *strInput = gamepad ? TextFormat("GAMEPAD: %s", ObterNomeGamepad()) : "ENTRADA: TECLADO + MOUSE";
        DrawText(strInput, 25, alturaTela - 36, 11, gamepad ? GREEN : SKYBLUE);
    }

    // Painel didático (alternável com H): a fórmula do pipeline e os ângulos em tempo real
    void DesenharPainelDidatico(const JogoEstado &jogo, int larguraTela, int alturaTela) {
        const int pw = 370;
        const int ph = 190;
        const int px = larguraTela - pw - 15;
        const int py = alturaTela - ph - 15;

        DrawRectangle(px, py, pw, ph, Color{8, 12, 24, 230});
        DrawRectangleLines(px, py, pw, ph, Color{0, 200, 255, 180});

        DrawText("TRANSFORMACOES GEOMETRICAS 3D (CG)", px + 15, py + 10, 13, CIANO_HUD);
        DrawText("Pipeline: M = T(x,y,z) * Ry(yaw) * Rx(pitch) * Rz(roll) * S", px + 15, py + 28, 11, YELLOW);
        DrawText(TextFormat("- Yaw (Guinada Y):   %.1f deg", jogo.nave.yaw), px + 15, py + 44, 11, LIGHTGRAY);
        DrawText(TextFormat("- Pitch (Arfagem X): %.1f deg (Eixo Y %s [I])", jogo.nave.pitch, jogo.inverterPitch ? "Invertido" : "Direto"),
                 px + 15, py + 58, 11, LIGHTGRAY);
        DrawText(TextFormat("- Roll (Rolagem Z):  %.1f deg (Banking Dinamico)", jogo.nave.roll), px + 15, py + 72, 11, LIGHTGRAY);
        DrawText(TextFormat("- Projecao: Perspectiva 3D (FOV %.0f deg)", Config::FOV_PADRAO), px + 15, py + 86, 11, SKYBLUE);

        DrawText("TECLADO: W/S/A/D=Voo | Q/E=Roll | ESPACO=Laser | SHIFT=Nitro", px + 15, py + 106, 11, RAYWHITE);
        DrawText("MOUSE: Botao Esquerdo=Atirar | Botao Direito=Nitro", px + 15, py + 122, 11, RAYWHITE);
        DrawText("GAMEPAD: Stick=Voo | LB/RB=Roll | RT=Nitro | A/X=Laser", px + 15, py + 138, 11, RAYWHITE);
        DrawText("ATALHOS: C=Camera | I=Inverter Pitch | R=Reset | H=HUD", px + 15, py + 154, 11, Color{160, 200, 255, 200});
    }
}

void RenderizarHUD(const JogoEstado &jogo) {
    const int larguraTela = GetScreenWidth();
    const int alturaTela  = GetScreenHeight();

    DesenharMira(jogo, larguraTela, alturaTela);
    DesenharMarcadoresAlvos(jogo);
    DesenharBarraSuperior(jogo, larguraTela);
    DesenharBanner(jogo, larguraTela);
    DesenharTelemetria(jogo, alturaTela);

    if (jogo.mostrarHUD_CG) {
        DesenharPainelDidatico(jogo, larguraTela, alturaTela);
    } else {
        DrawText("Pressione [H] para exibir detalhes de CG e Controles", larguraTela - 380, alturaTela - 25, 12, GRAY);
    }
}
