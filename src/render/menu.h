// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/render/menu.h
//
// Descrição:
//   Tela inicial, submenus (Controles / Sobre o projeto) e menu de pausa.
//   Os botões seguem o estilo "modo imediato": desenhar e tratar o clique
//   acontecem na mesma chamada, por isso as funções recebem o estado
//   modificável (`JogoEstado&`, `EstadoApp&`).
// ============================================================================

#ifndef MENU_H
#define MENU_H

#include "core/estado.h"

// Atalhos de teclado dos menus (ENTER inicia, ESC volta/continua, R reinicia na pausa)
void AtualizarMenu(JogoEstado &jogo, EstadoApp &app);

// Desenha o menu principal e seus submenus
void RenderizarMenuPrincipal(JogoEstado &jogo, EstadoApp &app);

// Desenha o menu de pausa sobre a cena congelada
void RenderizarMenuPausa(JogoEstado &jogo, EstadoApp &app);

#endif // MENU_H
