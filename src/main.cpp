#include <raylib.h>
#include "camera.h"
#include "lighting.h"
#include "scene.h"
#include "hud.h"

namespace {
    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 720;

    const float TRANSLATE_STEP = 0.15f;
    const float ROTATE_STEP    = 3.0f;
    const float SCALE_STEP     = 0.05f;
    const float CAMERA_ROT_STEP= 2.0f;
    const float CAMERA_ZOOM_STEP= 0.4f;
    const float CAMERA_PAN_STEP = 0.2f;
}

void processInput(GalleryCamera& camera, LightingManager& lighting, Scene& scene, HUD& hud) {
    // 1. Seleção de Objetos e Luz Posicional
    if (IsKeyPressed(KEY_ONE))   scene.selectEntity(1);
    if (IsKeyPressed(KEY_TWO))   scene.selectEntity(2);
    if (IsKeyPressed(KEY_THREE)) scene.selectEntity(3);
    if (IsKeyPressed(KEY_FOUR))  scene.selectEntity(4);
    if (IsKeyPressed(KEY_ZERO))  scene.selectEntity(0);

    // 2. Translações no Objeto ou Luz Selecionada (W/S, A/D, Q/E)
    if (IsKeyDown(KEY_W)) scene.translateSelected(0.0f, 0.0f, -TRANSLATE_STEP, lighting);
    if (IsKeyDown(KEY_S)) scene.translateSelected(0.0f, 0.0f,  TRANSLATE_STEP, lighting);
    if (IsKeyDown(KEY_A)) scene.translateSelected(-TRANSLATE_STEP, 0.0f, 0.0f, lighting);
    if (IsKeyDown(KEY_D)) scene.translateSelected( TRANSLATE_STEP, 0.0f, 0.0f, lighting);
    if (IsKeyDown(KEY_Q)) scene.translateSelected(0.0f, -TRANSLATE_STEP, 0.0f, lighting);
    if (IsKeyDown(KEY_E)) scene.translateSelected(0.0f,  TRANSLATE_STEP, 0.0f, lighting);

    // 3. Rotações no Objeto Selecionado (I/K, J/L, U/O)
    if (IsKeyDown(KEY_I)) scene.rotateSelected(-ROTATE_STEP, 0.0f, 0.0f);
    if (IsKeyDown(KEY_K)) scene.rotateSelected( ROTATE_STEP, 0.0f, 0.0f);
    if (IsKeyDown(KEY_J)) scene.rotateSelected(0.0f, -ROTATE_STEP, 0.0f);
    if (IsKeyDown(KEY_L)) scene.rotateSelected(0.0f,  ROTATE_STEP, 0.0f);
    if (IsKeyDown(KEY_U)) scene.rotateSelected(0.0f, 0.0f, -ROTATE_STEP);
    if (IsKeyDown(KEY_O)) scene.rotateSelected(0.0f, 0.0f,  ROTATE_STEP);

    // 4. Escala (+ / -)
    if (IsKeyDown(KEY_EQUAL) || IsKeyDown(KEY_KP_ADD))      scene.scaleSelected( SCALE_STEP);
    if (IsKeyDown(KEY_MINUS) || IsKeyDown(KEY_KP_SUBTRACT)) scene.scaleSelected(-SCALE_STEP);

    // 5. Câmera: Rotação, Zoom e Pan
    bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (IsKeyDown(KEY_LEFT)) {
        if (shiftDown) camera.pan(-CAMERA_PAN_STEP, 0.0f);
        else camera.rotate(-CAMERA_ROT_STEP, 0.0f);
    }
    if (IsKeyDown(KEY_RIGHT)) {
        if (shiftDown) camera.pan(CAMERA_PAN_STEP, 0.0f);
        else camera.rotate(CAMERA_ROT_STEP, 0.0f);
    }
    if (IsKeyDown(KEY_UP)) {
        if (shiftDown) camera.pan(0.0f, CAMERA_PAN_STEP);
        else camera.rotate(0.0f, CAMERA_ROT_STEP);
    }
    if (IsKeyDown(KEY_DOWN)) {
        if (shiftDown) camera.pan(0.0f, -CAMERA_PAN_STEP);
        else camera.rotate(0.0f, -CAMERA_ROT_STEP);
    }

    if (IsKeyDown(KEY_PAGE_UP))   camera.zoom(-CAMERA_ZOOM_STEP);
    if (IsKeyDown(KEY_PAGE_DOWN)) camera.zoom( CAMERA_ZOOM_STEP);

    // 6. Projeção (P)
    if (IsKeyPressed(KEY_P)) camera.toggleProjection();

    // 7. Iluminação e Toggles (F1 - F5)
    if (IsKeyPressed(KEY_F1)) lighting.toggleDirectional();
    if (IsKeyPressed(KEY_F2)) lighting.togglePositional();
    if (IsKeyPressed(KEY_F3)) lighting.toggleAmbient();
    if (IsKeyPressed(KEY_F4)) lighting.toggleDiffuse();
    if (IsKeyPressed(KEY_F5)) lighting.toggleSpecular();

    // 8. Reset (R)
    if (IsKeyPressed(KEY_R)) {
        camera.reset();
        lighting.reset();
        scene.reset();
    }

    // 9. Ajuda HUD (H)
    if (IsKeyPressed(KEY_H)) hud.toggleHelp();
}

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Galeria de Arte 3D Interativa - Computacao Grafica (Raylib)");
    SetTargetFPS(60);

    GalleryCamera camera;
    LightingManager lighting;
    Scene scene;
    HUD hud;

    // Inicializar iluminação e modelos da cena
    lighting.init();
    scene.init();

    while (!WindowShouldClose()) {
        // Atualizar lógica e controles
        processInput(camera, lighting, scene, hud);
        lighting.update(camera.getPosition());

        int currentW = GetScreenWidth();
        int currentH = GetScreenHeight();

        // Renderização
        BeginDrawing();
        ClearBackground(GetColor(0x18181EFF));

        // Passe 3D
        BeginMode3D(camera.getRaylibCamera());
        scene.render(lighting);
        EndMode3D();

        // Passe 2D (HUD)
        hud.render(currentW, currentH, camera, lighting, scene);

        EndDrawing();
    }

    // Liberação de recursos
    scene.unload();
    lighting.unload();
    CloseWindow();

    return 0;
}
