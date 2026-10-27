// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/core/estado.cpp
// ============================================================================

#include "core/estado.h"

void MudarEstado(EstadoApp &app, EstadoTela novaTela) {
    app.tela = novaTela;

    if (novaTela == EstadoTela::Jogando) {
        DisableCursor();  // Cursor oculto e travado durante a partida
    } else {
        EnableCursor();   // Menus precisam do cursor para os botões
    }
}

void IniciarMissao(JogoEstado &jogo, EstadoApp &app) {
    ReiniciarJogoCompleto(jogo);
    MudarEstado(app, EstadoTela::Jogando);
}
