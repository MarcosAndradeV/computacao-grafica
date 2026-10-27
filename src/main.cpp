// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo principal: src/main.cpp
//
// Descrição:
//   Ponto de entrada. Cria a janela OpenGL (MSAA 4x), mantém dois estados
//   separados — `JogoEstado` (simulação) e `EstadoApp` (tela ativa) — e roda
//   o laço principal: atualizar conforme a tela e desenhar.
//
// Estrutura de pastas:
//   - src/core/entidades.*    -> Entidades 3D, constantes (Config) e matemática auxiliar
//   - src/core/estado.*       -> Tela ativa (menu / jogando / pausado) e controle do cursor
//   - src/input/input.*       -> Teclado, mouse e gamepad unificados em ComandosEntrada
//   - src/fisica/logica.*     -> Voo (Euler), disparo, colisões, ondas, anéis, partículas e câmera
//   - src/render/modelos_3d.* -> Malha triangular da nave e transformações dos alvos
//   - src/render/cena_3d.*    -> Passe 3D (BeginMode3D) e pilha de matrizes
//   - src/render/hud.*        -> Mira projetada, marcadores e painel didático
//   - src/render/menu.*       -> Menu principal, submenus e menu de pausa
// ============================================================================

#include "core/entidades.h"
#include "core/estado.h"
#include "input/input.h"
#include "fisica/logica.h"
#include "render/cena_3d.h"
#include "render/hud.h"
#include "render/menu.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>

int main() {
    // Semente aleatória: cada partida gera um mapa diferente
    std::srand((unsigned int)std::time(nullptr));

    // 1. Janela e contexto OpenGL
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(Config::LARGURA_INICIAL, Config::ALTURA_INICIAL, "Space Combat 3D - Transformacoes Geometricas - Raylib");
    SetExitKey(KEY_NULL);  // Sem isto o raylib fecha a janela com ESC; aqui ESC pausa/volta
    SetTargetFPS(Config::FPS_ALVO);
    MaximizeWindow();

    // 2. Estados: simulação e tela ativa (começa no menu, com o cursor visível)
    JogoEstado jogo;
    EstadoApp app;
    ReiniciarJogoCompleto(jogo);

    // 3. Laço principal
    while (!WindowShouldClose() && !app.deveSair) {
        float dt = std::min(GetFrameTime(), Config::DT_MAXIMO);

        // 3.1 Atualização (depende da tela)
        switch (app.tela) {
        case EstadoTela::Menu:
            jogo.faseAnimacao += dt * Config::VEL_FASE_MENU;  // Só anima o cenário de fundo
            AtualizarMenu(jogo, app);
            break;
        case EstadoTela::Jogando: {
            ComandosEntrada cmd = LerComandosEntrada();
            if (cmd.pausar)
                MudarEstado(app, EstadoTela::Pausado);
            else
                AtualizarJogo(jogo, cmd, dt);
            break;
        }
        case EstadoTela::Pausado:
            AtualizarMenu(jogo, app);  // Mundo congelado
            break;
        }

        // 3.2 Desenho: o mundo 3D aparece em todas as telas; muda só a camada de interface
        BeginDrawing();
        ClearBackground(Color{8, 10, 20, 255});  // Fundo espacial escuro

        RenderizarMundo3D(jogo);

        switch (app.tela) {
        case EstadoTela::Menu:
            RenderizarMenuPrincipal(jogo, app);
            break;
        case EstadoTela::Jogando:
            RenderizarHUD(jogo);
            break;
        case EstadoTela::Pausado:
            RenderizarHUD(jogo);
            RenderizarMenuPausa(jogo, app);
            break;
        }

        EndDrawing();
    }

    // 4. Encerramento
    CloseWindow();
    return 0;
}
