// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/render/hud.h
//
// Descrição:
//   Declara a renderização da interface gráfica sobreposta (HUD), incluindo:
//   - Transformação de coordenadas do Espaço do Mundo 3D para a Viewport da tela
//     (`GetWorldToScreen`) para o retículo de mira e caixas holográficas
//   - Teste de visibilidade frontal via Produto Escalar (`Vector3DotProduct`)
//   - Painel didático em tempo real das Matrizes e Ângulos de Euler (CG)
// ============================================================================

#ifndef HUD_H
#define HUD_H

#include "core/entidades.h"

// Renderiza o HUD, retículo projetado, marcadores de alvos e painel didático de CG
void RenderizarHUD(const JogoEstado &jogo);

#endif // HUD_H
