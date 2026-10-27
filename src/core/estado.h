// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/core/estado.h
//
// Descrição:
//   Estado da aplicação (qual tela está ativa), separado do estado da
//   simulação (`JogoEstado`). Toda troca de tela passa por `MudarEstado`,
//   que também decide se o cursor do mouse fica visível.
// ============================================================================

#ifndef ESTADO_H
#define ESTADO_H

#include "core/entidades.h"

// Tela ativa da aplicação (máquina de estados)
enum class EstadoTela {
    Menu,     // Tela inicial
    Jogando,  // Partida em andamento
    Pausado   // Partida congelada com menu de pausa
};

// Subtelas do menu principal
enum class SubMenu {
    Principal,
    Controles,
    SobreCG
};

struct EstadoApp {
    EstadoTela tela = EstadoTela::Menu;
    SubMenu subMenu = SubMenu::Principal;
    bool deveSair   = false;  // Pedido de encerramento (botão "Sair")
};

// Troca de tela. Jogando oculta o cursor; Menu e Pausado o exibem.
void MudarEstado(EstadoApp &app, EstadoTela novaTela);

// Reinicia a simulação do zero e entra em Jogando
void IniciarMissao(JogoEstado &jogo, EstadoApp &app);

#endif // ESTADO_H
