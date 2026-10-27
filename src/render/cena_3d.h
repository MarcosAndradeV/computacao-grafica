// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/render/cena_3d.h
//
// Descrição:
//   Declara o passe de renderização tridimensional da cena (`RenderizarMundo3D`),
//   responsável por ativar a Matriz de Projeção Perspectiva e a Matriz de
//   Visualização (`BeginMode3D`) e posicionar todas as entidades do espaço 3D
//   (estrelas, grade de referência, anéis, alvos, projéteis, nave e partículas)
//   utilizando a pilha de matrizes OpenGL/rlgl.
// ============================================================================

#ifndef CENA_3D_H
#define CENA_3D_H

#include "core/entidades.h"

// Executa o passe de renderização 3D completo, vendo a cena pela câmera do jogo
// (já com o tremor do quadro, via `ObterCameraRender`)
void RenderizarMundo3D(const JogoEstado &jogo);

#endif // CENA_3D_H
