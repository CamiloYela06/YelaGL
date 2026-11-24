#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

#include "shader.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

struct MeshGL {
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    GLsizei indexCount = 0;
    unsigned int textureID = 0;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec2 uv;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// =========================================================================
// VARIABLES DE LA CÁMARA (Vista lateral/diagonal para apreciar el lanzamiento)
// =========================================================================
glm::vec3 cameraPos = glm::vec3(0.0f, 1.5f, 7.5f);
glm::vec3 cameraFront = glm::normalize(glm::vec3(0.0f, 0.2f, 0.0f) - cameraPos);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float cameraSpeed = 3.5f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// =========================================================================
// VARIABLES DE LA CURVA DE BÉZIER (TRAYECTORIA DE LANZAMIENTO)
// =========================================================================
// P0: Punto de origen (donde se lanza el hacha)
glm::vec3 P0 = glm::vec3(-4.0f, -0.5f, 1.5f);
// P1: Primer punto de control (impulso inicial hacia arriba y adelante)
glm::vec3 P1 = glm::vec3(-1.5f, 2.2f, 0.5f);
// P2: Segundo punto de control (punto más alto del arco antes de caer)
glm::vec3 P2 = glm::vec3(1.5f, 2.0f, -0.5f);
// P3: Punto de destino (el objetivo donde impacta el hacha)
glm::vec3 P3 = glm::vec3(3.8f, -0.3f, -1.5f);

float bezierT = 0.0f;         // Progreso en la curva: 0.0 (inicio) -> 1.0 (objetivo)
float throwSpeed = 0.75f;     // Velocidad del lanzamiento (menor = más lento)

// Función que calcula la posición exacta 3D en una Curva de Bézier Cúbica para un t en [0, 1]
glm::vec3 calcularBezierCubica(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    glm::vec3 punto = uuu * p0;        // (1 - t)^3 * P0
    punto += 3.0f * uu * t * p1;       // 3(1 - t)^2 * t * P1
    punto += 3.0f * u * tt * p2;       // 3(1 - t) * t^2 * P2
    punto += ttt * p3;                 // t^3 * P3

    return punto;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Tecla R: Reiniciar el lanzamiento del hacha desde P0
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        bezierT = 0.0f;
    }

    glm::vec3 front = glm::normalize(cameraFront);
    glm::vec3 right = glm::normalize(glm::cross(front, cameraUp));
    float velocity = cameraSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cameraPos += front * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        cameraPos -= front * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cameraPos -= right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cameraPos += right * velocity;
}

unsigned int loadTextureFromMemory(const unsigned char* data, size_t size) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* bytes = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &nrChannels, 4);

    if (bytes == nullptr) {
        std::cerr << "Error al decodificar textura del GLB: " << stbi_failure_reason() << std::endl;
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(bytes);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

std::vector<MeshGL> loadGLB(const std::filesystem::path& path) {
    std::vector<MeshGL> meshes;

    fastgltf::Parser parser;
    auto data = fastgltf::GltfDataBuffer::FromPath(path);
    if (data.error() != fastgltf::Error::None) {
        std::cerr << "Error al leer el archivo GLB: " << path << std::endl;
        return meshes;
    }

    auto asset = parser.loadGltfBinary(data.get(), path.parent_path(), fastgltf::Options::None);
    if (asset.error() != fastgltf::Error::None) {
        std::cerr << "Error al parsear el GLB con fastgltf." << std::endl;
        return meshes;
    }

    std::vector<unsigned int> loadedTextures(asset->images.size(), 0);
    for (size_t i = 0; i < asset->images.size(); ++i) {
        const auto& img = asset->images[i];
        if (const auto* view = std::get_if<fastgltf::sources::BufferView>(&img.data)) {
            const auto& bufferView = asset->bufferViews[view->bufferViewIndex];
            const auto& buffer = asset->buffers[bufferView.bufferIndex];
            if (const auto* bytes = std::get_if<fastgltf::sources::Array>(&buffer.data)) {
                const auto* ptr = reinterpret_cast<const unsigned char*>(bytes->bytes.data() + bufferView.byteOffset);
                loadedTextures[i] = loadTextureFromMemory(ptr, bufferView.byteLength);
            }
        }
    }

    for (const auto& mesh : asset->meshes) {
        for (const auto& primitive : mesh.primitives) {
            auto posIt = primitive.findAttribute("POSITION");
            if (posIt == primitive.attributes.end()) continue;

            const auto& posAccessor = asset->accessors[posIt->accessorIndex];
            std::vector<Vertex> vertices(posAccessor.count);

            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
                asset.get(), posAccessor, [&](fastgltf::math::fvec3 pos, size_t idx) {
                    vertices[idx].pos = glm::vec3(pos.x(), pos.y(), pos.z());
                    vertices[idx].uv = glm::vec2(0.0f, 0.0f);
                });

            auto uvIt = primitive.findAttribute("TEXCOORD_0");
            if (uvIt != primitive.attributes.end()) {
                const auto& uvAccessor = asset->accessors[uvIt->accessorIndex];
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(
                    asset.get(), uvAccessor, [&](fastgltf::math::fvec2 uv, size_t idx) {
                        vertices[idx].uv = glm::vec2(uv.x(), uv.y());
                    });
            }

            std::vector<unsigned int> indices;
            if (primitive.indicesAccessor.has_value()) {
                const auto& idxAccessor = asset->accessors[primitive.indicesAccessor.value()];
                indices.resize(idxAccessor.count);
                fastgltf::iterateAccessorWithIndex<std::uint32_t>(
                    asset.get(), idxAccessor, [&](std::uint32_t index, size_t idx) {
                        indices[idx] = index;
                    });
            }

            MeshGL glMesh;
            glMesh.indexCount = static_cast<GLsizei>(indices.size());

            if (primitive.materialIndex.has_value()) {
                const auto& mat = asset->materials[primitive.materialIndex.value()];
                if (mat.pbrData.baseColorTexture.has_value()) {
                    size_t texIdx = mat.pbrData.baseColorTexture->textureIndex;
                    if (asset->textures[texIdx].imageIndex.has_value()) {
                        size_t imgIdx = asset->textures[texIdx].imageIndex.value();
                        glMesh.textureID = loadedTextures[imgIdx];
                    }
                }
            }

            glGenVertexArrays(1, &glMesh.vao);
            glGenBuffers(1, &glMesh.vbo);
            glGenBuffers(1, &glMesh.ebo);

            glBindVertexArray(glMesh.vao);

            glBindBuffer(GL_ARRAY_BUFFER, glMesh.vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh.ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            meshes.push_back(glMesh);
        }
    }

    return meshes;
}

int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << " Proyecto: Lanzamiento Bezier - CAMILO YELA" << std::endl;
    std::cout << " Controles: WASD / Flechas = Camara | R = Relanzar" << std::endl;
    std::cout << "==========================================" << std::endl;

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lanzamiento Bezier - CAMILO YELA", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Shader shader("default.vert", "default.frag");
    shader.use();
    shader.setInt("texture1", 0);
    std::vector hatchetMeshes = loadGLB("psx_hatchet_low-poly.glb");
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);
        // Avanzar el progreso 't' en la curva de Bézier hasta llegar a 1.0 (el objetivo)
        if (bezierT < 1.0f) {
            bezierT += throwSpeed * deltaTime;
            if (bezierT > 1.0f) {
                bezierT = 1.0f; // Se clava/detiene en el punto objetivo P3
            }
        }
        // Calcular la posición 3D actual del hacha sobre la curva
        glm::vec3 posActual = calcularBezierCubica(bezierT, P0, P1, P2, P3);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = width > 0 ? (float)width / (float)height : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp
        );
        // MATRIZ DE MODELO:
        // 1. Trasladamos el hacha al punto calculado en la Curva de Bézier
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, posActual);
        // 2. Orientación fija de lanzamiento (sin girar sobre su eje)
        // Ajusta este ángulo si quieres que el filo apunte hacia la derecha (+X)
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // 3. Escala de 0.9f que configuraste
        model = glm::scale(model, glm::vec3(0.9f));
        glm::mat4 mvp = projection * view * model;
        shader.setMat4("uMVP", mvp);
        for (const auto& mesh : hatchetMeshes) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.textureID);
            glBindVertexArray(mesh.vao);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    for (const auto& mesh : hatchetMeshes) {
        glDeleteVertexArrays(1, &mesh.vao);
        glDeleteBuffers(1, &mesh.vbo);
        glDeleteBuffers(1, &mesh.ebo);
        if (mesh.textureID != 0) {
            glDeleteTextures(1, &mesh.textureID);
        }
    }
    shader.destroy();
    glfwTerminate();
    return 0;
}