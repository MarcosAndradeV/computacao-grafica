// ============================================================================
// Computação Gráfica - Versão 3D Completa com Raylib
// Demonstração Avançada de Transformações Geométricas 3D:
//   - Matriz de Visualização (Câmeras 3ª Pessoa, 1ª Pessoa e Tática)
//   - Matriz de Projeção Perspectiva 3D (FOV, Frustum)
//   - Pipeline de Transformações de Modelagem: T * R_y * R_x * R_z * S
//   - Geometria Tridimensional com Faces, Cores e Iluminação
//   - Dinâmica de Voo Espacial 3D (Pitch, Yaw, Roll, Aceleração)
//   - Disparos Laser 3D, Colisão Esférica e Sistema de Partículas de Explosão
// ============================================================================

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

// ----------------------------------------------------------------------------
// ESTRUTURAS DO JOGO
// ----------------------------------------------------------------------------

struct Projetil3D {
    Vector3 pos;
    Vector3 dir;
    float vida;
    bool ativo;
};

struct Particula3D {
    Vector3 pos;
    Vector3 vel;
    Vector3 rot;
    Vector3 rotVel;
    float tamanho;
    Color cor;
    float vida;
    float vidaMax;
    bool ativo;
};

struct Alvo3D {
    Vector3 pos;
    Vector3 rotAngulo;
    Vector3 rotVel;
    float escalaBase;
    Color cor;
    int tipo; // 0: Drone Sentinela, 1: Asteroide Cristalino, 2: Prisma de Energia
    bool ativo;
};

struct AnelCheckpoint {
    Vector3 pos;
    float rotY;
    float raio;
    bool coletado;
};

struct Estrela3D {
    Vector3 pos;
    Color cor;
    float tamanho;
    float faseBrilho;
};

struct Nave3D {
    Vector3 pos;
    Vector3 vel;
    float pitch; // Arfagem (eixo X)
    float yaw;   // Guinada (eixo Y)
    float roll;  // Rolagem (eixo Z)
    float rollDesejado;
    float escala;
    float velocidadeAtual;
    float velocidadeMax;
    float aceleracao;
    bool acelerando;
    bool freando;
};

// ----------------------------------------------------------------------------
// VARIÁVEIS GLOBAIS E ESTADO
// ----------------------------------------------------------------------------

static Nave3D nave;
static Camera3D camera;
static enum {
    Chase,
    Cockpit,
    Orbital,
} modoCamera = Chase;
static bool inverterPitch = true;
static bool mostrarHUD_CG = true;
static int pontuacao = 0;
static int ondaAtual = 1;
static float tempoAnimacao = 0.0f;
static float alertaTemporizador = 0.0f;
static float cameraTrauma = 0.0f;
static std::string alertaTexto = "";

static std::vector<Projetil3D> tiros;
static std::vector<Particula3D> particulas;
static std::vector<Alvo3D> alvos;
static std::vector<AnelCheckpoint> aneis;
static std::vector<Estrela3D> estrelas;

// ----------------------------------------------------------------------------
// FUNÇÕES AUXILIARES DE MATEMÁTICA E VETORES
// ----------------------------------------------------------------------------

static inline float AleatorioFloat(float minVal, float maxVal) { return minVal + ((float)rand() / (float)RAND_MAX) * (maxVal - minVal); }

static Vector3 VetorAleatorio3D(float intensidade) {
    float theta = AleatorioFloat(0.0f, 2.0f * PI);
    float phi = AleatorioFloat(-PI * 0.5f, PI * 0.5f);
    return (Vector3){intensidade * cosf(phi) * cosf(theta), intensidade * sinf(phi), intensidade * cosf(phi) * sinf(theta)};
}

// Calcula vetores Forward, Up e Right a partir dos ângulos de Euler (Yaw, Pitch, Roll)
static void CalcularOrientacaoNave(const Nave3D &n, Vector3 &outFwd, Vector3 &outUp, Vector3 &outRight) {
    Matrix matRot =
        MatrixMultiply(MatrixRotateZ(n.roll * DEG2RAD), MatrixMultiply(MatrixRotateX(-n.pitch * DEG2RAD), MatrixRotateY(n.yaw * DEG2RAD)));
    outFwd = Vector3Normalize(Vector3Transform((Vector3){0.0f, 0.0f, 1.0f}, matRot));
    outUp = Vector3Normalize(Vector3Transform((Vector3){0.0f, 1.0f, 0.0f}, matRot));
    outRight = Vector3Normalize(Vector3Transform((Vector3){1.0f, 0.0f, 0.0f}, matRot));
}

// ----------------------------------------------------------------------------
// INICIALIZAÇÃO DE ELEMENTOS DO MUNDO 3D
// ----------------------------------------------------------------------------

static void InicializarEstrelas() {
    estrelas.clear();
    for (int i = 0; i < 500; ++i) {
        float r = AleatorioFloat(80.0f, 180.0f);
        float theta = AleatorioFloat(0.0f, 2.0f * PI);
        float phi = AleatorioFloat(-PI * 0.48f, PI * 0.48f);

        Vector3 p = {r * cosf(phi) * cosf(theta), r * sinf(phi), r * cosf(phi) * sinf(theta)};

        Color c = WHITE;
        int t = rand() % 5;
        if (t == 0)
            c = SKYBLUE;
        else if (t == 1)
            c = (Color){200, 220, 255, 255};
        else if (t == 2)
            c = (Color){255, 230, 180, 255};
        else if (t == 3)
            c = (Color){255, 180, 180, 255};

        estrelas.push_back({p, c, AleatorioFloat(0.4f, 1.2f), AleatorioFloat(0.0f, 6.28f)});
    }
}

static void InicializarAlvos() {
    alvos.clear();
    // Gera alvos distribuídos em um volume tridimensional
    const int qtd = 8 + (ondaAtual - 1) * 2;
    for (int i = 0; i < qtd; ++i) {
        Alvo3D a;
        float angulo = ((float)i / (float)qtd) * 2.0f * PI + AleatorioFloat(-0.2f, 0.2f);
        float dist = AleatorioFloat(18.0f, 42.0f);
        float altura = AleatorioFloat(-8.0f, 14.0f);

        a.pos = (Vector3){cosf(angulo) * dist, altura, sinf(angulo) * dist};

        a.rotAngulo = (Vector3){AleatorioFloat(0.0f, 360.0f), AleatorioFloat(0.0f, 360.0f), AleatorioFloat(0.0f, 360.0f)};
        a.rotVel = (Vector3){AleatorioFloat(-45.0f, 45.0f), AleatorioFloat(40.0f, 90.0f), AleatorioFloat(-30.0f, 30.0f)};
        a.escalaBase = AleatorioFloat(1.2f, 2.0f);
        a.tipo = i % 3;

        if (a.tipo == 0)
            a.cor = (Color){255, 80, 80, 255}; // Drone Sentinela (Vermelho)
        else if (a.tipo == 1)
            a.cor = (Color){60, 255, 160, 255}; // Asteroide Cristalino (Verde Esmeralda)
        else
            a.cor = (Color){255, 215, 0, 255}; // Prisma de Energia (Dourado)

        a.ativo = true;
        alvos.push_back(a);
    }
}

static void InicializarAneis() {
    aneis.clear();
    for (int i = 0; i < 5; ++i) {
        float ang = ((float)i / 5.0f) * 2.0f * PI;
        float dist = 28.0f;
        aneis.push_back({(Vector3){cosf(ang) * dist, AleatorioFloat(2.0f, 8.0f), sinf(ang) * dist}, ang * RAD2DEG + 90.0f, 4.0f, false});
    }
}

static void ReiniciarJogoCompleto() {
    nave.pos = (Vector3){0.0f, 0.0f, -15.0f};
    nave.vel = (Vector3){0.0f, 0.0f, 0.0f};
    nave.pitch = 0.0f;
    nave.yaw = 0.0f;
    nave.roll = 0.0f;
    nave.rollDesejado = 0.0f;
    nave.escala = 1.0f;
    nave.velocidadeAtual = 8.0f;
    nave.velocidadeMax = 26.0f;
    nave.aceleracao = 14.0f;
    nave.acelerando = false;
    nave.freando = false;

    camera.position = (Vector3){0.0f, 6.0f, -28.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    tiros.clear();
    particulas.clear();
    InicializarAlvos();
    InicializarAneis();
    InicializarEstrelas();

    alertaTexto = "COMBATE INICIADO! DESTRUA OS ALVOS 3D";
    alertaTemporizador = 3.5f;
}

// ----------------------------------------------------------------------------
// SISTEMA DE EXPLOSÃO DE PARTÍCULAS 3D
// ----------------------------------------------------------------------------

static void GerarExplosao3D(Vector3 centro, Color corBase, int quantidade) {
    for (int i = 0; i < quantidade; ++i) {
        Particula3D p;
        p.pos = centro;
        p.vel = VetorAleatorio3D(AleatorioFloat(4.0f, 16.0f));
        p.rot = (Vector3){AleatorioFloat(0.0f, 360.0f), AleatorioFloat(0.0f, 360.0f), AleatorioFloat(0.0f, 360.0f)};
        p.rotVel = (Vector3){AleatorioFloat(-200.0f, 200.0f), AleatorioFloat(-200.0f, 200.0f), AleatorioFloat(-200.0f, 200.0f)};
        p.tamanho = AleatorioFloat(0.18f, 0.55f);
        p.cor = (rand() % 3 == 0) ? YELLOW : (rand() % 2 == 0 ? ORANGE : corBase);
        p.vida = AleatorioFloat(0.6f, 1.3f);
        p.vidaMax = p.vida;
        p.ativo = true;
        particulas.push_back(p);
    }
}

// ----------------------------------------------------------------------------
// DESENHO DA GEOMETRIA 3D DA NAVE
// Modelada com primitivas de triângulos, gradientes e transformações locais
// ----------------------------------------------------------------------------

static void DesenharGeometriaNave3D(float tAnim, bool acelerando, bool turbo) {
    // 1. Fuselagem Principal Tridimensional
    rlBegin(RL_TRIANGLES);
    // --- PARTE SUPERIOR (CRISTA E DORSO DA NAVE) ---
    // Face Superior-Esquerda (Azul Ciano Metálico)
    rlColor3f(0.25f, 0.75f, 1.0f);
    rlVertex3f(0.0f, 0.0f, 2.0f); // Nariz pontiagudo
    rlColor3f(0.12f, 0.40f, 0.75f);
    rlVertex3f(-0.55f, 0.08f, -0.6f); // Ombro esquerdo
    rlColor3f(0.35f, 0.85f, 1.0f);
    rlVertex3f(0.0f, 0.45f, 0.1f); // Crista superior

    // Face Superior-Direita (Azul Ciano Metálico)
    rlColor3f(0.25f, 0.75f, 1.0f);
    rlVertex3f(0.0f, 0.0f, 2.0f); // Nariz pontiagudo
    rlColor3f(0.35f, 0.85f, 1.0f);
    rlVertex3f(0.0f, 0.45f, 0.1f); // Crista superior
    rlColor3f(0.12f, 0.40f, 0.75f);
    rlVertex3f(0.55f, 0.08f, -0.6f); // Ombro direito

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
    rlVertex3f(0.35f, 0.15f, -1.3f); // Traseira direita

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
    rlVertex3f(0.0f, -0.28f, 0.0f); // Quilha inferior
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
    rlVertex3f(-0.45f, 0.05f, 0.3f); // Raiz da asa
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

    // --- COCKPIT LUMINOSO (CANÓPIA DE VIDRO CIANO) ---
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

    // 3. Bocais das Turbinas e Chamas de Plasma (Exhaust Flames)
    float pulsoFogo = 0.8f + 0.25f * sinf(tAnim * 28.0f);
    if (acelerando)
        pulsoFogo *= 1.6f;
    if (turbo)
        pulsoFogo *= 2.2f;

    // Turbina Esquerda
    Vector3 bocalEsq = {-0.20f, 0.02f, -1.3f};
    Vector3 fogoEsq = {-0.20f, 0.02f, -1.3f - pulsoFogo};
    DrawCylinderWires(bocalEsq, 0.14f, 0.14f, 0.1f, 8, SKYBLUE);
    DrawCylinderEx(bocalEsq, fogoEsq, 0.12f, 0.01f, 8, (turbo ? SKYBLUE : ORANGE));
    DrawCylinderEx(bocalEsq, (Vector3){bocalEsq.x, bocalEsq.y, -1.3f - pulsoFogo * 0.55f}, 0.07f, 0.01f, 6, (turbo ? WHITE : YELLOW));

    // Turbina Direita
    Vector3 bocalDir = {0.20f, 0.02f, -1.3f};
    Vector3 fogoDir = {0.20f, 0.02f, -1.3f - pulsoFogo};
    DrawCylinderWires(bocalDir, 0.14f, 0.14f, 0.1f, 8, SKYBLUE);
    DrawCylinderEx(bocalDir, fogoDir, 0.12f, 0.01f, 8, (turbo ? SKYBLUE : ORANGE));
    DrawCylinderEx(bocalDir, (Vector3){bocalDir.x, bocalDir.y, -1.3f - pulsoFogo * 0.55f}, 0.07f, 0.01f, 6, (turbo ? WHITE : YELLOW));
}

// ----------------------------------------------------------------------------
// DESENHO DOS ALVOS TRIDIMENSIONAIS (DEMONSTRAÇÃO DE TRANSFORMAÇÕES T * R * S)
// ----------------------------------------------------------------------------

static void DesenharAlvo3D(const Alvo3D &alvo, float tAnim, int id) {
    rlPushMatrix();
    // 1. Translação 3D no Mundo (T)
    rlTranslatef(alvo.pos.x, alvo.pos.y, alvo.pos.z);

    // 2. Rotação Tridimensional Multieixo (R_y * R_x * R_z)
    rlRotatef(alvo.rotAngulo.y, 0.0f, 1.0f, 0.0f);
    rlRotatef(alvo.rotAngulo.x, 1.0f, 0.0f, 0.0f);
    rlRotatef(alvo.rotAngulo.z, 0.0f, 0.0f, 1.0f);

    // 3. Escala com Pulsação Senoidal Harmônica (S)
    float fatorPulso = 1.0f + 0.20f * sinf(tAnim * 3.0f + (float)id * 1.3f);
    float esc = alvo.escalaBase * fatorPulso;
    rlScalef(esc, esc, esc);

    // Renderização segundo o tipo de geometria
    if (alvo.tipo == 0) {
        // Drone Sentinela: Cubo Wireframe + Núcleo Interno + Anéis Orbitais
        DrawCubeWires((Vector3){0, 0, 0}, 1.2f, 1.2f, 1.2f, alvo.cor);
        DrawSphereWires((Vector3){0, 0, 0}, 0.5f, 8, 8, WHITE);

        rlPushMatrix();
        rlRotatef(tAnim * 120.0f, 1.0f, 0.5f, 0.0f);
        DrawCylinderWires((Vector3){0, 0, 0}, 1.5f, 1.5f, 0.04f, 16, alvo.cor);
        rlPopMatrix();

        rlPushMatrix();
        rlRotatef(-tAnim * 95.0f, 0.0f, 0.5f, 1.0f);
        DrawCylinderWires((Vector3){0, 0, 0}, 1.8f, 1.8f, 0.04f, 16, SKYBLUE);
        rlPopMatrix();
    } else if (alvo.tipo == 1) {
        // Asteroide Cristalino: Geometria entrelaçada com rotação composta
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

// ----------------------------------------------------------------------------
// ATUALIZAÇÃO DO JOGO (INPUTS, FÍSICA E COLISÕES)
// ----------------------------------------------------------------------------

static void AtualizarJogo(float dt) {
    tempoAnimacao += dt * 2.0f;

    if (alertaTemporizador > 0.0f) {
        alertaTemporizador -= dt;
    }

    // --- CONTROLES DE VOO DA NAVE ---
    float sensibilidade = 95.0f;
    float fatorInvert = inverterPitch ? -1.0f : 1.0f;

    // Arfagem (Pitch - Eixo X)
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        nave.pitch += sensibilidade * fatorInvert * dt;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        nave.pitch -= sensibilidade * fatorInvert * dt;

    // Limites de arfagem para evitar giro total desorientador
    if (nave.pitch > 85.0f)
        nave.pitch = 85.0f;
    if (nave.pitch < -85.0f)
        nave.pitch = -85.0f;

    // Guinada (Yaw - Eixo Y) com Banking / Auto-Roll Dinâmico
    nave.rollDesejado = 0.0f;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        nave.yaw += sensibilidade * dt;
        nave.rollDesejado = -35.0f;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        nave.yaw -= sensibilidade * dt;
        nave.rollDesejado = 35.0f;
    }

    // Rolagem Manual Opcional (Roll - Eixo Z)
    if (IsKeyDown(KEY_Q))
        nave.rollDesejado += 45.0f;
    if (IsKeyDown(KEY_E))
        nave.rollDesejado -= 45.0f;

    // Interpolação suave do Roll (amortecimento inercial)
    nave.roll = Lerp(nave.roll, nave.rollDesejado, 8.0f * dt);

    // Normaliza Yaw entre 0 e 360
    if (nave.yaw > 360.0f)
        nave.yaw -= 360.0f;
    if (nave.yaw < 0.0f)
        nave.yaw += 360.0f;

    // Aceleração e Turbo
    bool turbo = IsKeyDown(KEY_LEFT_SHIFT);
    nave.acelerando = IsKeyDown(KEY_SPACE) || turbo || IsKeyDown(KEY_W);
    nave.freando = IsKeyDown(KEY_LEFT_CONTROL);

    float velAlvo = 12.0f;
    if (turbo)
        velAlvo = 28.0f;
    else if (nave.freando)
        velAlvo = 4.0f;

    nave.velocidadeAtual = Lerp(nave.velocidadeAtual, velAlvo, 4.0f * dt);

    // Calcula vetores direcionais da nave
    Vector3 forward, up, right;
    CalcularOrientacaoNave(nave, forward, up, right);

    // Atualiza posição da nave
    nave.vel = Vector3Scale(forward, nave.velocidadeAtual);
    nave.pos = Vector3Add(nave.pos, Vector3Scale(nave.vel, dt));

    // Manter nave dentro dos limites do espaço de combate (soft bounce)
    const float RAIO_MUNDO = 65.0f;
    if (Vector3Length(nave.pos) > RAIO_MUNDO) {
        nave.pos = Vector3Scale(Vector3Normalize(nave.pos), RAIO_MUNDO);
    }

    // --- DISPARO DE PROJÉTEIS LASER 3D ---
    if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        // Dispara lasers duplos a partir das pontas dos canhões das asas
        Vector3 canhaoEsq = Vector3Add(nave.pos, Vector3Add(Vector3Scale(forward, 0.4f), Vector3Scale(right, -1.85f)));
        Vector3 canhaoDir = Vector3Add(nave.pos, Vector3Add(Vector3Scale(forward, 0.4f), Vector3Scale(right, 1.85f)));

        tiros.push_back({canhaoEsq, forward, 2.5f, true});
        tiros.push_back({canhaoDir, forward, 2.5f, true});
        cameraTrauma += 0.06f;
    }

    // --- ATUALIZAÇÃO DOS TIROS E DETECÇÃO DE COLISÃO 3D ---
    const float VELOCIDADE_TIRO = 75.0f;
    for (auto &t : tiros) {
        if (!t.ativo)
            continue;

        t.pos = Vector3Add(t.pos, Vector3Scale(t.dir, VELOCIDADE_TIRO * dt));
        t.vida -= dt;
        if (t.vida <= 0.0f) {
            t.ativo = false;
            continue;
        }

        // Teste de Colisão Esférica 3D com Alvos
        for (auto &a : alvos) {
            if (!a.ativo)
                continue;

            float raioColisao = a.escalaBase * 1.15f;
            if (Vector3Distance(t.pos, a.pos) < raioColisao) {
                a.ativo = false;
                t.ativo = false;
                pontuacao += 100;
                cameraTrauma += 0.35f;
                GerarExplosao3D(a.pos, a.cor, 28);
                break;
            }
        }
    }

    // Remove tiros inativos
    tiros.erase(std::remove_if(tiros.begin(), tiros.end(), [](const Projetil3D &t) { return !t.ativo; }), tiros.end());

    // --- ROTAÇÃO CONTÍNUA DOS ALVOS ---
    int alvosVivos = 0;
    for (auto &a : alvos) {
        if (!a.ativo)
            continue;
        alvosVivos++;
        a.rotAngulo.x += a.rotVel.x * dt;
        a.rotAngulo.y += a.rotVel.y * dt;
        a.rotAngulo.z += a.rotVel.z * dt;
    }

    // Próxima onda se todos os alvos forem destruídos
    if (alvosVivos == 0 && alvos.size() > 0) {
        ondaAtual++;
        pontuacao += 500;
        InicializarAlvos();
        alertaTexto = "ONDA " + std::to_string(ondaAtual) + " INICIADA! +500 PTS";
        alertaTemporizador = 3.5f;
    }

    // --- VERIFICAÇÃO DE PASSAGEM POR ANÉIS DE CHECKPOINT ---
    for (auto &anel : aneis) {
        if (anel.coletado)
            continue;
        if (Vector3Distance(nave.pos, anel.pos) < anel.raio * 1.1f) {
            anel.coletado = true;
            pontuacao += 200;
            GerarExplosao3D(anel.pos, SKYBLUE, 20);
            alertaTexto = "ANEL ESPACIAL CRUZADO! +200 PTS";
            alertaTemporizador = 2.0f;
        }
    }

    // --- ATUALIZAÇÃO DO SISTEMA DE PARTÍCULAS 3D ---
    for (auto &p : particulas) {
        if (!p.ativo)
            continue;
        p.pos = Vector3Add(p.pos, Vector3Scale(p.vel, dt));
        p.rot = Vector3Add(p.rot, Vector3Scale(p.rotVel, dt));
        p.vel = Vector3Scale(p.vel, 0.96f); // Resistência inercial
        p.vida -= dt;
        if (p.vida <= 0.0f)
            p.ativo = false;
    }
    particulas.erase(std::remove_if(particulas.begin(), particulas.end(), [](const Particula3D &p) { return !p.ativo; }), particulas.end());

    // --- TROCA DE CÂMERA E ATALHOS ---
    if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_TAB)) {
        switch (modoCamera) {
        case Chase:
            modoCamera = Cockpit;
            break;
        case Cockpit:
            modoCamera = Orbital;
            break;
        case Orbital:
            modoCamera = Chase;
            break;
        }
    }
    if (IsKeyPressed(KEY_I)) {
        inverterPitch = !inverterPitch;
    }
    if (IsKeyPressed(KEY_H)) {
        mostrarHUD_CG = !mostrarHUD_CG;
    }
    if (IsKeyPressed(KEY_R)) {
        ReiniciarJogoCompleto();
    }

    // --- ATUALIZAÇÃO DA CÂMERA 3D (MATRIZ DE VISUALIZAÇÃO) ---
    if (modoCamera == Chase) {
        // Modo 3ª Pessoa (Chase Cam Dinâmica com Amortecimento Inercial)
        Vector3 posDesejada = Vector3Add(nave.pos, Vector3Add(Vector3Scale(forward, -7.5f), Vector3Scale(up, 2.6f)));
        camera.position = Vector3Lerp(camera.position, posDesejada, 12.0f * dt);

        Vector3 targetDesejado = Vector3Add(nave.pos, Vector3Scale(forward, 15.0f));
        camera.target = Vector3Lerp(camera.target, targetDesejado, 14.0f * dt);
        camera.up = Vector3Lerp(camera.up, up, 10.0f * dt);
    } else if (modoCamera == 1) {
        // Modo 1ª Pessoa (Cockpit View)
        camera.position = Vector3Add(nave.pos, Vector3Add(Vector3Scale(forward, 0.3f), Vector3Scale(up, 0.35f)));
        camera.target = Vector3Add(camera.position, Vector3Scale(forward, 30.0f));
        camera.up = up;
    } else {
        // Modo Visão Tática / Orbital
        Vector3 posTatica = Vector3Add(nave.pos, (Vector3){0.0f, 32.0f, -24.0f});
        camera.position = Vector3Lerp(camera.position, posTatica, 8.0f * dt);
        camera.target = nave.pos;
        camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    }

    // Aplicar Screen Shake (Trauma da Câmera)
    if (cameraTrauma > 1.0f)
        cameraTrauma = 1.0f;
    if (cameraTrauma > 0.001f) {
        float shakeX = AleatorioFloat(-1.0f, 1.0f) * cameraTrauma * 0.35f;
        float shakeY = AleatorioFloat(-1.0f, 1.0f) * cameraTrauma * 0.35f;
        camera.position.x += shakeX;
        camera.position.y += shakeY;
        camera.target.x += shakeX;
        camera.target.y += shakeY;
        cameraTrauma -= dt * 1.6f;
        if (cameraTrauma < 0.0f)
            cameraTrauma = 0.0f;
    }
}

// ----------------------------------------------------------------------------
// RENDERIZAÇÃO COMPLETA DO JOGO
// ----------------------------------------------------------------------------

static void RenderizarJogo() {
    BeginDrawing();
    ClearBackground((Color){8, 10, 20, 255}); // Espaço sideral escuro

    // ------------------------------------------------------------------------
    // MUNDO 3D (BeginMode3D ativa Projeção Perspectiva e Matriz ModelView)
    // ------------------------------------------------------------------------
    BeginMode3D(camera);
    rlDisableBackfaceCulling(); // Garante visualização das faces poligonais em qualquer ângulo

    // 1. Campo Estelar 3D
    for (const auto &est : estrelas) {
        float pulso = 0.8f + 0.2f * sinf(tempoAnimacao * 2.0f + est.faseBrilho);
        DrawCube(est.pos, est.tamanho * pulso, est.tamanho * pulso, est.tamanho * pulso, est.cor);
    }

    // 2. Grade de Referência Espacial Holográfica (Plano Y = -16.0f)
    rlPushMatrix();
    rlTranslatef(0.0f, -16.0f, 0.0f);
    DrawGrid(50, 2.5f);
    rlPopMatrix();

    // Limite Esférico Wireframe do Espaço de Combate
    DrawSphereWires((Vector3){0, 0, 0}, 65.0f, 16, 16, (Color){20, 60, 100, 70});

    // 3. Anéis de Checkpoint Espaciais
    for (const auto &anel : aneis) {
        rlPushMatrix();
        rlTranslatef(anel.pos.x, anel.pos.y, anel.pos.z);
        rlRotatef(anel.rotY, 0.0f, 1.0f, 0.0f);
        Color cAnel = anel.coletado ? DARKGRAY : (Color){0, 240, 255, 220};
        DrawCylinderWires((Vector3){0, 0, 0}, anel.raio, anel.raio, 0.35f, 24, cAnel);
        DrawCylinderWires((Vector3){0, 0, 0}, anel.raio * 0.94f, anel.raio * 0.94f, 0.15f, 24, ColorAlpha(cAnel, 0.4f));
        rlPopMatrix();
    }

    // 4. Alvos Tridimensionais com Transformações Compostas
    for (size_t i = 0; i < alvos.size(); ++i) {
        if (alvos[i].ativo) {
            DesenharAlvo3D(alvos[i], tempoAnimacao, (int)i);
        }
    }

    // 5. Projéteis Laser 3D
    for (const auto &t : tiros) {
        if (!t.ativo)
            continue;
        Vector3 fim = Vector3Add(t.pos, Vector3Scale(t.dir, 1.8f));
        DrawCylinderEx(t.pos, fim, 0.08f, 0.08f, 6, YELLOW);
        DrawCylinderEx(t.pos, fim, 0.20f, 0.20f, 6, ColorAlpha(ORANGE, 0.45f));
    }

    // 6. Nave do Jogador (Renderizada apenas se não estiver na visão cockpit direta)
    if (modoCamera != 1) {
        rlPushMatrix();
        // Pipeline T * R_y * R_x * R_z * S
        rlTranslatef(nave.pos.x, nave.pos.y, nave.pos.z);
        rlRotatef(nave.yaw, 0.0f, 1.0f, 0.0f);
        rlRotatef(-nave.pitch, 1.0f, 0.0f, 0.0f);
        rlRotatef(nave.roll, 0.0f, 0.0f, 1.0f);
        rlScalef(nave.escala, nave.escala, nave.escala);

        DesenharGeometriaNave3D(tempoAnimacao, nave.acelerando, IsKeyDown(KEY_LEFT_SHIFT));
        rlPopMatrix();
    }

    // 7. Sistema de Partículas de Explosão 3D
    for (const auto &p : particulas) {
        if (!p.ativo)
            continue;
        rlPushMatrix();
        rlTranslatef(p.pos.x, p.pos.y, p.pos.z);
        rlRotatef(p.rot.x, 1, 0, 0);
        rlRotatef(p.rot.y, 0, 1, 0);
        rlRotatef(p.rot.z, 0, 0, 1);
        float a = p.vida / p.vidaMax;
        Color c = ColorAlpha(p.cor, a);
        DrawCubeWires((Vector3){0, 0, 0}, p.tamanho, p.tamanho, p.tamanho, c);
        rlPopMatrix();
    }
    EndMode3D();

    // ------------------------------------------------------------------------
    // HUD 2D (Interface, Mira Tridimensional, Métricas de CG e Placar)
    // ------------------------------------------------------------------------
    int larguraTela = GetScreenWidth();
    int alturaTela = GetScreenHeight();

    // Retículo de Mira 3D Projetado no Espaço à frente da nave
    Vector3 fwd, up, right;
    CalcularOrientacaoNave(nave, fwd, up, right);
    Vector3 pontoMira3D = Vector3Add(nave.pos, Vector3Scale(fwd, 22.0f));
    Vector2 miraTela = GetWorldToScreen(pontoMira3D, camera);

    if (miraTela.x > 0 && miraTela.x < larguraTela && miraTela.y > 0 && miraTela.y < alturaTela) {
        // Círculo central da mira
        DrawCircleLines((int)miraTela.x, (int)miraTela.y, 14, (Color){0, 240, 255, 200});
        DrawCircleLines((int)miraTela.x, (int)miraTela.y, 4, (Color){255, 255, 255, 230});
        // Cruz da mira
        DrawLine((int)miraTela.x - 24, (int)miraTela.y, (int)miraTela.x - 8, (int)miraTela.y, (Color){0, 240, 255, 180});
        DrawLine((int)miraTela.x + 8, (int)miraTela.y, (int)miraTela.x + 24, (int)miraTela.y, (Color){0, 240, 255, 180});
        DrawLine((int)miraTela.x, (int)miraTela.y - 24, (int)miraTela.x, (int)miraTela.y - 8, (Color){0, 240, 255, 180});
        DrawLine((int)miraTela.x, (int)miraTela.y + 8, (int)miraTela.x, (int)miraTela.y + 24, (Color){0, 240, 255, 180});
    }

    // Marcadores de Alvos Ativos (Brackets 3D na Tela)
    for (const auto &a : alvos) {
        if (!a.ativo)
            continue;
        Vector2 posTela = GetWorldToScreen(a.pos, camera);
        float dist = Vector3Distance(nave.pos, a.pos);

        // Apenas desenha se o alvo estiver na frente da câmera
        Vector3 dirCam = Vector3Subtract(camera.target, camera.position);
        Vector3 dirAlvo = Vector3Subtract(a.pos, camera.position);
        if (Vector3DotProduct(dirCam, dirAlvo) > 0.0f) {
            int bx = (int)posTela.x;
            int by = (int)posTela.y;
            int tamBox = (int)Clamp(400.0f / dist, 12.0f, 50.0f);

            // Bounding Brackets
            DrawLine(bx - tamBox, by - tamBox, bx - tamBox + 6, by - tamBox, a.cor);
            DrawLine(bx - tamBox, by - tamBox, bx - tamBox, by - tamBox + 6, a.cor);
            DrawLine(bx + tamBox, by - tamBox, bx + tamBox - 6, by - tamBox, a.cor);
            DrawLine(bx + tamBox, by - tamBox, bx + tamBox, by - tamBox + 6, a.cor);
            DrawLine(bx - tamBox, by + tamBox, bx - tamBox + 6, by + tamBox, a.cor);
            DrawLine(bx - tamBox, by + tamBox, bx - tamBox, by + tamBox - 6, a.cor);
            DrawLine(bx + tamBox, by + tamBox, bx + tamBox - 6, by + tamBox, a.cor);
            DrawLine(bx + tamBox, by + tamBox, bx + tamBox, by + tamBox - 6, a.cor);

            DrawText(TextFormat("%.0fm", dist), bx - 14, by + tamBox + 4, 11, a.cor);
        }
    }

    // Moldura Superior do HUD
    DrawRectangle(0, 0, larguraTela, 48, (Color){10, 15, 30, 220});
    DrawLine(0, 48, larguraTela, 48, (Color){0, 180, 255, 150});

    DrawText("COMPUTACAO GRAFICA 3D - SPACE COMBAT", 20, 14, 18, (Color){0, 220, 255, 255});
    DrawText(TextFormat("PONTUACAO: %05d", pontuacao), larguraTela - 240, 14, 18, YELLOW);
    DrawText(TextFormat("ONDA: %d", ondaAtual), larguraTela - 360, 14, 18, LIME);

    // Alerta Central Animado
    if (alertaTemporizador > 0.0f) {
        int tamTxt = MeasureText(alertaTexto.c_str(), 20);
        int ax = (larguraTela - tamTxt) / 2;
        DrawRectangle(ax - 20, 60, tamTxt + 40, 36, (Color){0, 20, 40, 210});
        DrawRectangleLines(ax - 20, 60, tamTxt + 40, 36, (Color){0, 240, 255, 230});
        DrawText(alertaTexto.c_str(), ax, 68, 20, (Color){0, 255, 220, 255});
    }

    // Painel Inferior de Estatisticas de Navegacao e Camera
    DrawRectangle(15, alturaTela - 80, 290, 65, (Color){10, 15, 30, 200});
    DrawRectangleLines(15, alturaTela - 80, 290, 65, (Color){0, 180, 255, 120});
    DrawText(TextFormat("VELOCIDADE: %.1f km/s", nave.velocidadeAtual * 10.0f), 25, alturaTela - 72, 13, WHITE);
    DrawText(TextFormat("POSICAO: (X:%.1f, Y:%.1f, Z:%.1f)", nave.pos.x, nave.pos.y, nave.pos.z), 25, alturaTela - 54, 12, LIGHTGRAY);

    const char *strCam = (modoCamera == 0) ? "3a PESSOA (CHASE)" : (modoCamera == 1 ? "1a PESSOA (COCKPIT)" : "TATICA (ORBITAL)");
    DrawText(TextFormat("CAMERA [C]: %s", strCam), 25, alturaTela - 36, 12, (Color){0, 240, 255, 255});

    // Painel Didatico de Computacao Grafica (Toggle com H)
    if (mostrarHUD_CG) {
        int pw = 360;
        int ph = 175;
        int px = larguraTela - pw - 15;
        int py = alturaTela - ph - 15;

        DrawRectangle(px, py, pw, ph, (Color){8, 12, 24, 230});
        DrawRectangleLines(px, py, pw, ph, (Color){0, 200, 255, 180});

        DrawText("TRANSFORMACOES GEOMETRICAS 3D (CG)", px + 15, py + 10, 13, (Color){0, 240, 255, 255});
        DrawText("Pipeline: M = T(x,y,z) * Ry(yaw) * Rx(pitch) * Rz(roll) * S", px + 15, py + 30, 11, YELLOW);
        DrawText(TextFormat("- Yaw (Guinada Y):   %.1f deg", nave.yaw), px + 15, py + 48, 11, LIGHTGRAY);
        DrawText(TextFormat("- Pitch (Arfagem X): %.1f deg (Eixo Y %s [I])", nave.pitch, inverterPitch ? "Invertido" : "Direto"),
                 px + 15,
                 py + 64,
                 11,
                 LIGHTGRAY);
        DrawText(TextFormat("- Roll (Rolagem Z):  %.1f deg (Banking Dinamico)", nave.roll), px + 15, py + 80, 11, LIGHTGRAY);
        DrawText("- Projecao: Perspectiva 3D (FOV 60 deg)", px + 15, py + 96, 11, SKYBLUE);

        DrawText("CONTROLES: W/S/A/D ou Setas = Voo | Q/E = Roll", px + 15, py + 118, 11, RAYWHITE);
        DrawText("ESPACO = Laser | SHIFT = Turbo | C = Camera", px + 15, py + 134, 11, RAYWHITE);
        DrawText("R = Reiniciar | H = Ocultar Painel CG", px + 15, py + 150, 11, (Color){160, 200, 255, 200});
    } else {
        DrawText("Pressione [H] para exibir detalhes de CG e Controles", larguraTela - 380, alturaTela - 25, 12, GRAY);
    }

    EndDrawing();
}

int main() {
    const int largura = 1024;
    const int altura = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(largura, altura, "Space Combat 3D - Transformacoes Geometricas - Raylib");
    SetTargetFPS(60);

    MaximizeWindow();

    ReiniciarJogoCompleto();

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.1f)
            dt = 0.1f;

        AtualizarJogo(dt);
        RenderizarJogo();
    }

    CloseWindow();
    return 0;
}
