// ============================================================================
// Computação Gráfica - Space Combat 3D
// Módulo: src/render/modelos_3d.cpp
//
// Descrição:
//   Implementa a modelagem poligonal explícita da nave espacial (malha de
//   triângulos 3D e propulsores de plasma) e as transformações geométricas
//   compostas e hierárquicas dos alvos tridimensionais.
// ============================================================================

#include "render/modelos_3d.h"
#include "rlgl.h"

#include <cmath>

// ----------------------------------------------------------------------------
// MODELAGEM POLIGONAL 3D DA NAVE ESPACIAL (COORDENADAS DE OBJETO)
// Construída explicitamente via malha de triângulos (RL_TRIANGLES) com
// interpolação de cores por vértice (Gouraud Shading das cores nos vértices).
// ----------------------------------------------------------------------------

void DesenharGeometriaNave3D(float tAnim, bool acelerando, bool turbo) {
    // 1. Fuselagem Principal, Asas e Estabilizador Vertical
    rlBegin(RL_TRIANGLES);

    // --- PARTE SUPERIOR (CRISTA E DORSO DA NAVE) ---
    // Face Superior-Esquerda (Azul Ciano Metálico)
    rlColor3f(0.25f, 0.75f, 1.0f);
    rlVertex3f(0.0f, 0.0f, 2.0f);     // Nariz pontiagudo (Eixo Z+)
    rlColor3f(0.12f, 0.40f, 0.75f);
    rlVertex3f(-0.55f, 0.08f, -0.6f); // Ombro esquerdo
    rlColor3f(0.35f, 0.85f, 1.0f);
    rlVertex3f(0.0f, 0.45f, 0.1f);    // Crista superior

    // Face Superior-Direita (Azul Ciano Metálico)
    rlColor3f(0.25f, 0.75f, 1.0f);
    rlVertex3f(0.0f, 0.0f, 2.0f);     // Nariz pontiagudo
    rlColor3f(0.35f, 0.85f, 1.0f);
    rlVertex3f(0.0f, 0.45f, 0.1f);    // Crista superior
    rlColor3f(0.12f, 0.40f, 0.75f);
    rlVertex3f(0.55f, 0.08f, -0.6f);  // Ombro direito

    // Dorso Traseiro Esquerdo
    rlColor3f(0.35f, 0.85f, 1.0f);
    rlVertex3f(0.0f, 0.45f, 0.1f);
    rlColor3f(0.12f, 0.40f, 0.75f);
    rlVertex3f(-0.55f, 0.08f, -0.6f);
    rlColor3f(0.08f, 0.25f, 0.50f);
    rlVertex3f(-0.35f, 0.15f, -1.3f); // Traseira esquerda

    // Dorso Traseiro Central
    rlColor3f(0.35f, 0.85f, 1.0f);
    rlVertex3f(0.0f, 0.45f, 0.1f);
    rlColor3f(0.08f, 0.25f, 0.50f);
    rlVertex3f(-0.35f, 0.15f, -1.3f);
    rlColor3f(0.08f, 0.25f, 0.50f);
    rlVertex3f(0.35f, 0.15f, -1.3f);  // Traseira direita

    // Dorso Traseiro Direito
    rlColor3f(0.35f, 0.85f, 1.0f);
    rlVertex3f(0.0f, 0.45f, 0.1f);
    rlColor3f(0.08f, 0.25f, 0.50f);
    rlVertex3f(0.35f, 0.15f, -1.3f);
    rlColor3f(0.12f, 0.40f, 0.75f);
    rlVertex3f(0.55f, 0.08f, -0.6f);

    // --- PARTE INFERIOR (VENTRE DA NAVE) ---
    // Ventre Esquerdo
    rlColor3f(0.10f, 0.30f, 0.60f);
    rlVertex3f(0.0f, 0.0f, 2.0f);
    rlColor3f(0.05f, 0.15f, 0.35f);
    rlVertex3f(0.0f, -0.28f, 0.0f);   // Quilha inferior
    rlColor3f(0.12f, 0.40f, 0.75f);
    rlVertex3f(-0.55f, 0.08f, -0.6f);

    // Ventre Direito
    rlColor3f(0.10f, 0.30f, 0.60f);
    rlVertex3f(0.0f, 0.0f, 2.0f);
    rlColor3f(0.12f, 0.40f, 0.75f);
    rlVertex3f(0.55f, 0.08f, -0.6f);
    rlColor3f(0.05f, 0.15f, 0.35f);
    rlVertex3f(0.0f, -0.28f, 0.0f);

    // Ventre Traseiro Esquerdo
    rlColor3f(0.05f, 0.15f, 0.35f);
    rlVertex3f(0.0f, -0.28f, 0.0f);
    rlColor3f(0.04f, 0.10f, 0.25f);
    rlVertex3f(-0.35f, -0.10f, -1.3f);
    rlColor3f(0.12f, 0.40f, 0.75f);
    rlVertex3f(-0.55f, 0.08f, -0.6f);

    // Ventre Traseiro Direito
    rlColor3f(0.05f, 0.15f, 0.35f);
    rlVertex3f(0.0f, -0.28f, 0.0f);
    rlColor3f(0.12f, 0.40f, 0.75f);
    rlVertex3f(0.55f, 0.08f, -0.6f);
    rlColor3f(0.04f, 0.10f, 0.25f);
    rlVertex3f(0.35f, -0.10f, -1.3f);

    // Fechamento Traseiro dos Motores
    rlColor3f(0.06f, 0.12f, 0.20f);
    rlVertex3f(-0.35f, 0.15f, -1.3f);
    rlVertex3f(-0.35f, -0.10f, -1.3f);
    rlVertex3f(0.35f, -0.10f, -1.3f);

    rlVertex3f(-0.35f, 0.15f, -1.3f);
    rlVertex3f(0.35f, -0.10f, -1.3f);
    rlVertex3f(0.35f, 0.15f, -1.3f);

    // --- ASAS TRIDIMENSIONAIS (DIEDRO E PONTAS VERMELHAS) ---
    // Asa Esquerda - Face Superior
    rlColor3f(0.18f, 0.50f, 0.90f);
    rlVertex3f(-0.45f, 0.05f, 0.3f);  // Raiz frontal da asa
    rlColor3f(0.95f, 0.20f, 0.20f);
    rlVertex3f(-1.95f, 0.00f, -0.9f); // Ponta da asa
    rlColor3f(0.15f, 0.35f, 0.70f);
    rlVertex3f(-0.45f, 0.05f, -1.0f); // Raiz posterior

    // Asa Esquerda - Face Inferior
    rlColor3f(0.10f, 0.25f, 0.55f);
    rlVertex3f(-0.45f, 0.00f, 0.3f);
    rlColor3f(0.10f, 0.25f, 0.55f);
    rlVertex3f(-0.45f, 0.00f, -1.0f);
    rlColor3f(0.70f, 0.10f, 0.10f);
    rlVertex3f(-1.95f, -0.05f, -0.9f);

    // Asa Direita - Face Superior
    rlColor3f(0.18f, 0.50f, 0.90f);
    rlVertex3f(0.45f, 0.05f, 0.3f);
    rlColor3f(0.15f, 0.35f, 0.70f);
    rlVertex3f(0.45f, 0.05f, -1.0f);
    rlColor3f(0.95f, 0.20f, 0.20f);
    rlVertex3f(1.95f, 0.00f, -0.9f);

    // Asa Direita - Face Inferior
    rlColor3f(0.10f, 0.25f, 0.55f);
    rlVertex3f(0.45f, 0.00f, 0.3f);
    rlColor3f(0.70f, 0.10f, 0.10f);
    rlVertex3f(1.95f, -0.05f, -0.9f);
    rlColor3f(0.10f, 0.25f, 0.55f);
    rlVertex3f(0.45f, 0.00f, -1.0f);

    // --- ESTABILIZADOR VERTICAL (LEME DE CAUDA) ---
    rlColor3f(0.15f, 0.55f, 0.95f);
    rlVertex3f(0.0f, 0.35f, -0.3f);
    rlColor3f(0.90f, 0.20f, 0.20f);
    rlVertex3f(0.0f, 0.95f, -1.3f);
    rlColor3f(0.08f, 0.25f, 0.50f);
    rlVertex3f(0.0f, 0.25f, -1.3f);

    // Leme lado direito (ligeiramente deslocado para espessura)
    rlColor3f(0.20f, 0.65f, 1.0f);
    rlVertex3f(0.02f, 0.35f, -0.3f);
    rlColor3f(0.10f, 0.30f, 0.60f);
    rlVertex3f(0.02f, 0.25f, -1.3f);
    rlColor3f(0.95f, 0.30f, 0.30f);
    rlVertex3f(0.02f, 0.95f, -1.3f);

    // --- COCKPIT LUMINOSO (CANÓPIA DE VIDRO CIANO TRANSLÚCIDO) ---
    rlColor4f(0.40f, 0.95f, 1.0f, 0.9f);
    rlVertex3f(0.0f, 0.15f, 1.3f);
    rlColor4f(0.10f, 0.70f, 0.95f, 0.8f);
    rlVertex3f(-0.22f, 0.28f, 0.2f);
    rlVertex3f(0.0f, 0.46f, 0.2f);

    rlColor4f(0.40f, 0.95f, 1.0f, 0.9f);
    rlVertex3f(0.0f, 0.15f, 1.3f);
    rlColor4f(0.10f, 0.70f, 0.95f, 0.8f);
    rlVertex3f(0.0f, 0.46f, 0.2f);
    rlVertex3f(0.22f, 0.28f, 0.2f);
    rlEnd();

    // 2. Canhões Laser nas Pontas das Asas
    DrawCylinderEx((Vector3){-1.85f, -0.02f, -0.4f}, (Vector3){-1.85f, -0.02f, 0.5f}, 0.05f, 0.05f, 6, DARKGRAY);
    DrawSphere((Vector3){-1.85f, -0.02f, 0.52f}, 0.08f, RED);
    DrawCylinderEx((Vector3){1.85f, -0.02f, -0.4f}, (Vector3){1.85f, -0.02f, 0.5f}, 0.05f, 0.05f, 6, DARKGRAY);
    DrawSphere((Vector3){1.85f, -0.02f, 0.52f}, 0.08f, RED);

    // 3. Bocais das Turbinas e Chamas de Plasma (Escala Dinâmica Senoidal)
    float pulsoFogo = 0.8f + 0.25f * std::sin(tAnim * 28.0f);
    if (acelerando)
        pulsoFogo *= 1.6f;
    if (turbo)
        pulsoFogo *= 2.2f;

    // Turbina Esquerda
    Vector3 bocalEsq = {-0.20f, 0.02f, -1.3f};
    Vector3 fogoEsq  = {-0.20f, 0.02f, -1.3f - pulsoFogo};
    DrawCylinderWires(bocalEsq, 0.14f, 0.14f, 0.1f, 8, SKYBLUE);
    DrawCylinderEx(bocalEsq, fogoEsq, 0.12f, 0.01f, 8, (turbo ? SKYBLUE : ORANGE));
    DrawCylinderEx(bocalEsq, (Vector3){bocalEsq.x, bocalEsq.y, -1.3f - pulsoFogo * 0.55f}, 0.07f, 0.01f, 6, (turbo ? WHITE : YELLOW));

    // Turbina Direita
    Vector3 bocalDir = {0.20f, 0.02f, -1.3f};
    Vector3 fogoDir  = {0.20f, 0.02f, -1.3f - pulsoFogo};
    DrawCylinderWires(bocalDir, 0.14f, 0.14f, 0.1f, 8, SKYBLUE);
    DrawCylinderEx(bocalDir, fogoDir, 0.12f, 0.01f, 8, (turbo ? SKYBLUE : ORANGE));
    DrawCylinderEx(bocalDir, (Vector3){bocalDir.x, bocalDir.y, -1.3f - pulsoFogo * 0.55f}, 0.07f, 0.01f, 6, (turbo ? WHITE : YELLOW));
}

// ----------------------------------------------------------------------------
// TRANSFORMAÇÕES GEOMÉTRICAS COMPOSTAS DOS ALVOS 3D (M = T * Ry * Rx * Rz * S)
// ----------------------------------------------------------------------------

void DesenharAlvo3D(const Alvo3D &alvo, float tAnim, int id) {
    rlPushMatrix();

    // 1. Matriz de Translação 3D no Mundo: T(x, y, z)
    rlTranslatef(alvo.pos.x, alvo.pos.y, alvo.pos.z);

    // 2. Matrizes de Rotação de Euler em 3 Eixos: R_y * R_x * R_z
    rlRotatef(alvo.rotAngulo.y, 0.0f, 1.0f, 0.0f);
    rlRotatef(alvo.rotAngulo.x, 1.0f, 0.0f, 0.0f);
    rlRotatef(alvo.rotAngulo.z, 0.0f, 0.0f, 1.0f);

    // 3. Matriz de Escala Uniforme com Pulsação Harmônica: S(t)
    float fatorPulso = 1.0f + 0.20f * std::sin(tAnim * 3.0f + (float)id * 1.3f);
    float esc        = alvo.escalaBase * fatorPulso;
    rlScalef(esc, esc, esc);

    // Renderização hierárquica segundo o tipo do alvo
    if (alvo.tipo == TipoAlvo::DroneSentinela) {
        // Drone Sentinela: Cubo Wireframe + Núcleo Esférico + 2 Anéis Orbitais Hierárquicos
        DrawCubeWires((Vector3){0, 0, 0}, 1.2f, 1.2f, 1.2f, alvo.cor);
        DrawSphereWires((Vector3){0, 0, 0}, 0.5f, 8, 8, WHITE);

        // Sub-transformação hierárquica do Anel Orbital 1
        rlPushMatrix();
        rlRotatef(tAnim * 120.0f, 1.0f, 0.5f, 0.0f);
        DrawCylinderWires((Vector3){0, 0, 0}, 1.5f, 1.5f, 0.04f, 16, alvo.cor);
        rlPopMatrix();

        // Sub-transformação hierárquica do Anel Orbital 2
        rlPushMatrix();
        rlRotatef(-tAnim * 95.0f, 0.0f, 0.5f, 1.0f);
        DrawCylinderWires((Vector3){0, 0, 0}, 1.8f, 1.8f, 0.04f, 16, SKYBLUE);
        rlPopMatrix();
    } else if (alvo.tipo == TipoAlvo::AsteroideCristalino) {
        // Asteroide Cristalino: Cubos entrelaçados com rotação relativa de 45°
        DrawCubeWires((Vector3){0, 0, 0}, 1.4f, 1.4f, 1.4f, alvo.cor);
        rlPushMatrix();
        rlRotatef(45.0f, 1.0f, 0.0f, 1.0f);
        DrawCubeWires((Vector3){0, 0, 0}, 1.4f, 1.4f, 1.4f, ColorAlpha(alvo.cor, 0.65f));
        rlPopMatrix();
        DrawSphereWires((Vector3){0, 0, 0}, 0.7f, 6, 6, LIME);
    } else {
        // Prisma de Energia: Octaedro / Pirâmides Duplas
        DrawCubeWires((Vector3){0, 0, 0}, 1.0f, 1.0f, 1.0f, alvo.cor);
        rlPushMatrix();
        rlRotatef(tAnim * 85.0f, 0.0f, 1.0f, 0.0f);
        DrawCylinderWires((Vector3){0, 0, 0}, 0.0f, 0.7f, 1.6f, 6, YELLOW);
        DrawCylinderWires((Vector3){0, 0, 0}, 0.7f, 0.0f, 1.6f, 6, ORANGE);
        rlPopMatrix();
    }

    rlPopMatrix();
}
