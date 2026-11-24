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

// Tamaño de la ventana
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

// Variables de la cámara
glm::vec3 cameraPos = glm::vec3(1.5f, 0.8f, 3.0f);
glm::vec3 cameraFront = glm::normalize(
    glm::vec3(0.0f, 0.0f, 0.0f) - cameraPos
);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float cameraSpeed = 2.5f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Procesar movimiento de la cámara (WASD y flechas)
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3 front = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
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

// Cargar textura incrustada en el .glb con estilo PSX (GL_NEAREST)
unsigned int loadTextureFromMemory(const unsigned char* data, size_t size) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* bytes = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &nrChannels, 4);

    if (bytes == nullptr) {
        std::cerr << "Error al cargar textura del GLB: " << stbi_failure_reason() << std::endl;
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Estilo retro PSX (sin suavizado)
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

// Cargar el modelo .glb con fastgltf
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

    // Extraer texturas incrustadas
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

    // Extraer geometría e índices
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

            // location = 0 -> aPos
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
            glEnableVertexAttribArray(0);

            // location = 1 -> aTexCoord
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
    std::cout << " Proyecto: Modelo GLB_ CAMILO YELA" << std::endl;
    std::cout << "==========================================" << std::endl;

    if (!glfwInit()) {
        std::cerr << "Error al inicializar GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Modelo GLB_ CAMILO YELA", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Error al crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Error al inicializar GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 3. Cargar Shaders desde la clase Shader
    Shader shader("default.vert", "default.frag");
    shader.use();
    shader.setInt("texture1", 0);

    // 4. Cargar el hacha PSX (.glb)
    std::vector<MeshGL> hatchetMeshes = loadGLB("psx_hatchet_low-poly.glb");
    if (hatchetMeshes.empty()) {
        std::cerr << "No se cargaron mallas desde psx_hatchet_low-poly.glb" << std::endl;
    }

    // 5. RENDER LOOP
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        // Matriz de Proyección (mismo FOV de 60° y aspect dinámico que tenías)
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = width > 0 ? (float)width / (float)height : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
        // Matriz de Vista: cámara móvil
        glm::mat4 view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp
        );
        // Matriz de Modelo (Rotación suave igual a tu código)
        float t = (float)glfwGetTime();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, t * 0.2f, glm::vec3(0.0f, 1.0f, 0.0f));
        // Si el hacha sale muy grande o pequeña, descomenta la siguiente línea:
        // model = glm::scale(model, glm::vec3(1.0f));
        glm::mat4 mvp = projection * view * model;
        shader.setMat4("uMVP", mvp);
        // Dibujar cada malla del .glb con su textura
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
    // Liberar recursos
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

