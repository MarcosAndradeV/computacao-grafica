#include "lighting.h"
#include <raymath.h>

const char *VS_CODE = R"(#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main() {
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

const char *FS_CODE = R"(#version 330
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform vec4 colDiffuse;

uniform int uDirLightEnabled;
uniform int uPosLightEnabled;
uniform int uAmbientEnabled;
uniform int uDiffuseEnabled;
uniform int uSpecularEnabled;

uniform vec3 uDirLightDir;
uniform vec3 uPosLightPos;
uniform vec3 uViewPos;

uniform vec4 uMaterialAmbient;
uniform vec4 uMaterialSpecular;
uniform float uMaterialShininess;

out vec4 finalColor;

void main() {
    vec4 baseColor = colDiffuse * fragColor;
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(uViewPos - fragPosition);

    vec3 ambientAccum = vec3(0.0);
    vec3 diffuseAccum = vec3(0.0);
    vec3 specularAccum = vec3(0.0);

    if (uAmbientEnabled == 1) {
        ambientAccum += uMaterialAmbient.rgb * 0.4 + vec3(0.15);
    }

    // 1. Luz Direcional Distante
    if (uDirLightEnabled == 1) {
        vec3 L = normalize(uDirLightDir);
        float NdotL = max(dot(N, L), 0.0);

        if (uDiffuseEnabled == 1) {
            diffuseAccum += vec3(0.85, 0.85, 0.80) * NdotL;
        }

        if (uSpecularEnabled == 1 && NdotL > 0.0) {
            vec3 H = normalize(L + V);
            float NdotH = max(dot(N, H), 0.0);
            float spec = pow(NdotH, max(uMaterialShininess, 1.0));
            specularAccum += uMaterialSpecular.rgb * spec * 0.8;
        }
    }

    // 2. Luz Posicional Próxima com atenuação
    if (uPosLightEnabled == 1) {
        vec3 lightVec = uPosLightPos - fragPosition;
        float dist = length(lightVec);
        vec3 L = normalize(lightVec);
        float att = 1.0 / (1.0 + 0.05 * dist + 0.01 * dist * dist);
        float NdotL = max(dot(N, L), 0.0);

        if (uDiffuseEnabled == 1) {
            diffuseAccum += vec3(1.0, 0.95, 0.75) * NdotL * att;
        }

        if (uSpecularEnabled == 1 && NdotL > 0.0) {
            vec3 H = normalize(L + V);
            float NdotH = max(dot(N, H), 0.0);
            float spec = pow(NdotH, max(uMaterialShininess, 1.0));
            specularAccum += uMaterialSpecular.rgb * spec * att;
        }
    }

    vec3 lightTotal = ambientAccum + diffuseAccum;
    vec3 resultRgb = baseColor.rgb * lightTotal + specularAccum;
    finalColor = vec4(clamp(resultRgb, 0.0, 1.0), baseColor.a);
}
)";

LightingManager::LightingManager() { reset(); }

void LightingManager::reset() {
    directionalEnabled = true;
    positionalEnabled = true;
    ambientEnabled = true;
    diffuseEnabled = true;
    specularEnabled = true;

    positionalPos = defaultPositionalPos;
    directionalDir = defaultDirectionalDir;
}

void LightingManager::init() {
    shader = LoadShaderFromMemory(VS_CODE, FS_CODE);

    // Obter locais dos uniformes
    uDirLightEnabledLoc = GetShaderLocation(shader, "uDirLightEnabled");
    uPosLightEnabledLoc = GetShaderLocation(shader, "uPosLightEnabled");
    uAmbientEnabledLoc = GetShaderLocation(shader, "uAmbientEnabled");
    uDiffuseEnabledLoc = GetShaderLocation(shader, "uDiffuseEnabled");
    uSpecularEnabledLoc = GetShaderLocation(shader, "uSpecularEnabled");
    uDirLightDirLoc = GetShaderLocation(shader, "uDirLightDir");
    uPosLightPosLoc = GetShaderLocation(shader, "uPosLightPos");
    uViewPosLoc = GetShaderLocation(shader, "uViewPos");
    uMaterialAmbientLoc = GetShaderLocation(shader, "uMaterialAmbient");
    uMaterialSpecularLoc = GetShaderLocation(shader, "uMaterialSpecular");
    uMaterialShininessLoc = GetShaderLocation(shader, "uMaterialShininess");

    shader.locs[SHADER_LOC_VECTOR_VIEW] = uViewPosLoc;
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
}

void LightingManager::unload() { UnloadShader(shader); }

void LightingManager::toggleDirectional() { directionalEnabled = !directionalEnabled; }
void LightingManager::togglePositional() { positionalEnabled = !positionalEnabled; }
void LightingManager::toggleAmbient() { ambientEnabled = !ambientEnabled; }
void LightingManager::toggleDiffuse() { diffuseEnabled = !diffuseEnabled; }
void LightingManager::toggleSpecular() { specularEnabled = !specularEnabled; }

void LightingManager::translatePositionalLight(float dx, float dy, float dz) {
    positionalPos.x += dx;
    positionalPos.y += dy;
    positionalPos.z += dz;

    if (positionalPos.y < 0.3f)
        positionalPos.y = 0.3f;
    if (positionalPos.y > 6.0f)
        positionalPos.y = 6.0f;
}

void LightingManager::update(Vector3 cameraPos) {
    int dirVal = directionalEnabled ? 1 : 0;
    int posVal = positionalEnabled ? 1 : 0;
    int ambVal = ambientEnabled ? 1 : 0;
    int difVal = diffuseEnabled ? 1 : 0;
    int speVal = specularEnabled ? 1 : 0;

    SetShaderValue(shader, uDirLightEnabledLoc, &dirVal, SHADER_UNIFORM_INT);
    SetShaderValue(shader, uPosLightEnabledLoc, &posVal, SHADER_UNIFORM_INT);
    SetShaderValue(shader, uAmbientEnabledLoc, &ambVal, SHADER_UNIFORM_INT);
    SetShaderValue(shader, uDiffuseEnabledLoc, &difVal, SHADER_UNIFORM_INT);
    SetShaderValue(shader, uSpecularEnabledLoc, &speVal, SHADER_UNIFORM_INT);

    SetShaderValue(shader, uDirLightDirLoc, &directionalDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, uPosLightPosLoc, &positionalPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, uViewPosLoc, &cameraPos, SHADER_UNIFORM_VEC3);
}

void LightingManager::applyMaterial(const MaterialData &mat) {
    SetShaderValue(shader, uMaterialAmbientLoc, &mat.ambient, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, uMaterialSpecularLoc, &mat.specular, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, uMaterialShininessLoc, &mat.shininess, SHADER_UNIFORM_FLOAT);
}

void LightingManager::renderLightMarker(bool isSelected) {
    if (!positionalEnabled)
        return;

    // Lâmpada indicadora amarela brilhante
    DrawSphere(positionalPos, 0.22f, YELLOW);

    if (isSelected) {
        // Indicador de seleção da luz
        DrawCircle3D(positionalPos, 0.40f, {0, 1, 0}, 90.0f, GOLD);
        DrawCircle3D(positionalPos, 0.40f, {1, 0, 0}, 90.0f, GOLD);
    }
}
