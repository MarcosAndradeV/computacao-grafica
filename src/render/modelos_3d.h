// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/render/modelos_3d.h
//
// Descrição:
//   Declara as rotinas de construção geométrica 3D dos objetos da cena:
//   1. A nave espacial do jogador modelada explicitamente com malha de
//      triângulos (`RL_TRIANGLES`), coordenadas de vértices (`rlVertex3f`) e
//      interpolação de cores por vértice (`rlColor3f` / `rlColor4f`).
//   2. Os alvos espaciais tridimensionais com aplicação da matriz de
//      transformação composta (M = T * Ry * Rx * Rz * S) e sub-transformações
//      hierárquicas na pilha OpenGL/rlgl.
// ============================================================================

#ifndef MODELOS_3D_H
#define MODELOS_3D_H

#include "core/entidades.h"

// Desenha a malha triangular 3D da nave espacial em coordenadas de objeto (Local Space)
void DesenharGeometriaNave3D(float tAnim, bool acelerando, bool turbo);

// Aplica a matriz composta T * R * S e renderiza um alvo 3D procedural
void DesenharAlvo3D(const Alvo3D &alvo, float tAnim, int id);

#endif // MODELOS_3D_H
