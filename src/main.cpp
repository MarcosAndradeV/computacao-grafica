// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo Principal: src/main.cpp
//
// Descrição:
//   Ponto de entrada da aplicação. Inicializa o contexto OpenGL/Raylib com
//   Anti-Aliasing (MSAA 4x), instancia a estrutura `JogoEstado` e executa o
//   Game Loop em tempo real orquestrando a Física e o Pipeline de Renderização
//   (Cena 3D + Interface HUD).
//
// Estrutura de Pastas e Módulos:
//   - src/core/entidades.h/.cpp    -> Estruturas 3D, estado do jogo e matemática vetorial
//   - src/fisica/logica.h/.cpp     -> Dinâmica de voo (Euler), colisões, partículas e câmera
//   - src/render/modelos_3d.h/.cpp -> Malha triangular explícita da nave e matrizes dos alvos
//   - src/render/cena_3d.h/.cpp    -> Renderização do Mundo 3D (BeginMode3D)
//   - src/render/hud.h/.cpp        -> Interface HUD, mira projetada e painel didático de CG
// ============================================================================

#include "core/entidades.h"
#include "fisica/logica.h"
#include "render/cena_3d.h"
#include "render/hud.h"

int main() {
    // 1. Configuração da Janela e Contexto Gráfico OpenGL (com MSAA 4x)
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(
        Config::LARGURA_INICIAL,
        Config::ALTURA_INICIAL,
        "Space Combat 3D - Transformacoes Geometricas - Raylib"
    );
    SetTargetFPS(Config::FPS_ALVO);
    MaximizeWindow();

    // 2. Instanciação e Inicialização do Estado do Jogo (sem variáveis globais soltas)
    JogoEstado jogo;
    ReiniciarJogoCompleto(jogo);

    // 3. Loop Principal de Simulação e Renderização em Tempo Real
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Limita o passo de tempo (delta time) para evitar saltos numéricos ao redimensionar a janela
        if (dt > 0.1f)
            dt = 0.1f;

        // Etapa 1: Atualização de Física, Controles, Colisões e Matriz de Câmera
        AtualizarJogo(jogo, dt);

        // Etapa 2: Pipeline de Renderização (Cena 3D + Interface HUD)
        BeginDrawing();
        ClearBackground((Color){8, 10, 20, 255}); // Fundo espacial profundo

        RenderizarMundo3D(jogo);
        RenderizarHUD(jogo);

        EndDrawing();
    }

    // 4. Encerramento do Contexto Gráfico
    CloseWindow();
    return 0;
}
