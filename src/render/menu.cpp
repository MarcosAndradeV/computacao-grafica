// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/render/menu.cpp
//
// Descrição:
//   Implementação do menu principal, dos submenus de Controles e Sobre o
//   Projeto, e do menu de pausa. Os textos da interface usam só ASCII porque
//   a fonte padrão do raylib não cobre bem os caracteres acentuados.
// ============================================================================

#include "render/menu.h"
#include "raylib.h"

namespace {
    const Color CIANO_TITULO  = {0, 240, 255, 255};
    const Color COR_PAINEL    = {8, 14, 28, 235};
    const Color COR_BORDA     = {0, 180, 255, 180};
    const Color COR_DIVISORIA = {0, 140, 200, 100};

    // Botão interativo. Retorna true no quadro em que é clicado.
    bool DesenharBotao(Rectangle rect, const char *texto, bool destaque = false) {
        bool sobre = CheckCollisionPointRec(GetMousePosition(), rect);

        Color corFundo = sobre ? (destaque ? Color{0, 180, 255, 230} : Color{20, 45, 80, 230})
                               : (destaque ? Color{0, 120, 200, 200} : Color{12, 22, 42, 200});
        Color corBorda = sobre ? Color{0, 240, 255, 255} : Color{0, 160, 220, 150};
        Color corTexto = sobre ? WHITE : (destaque ? Color{200, 245, 255, 255} : LIGHTGRAY);

        DrawRectangleRec(rect, corFundo);
        DrawRectangleLinesEx(rect, sobre ? 2.0f : 1.0f, corBorda);

        int tamFonte     = destaque ? 20 : 18;
        int larguraTexto = MeasureText(texto, tamFonte);
        DrawText(texto, (int)(rect.x + (rect.width - larguraTexto) / 2), (int)(rect.y + (rect.height - tamFonte) / 2), tamFonte, corTexto);

        return sobre && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    // Retângulo centralizado horizontalmente
    Rectangle BotaoCentralizado(int larguraTela, float y, float largura, float altura) {
        return Rectangle{(larguraTela - largura) / 2.0f, y, largura, altura};
    }

    void DesenharPainel(int x, int y, int largura, int altura) {
        DrawRectangle(x, y, largura, altura, COR_PAINEL);
        DrawRectangleLinesEx(Rectangle{(float)x, (float)y, (float)largura, (float)altura}, 2.0f, COR_BORDA);
    }

    void DesenharTituloCentralizado(const char *texto, int larguraTela, int y, int tamFonte, Color cor) {
        DrawText(texto, (larguraTela - MeasureText(texto, tamFonte)) / 2, y, tamFonte, cor);
    }

    void DesenharSubmenuControles(JogoEstado &, EstadoApp &app, int larguraTela, int painelX, int painelY, int painelW, int painelH) {
        DesenharTituloCentralizado("CONTROLES DA NAVE", larguraTela, painelY + 25, 24, CIANO_TITULO);
        DrawLine(painelX + 30, painelY + 60, painelX + painelW - 30, painelY + 60, COR_DIVISORIA);

        int y = painelY + 80;
        auto linha = [&](const char *acao, const char *teclas) {
            DrawText(acao, painelX + 40, y, 13, LIGHTGRAY);
            DrawText(teclas, painelX + 260, y, 13, CIANO_TITULO);
            y += 26;
        };

        linha("Arfagem (Pitch):", "W / S ou Setas Cima/Baixo");
        linha("Guinada (Yaw):", "A / D ou Setas Esq/Dir");
        linha("Rolagem (Roll):", "Q / E (Manual)");
        linha("Disparo Laser:", "Botao Esquerdo Mouse ou ESPACO");
        linha("Nitro / Turbo:", "Botao Direito Mouse ou SHIFT");
        linha("Freio Espacial:", "CTRL Esquerdo");
        linha("Alternar Camera:", "C ou TAB (Chase/Cockpit/Orbital)");
        linha("Inverter Eixo Y:", "I (Direto / Aviacao)");
        linha("Painel Didatico CG:", "H (Exibe telemetria em tela)");
        linha("Pausar Jogo:", "ESC");
        linha("Gamepad:", "Totalmente suportado (Plug & Play)");

        if (DesenharBotao(BotaoCentralizado(larguraTela, (float)(painelY + painelH - 55), 200, 38), "< VOLTAR [ESC]")) {
            app.subMenu = SubMenu::Principal;
        }
    }

    void DesenharSubmenuSobre(JogoEstado &, EstadoApp &app, int larguraTela, int painelX, int painelY, int painelW, int painelH) {
        DesenharTituloCentralizado("CONCEITOS DE COMPUTACAO GRAFICA", larguraTela, painelY + 25, 22, CIANO_TITULO);
        DrawLine(painelX + 30, painelY + 58, painelX + painelW - 30, painelY + 58, COR_DIVISORIA);

        int y = painelY + 75;
        auto paragrafo = [&](const char *cabecalho, const char *corpo) {
            DrawText(cabecalho, painelX + 35, y, 14, YELLOW);
            DrawText(corpo, painelX + 35, y + 18, 12, LIGHTGRAY);
            y += 48;
        };

        paragrafo("1. Transformacoes Geometricas (M = T * R * S):",
                  "Pilha rlPushMatrix/rlPopMatrix para isolar sistemas de coordenadas locais.");
        paragrafo("2. Modelagem Poligonal 3D Explicita:",
                  "Nave modelada vertice a vertice com rlBegin(RL_TRIANGLES) e cores por face.");
        paragrafo("3. Camera Sintetica (View Matrix):",
                  "3 modos com LookAt (Chase inercial, Cockpit e Tatica Orbital).");
        paragrafo("4. Projecao Perspectiva e Mundo->Tela:",
                  "Transformacao 3D para viewport 2D (GetWorldToScreen) para mira e marcadores.");
        paragrafo("5. Convergencia Balistica e Colisoes:",
                  "Lasers convergem na mira; colisao segmento x esfera e travessia de plano nos aneis.");

        if (DesenharBotao(BotaoCentralizado(larguraTela, (float)(painelY + painelH - 55), 200, 38), "< VOLTAR [ESC]")) {
            app.subMenu = SubMenu::Principal;
        }
    }
}

void AtualizarMenu(JogoEstado &jogo, EstadoApp &app) {
    if (app.tela == EstadoTela::Menu) {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            if (app.subMenu == SubMenu::Principal)
                IniciarMissao(jogo, app);
            else
                app.subMenu = SubMenu::Principal;
        }
        if (IsKeyPressed(KEY_ESCAPE) && app.subMenu != SubMenu::Principal) {
            app.subMenu = SubMenu::Principal;
        }
    } else if (app.tela == EstadoTela::Pausado) {
        if (IsKeyPressed(KEY_ESCAPE))
            MudarEstado(app, EstadoTela::Jogando);
        else if (IsKeyPressed(KEY_R))
            IniciarMissao(jogo, app);
    }
}

void RenderizarMenuPrincipal(JogoEstado &jogo, EstadoApp &app) {
    const int larguraTela = GetScreenWidth();
    const int alturaTela  = GetScreenHeight();

    const int painelW = 540;
    const int painelH = (app.subMenu == SubMenu::Principal) ? 440 : 520;
    const int painelX = (larguraTela - painelW) / 2;
    const int painelY = (alturaTela - painelH) / 2;

    DesenharPainel(painelX, painelY, painelW, painelH);

    if (app.subMenu == SubMenu::Controles) {
        DesenharSubmenuControles(jogo, app, larguraTela, painelX, painelY, painelW, painelH);
        return;
    }
    if (app.subMenu == SubMenu::SobreCG) {
        DesenharSubmenuSobre(jogo, app, larguraTela, painelX, painelY, painelW, painelH);
        return;
    }

    // Menu principal
    DesenharTituloCentralizado("SPACE COMBAT 3D", larguraTela, painelY + 35, 32, CIANO_TITULO);
    DesenharTituloCentralizado("Computacao Grafica - Simulador Espacial", larguraTela, painelY + 75, 14, Color{160, 200, 255, 220});
    DrawLine(painelX + 40, painelY + 105, painelX + painelW - 40, painelY + 105, COR_DIVISORIA);

    const float btnW = 320, btnH = 46, espaco = 60;
    float y = (float)(painelY + 130);

    if (DesenharBotao(BotaoCentralizado(larguraTela, y, btnW, btnH), "INICIAR MISSAO [ENTER]", true))
        IniciarMissao(jogo, app);
    if (DesenharBotao(BotaoCentralizado(larguraTela, y + espaco, btnW, btnH), "CONTROLES E COMANDOS"))
        app.subMenu = SubMenu::Controles;
    if (DesenharBotao(BotaoCentralizado(larguraTela, y + espaco * 2, btnW, btnH), "SOBRE O PROJETO (CG)"))
        app.subMenu = SubMenu::SobreCG;
    if (DesenharBotao(BotaoCentralizado(larguraTela, y + espaco * 3, btnW, btnH), "SAIR"))
        app.deveSair = true;

    DesenharTituloCentralizado("Use o mouse para clicar ou ENTER para iniciar", larguraTela, painelY + painelH - 28, 12, GRAY);
}

void RenderizarMenuPausa(JogoEstado &jogo, EstadoApp &app) {
    const int larguraTela = GetScreenWidth();
    const int alturaTela  = GetScreenHeight();

    DrawRectangle(0, 0, larguraTela, alturaTela, Color{4, 8, 16, 180});  // Escurece a cena congelada

    const int painelW = 400;
    const int painelH = 360;
    const int painelX = (larguraTela - painelW) / 2;
    const int painelY = (alturaTela - painelH) / 2;

    DesenharPainel(painelX, painelY, painelW, painelH);
    DesenharTituloCentralizado("JOGO PAUSADO", larguraTela, painelY + 30, 26, CIANO_TITULO);
    DrawLine(painelX + 40, painelY + 70, painelX + painelW - 40, painelY + 70, COR_DIVISORIA);

    const float btnW = 280, btnH = 42, espaco = 54;
    float y = (float)(painelY + 95);

    if (DesenharBotao(BotaoCentralizado(larguraTela, y, btnW, btnH), "CONTINUAR [ESC]", true))
        MudarEstado(app, EstadoTela::Jogando);
    if (DesenharBotao(BotaoCentralizado(larguraTela, y + espaco, btnW, btnH), "REINICIAR MISSAO [R]"))
        IniciarMissao(jogo, app);
    if (DesenharBotao(BotaoCentralizado(larguraTela, y + espaco * 2, btnW, btnH), "MENU PRINCIPAL")) {
        app.subMenu = SubMenu::Principal;
        MudarEstado(app, EstadoTela::Menu);
    }
    if (DesenharBotao(BotaoCentralizado(larguraTela, y + espaco * 3, btnW, btnH), "SAIR DO JOGO"))
        app.deveSair = true;
}
