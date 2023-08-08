//"Please refer to the licenses for GLFW, GLEW, GLM, CL, ASSIMP, and stb_image."

/* USG OPENGL ENGINE 1.1.0*/
/* USG (c) July 16th, 2023.
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/


#define GLFW_INCLUDE_NONE
#include <iostream>
#include <fstream>
#include <filesystem>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS 
#include <CL/cl.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ULOADER.h"
#include "UECS.h"


#include <string>
#include <stdlib.h>
#include <vector>
#include <sstream>

#include <assert.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct ViewMatrix
{
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
};

struct Camera
{
    glm::vec3 cameraPos     = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 cameraTarget  = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp      = glm::vec3(0.0f, 1.0f, 0.0f);
    float fov               = 45.0f;
    float aspect            = 1.3333f;
    float nearPlane         = 0.001f;
    float farPlane          = 100.0f;
};

struct OBJECT
{
    glm::vec3 position    = { 0.0f, 0.0f, 0.0f};
    glm::vec3 angle       = { 0.0f, 0.0f, 0.0f };
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec3 scale       = { 1.0f, 1.0f, 1.0f };
    glm::vec3 color       = { 1.0f, 0.0f, 1.0f };

    OBJECT(void) = default;
    OBJECT(glm::vec3 posiiton, glm::vec3 angle, glm::vec3 scale) 
    : position(posiiton), angle(angle), scale(scale) {}
};
// BOX COLLISION
struct COLLISION
{
    std::vector<glm::vec4> points = {
        {0.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f}
    };
};

struct OBJEC_PATH
{
    std::string _fbxPath;
    std::string _shaderVertexPath;
    std::string _shaderPragmentPath;
};

struct MODEL {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    std::vector<GLuint> texture;
    size_t _numVertices;
    size_t _numTextCoord;
    GLuint _vertex_shader;
    GLuint _fragment_shader;
    GLuint _shader_program;
};

namespace {
    std::string ReadShaderFile(const char* shaderPath)
    {
        std::string content;
        std::ifstream fileStream(shaderPath, std::ios::in);

        if (!fileStream.is_open()) {
            std::cerr << "Could not read file " << shaderPath << ". File does not exist." << std::endl;
            return "";
        }

        std::stringstream ss;
        ss << fileStream.rdbuf();
        content = ss.str();

        fileStream.close();
        return content;
    }
    void SetShader(MODEL* model, const char* vertexShaderPath, const char* fragmentShaderPath) {
        std::string vertexCode = ReadShaderFile(vertexShaderPath);
        std::string fragmentCode = ReadShaderFile(fragmentShaderPath);

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char* vertexShaderSource = vertexCode.c_str();
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fragmentShaderSource = fragmentCode.c_str();
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        model->_shader_program = glCreateProgram();
        glAttachShader(model->_shader_program, vertexShader);
        glAttachShader(model->_shader_program, fragmentShader);
        glLinkProgram(model->_shader_program);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    bool BindTexture(const int i, const aiScene* scene, std::vector<GLuint>& texture)
    {
        aiMaterial* material = scene->mMaterials[i];
        aiString path;

        if (material->GetTextureCount(aiTextureType_DIFFUSE) <= 0) return true;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) != AI_SUCCESS) return true;

        int width, height, numChannels;
        std::string folderPath = "resource/3d/";
        std::string fullPath = folderPath + path.C_Str();
        unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &numChannels, 0);
        if (!data) {
            std::cerr << fullPath << " is not found!\n";
            return true;
        }
        // Generate texture
        glGenTextures(1, &texture[i]);
        glBindTexture(GL_TEXTURE_2D, texture[i]);

        // Upload texture to GPU
        if (numChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else if (numChannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        else {
            std::cerr << "Unsupported number of channels: " << numChannels << "\n";
            return true;
        }

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        return false;
    }
    void GetVertices(const int j, const aiMesh* mesh, std::vector<float>& vertices)
    {
        // Positions
        if (mesh->HasPositions())
        {
            vertices.push_back(mesh->mVertices[j].x);
            vertices.push_back(mesh->mVertices[j].y);
            vertices.push_back(mesh->mVertices[j].z);
        }
        else
        {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
        // Normals
        if (mesh->HasNormals())
        {        
            vertices.push_back(mesh->mNormals[j].x);
            vertices.push_back(mesh->mNormals[j].y);
            vertices.push_back(mesh->mNormals[j].z);
        }
        else
        {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }

        if (mesh->HasTextureCoords(0))
        {
            vertices.push_back(mesh->mTextureCoords[0][j].x);
            vertices.push_back(mesh->mTextureCoords[0][j].y);
        }
        else
        {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }
    size_t SetVertexSize(const std::vector<float>& vertices)
    {
        return vertices.size() / 8;  // 8 components per vertex
    }
    void BindVertices(const std::vector<float>& vertices, GLuint& VAO, GLuint& VBO)
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // Texture coordinate attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }
    void SetModel(MODEL* model, const char* modelPath) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Failed to load " << modelPath << " file\n";
            return;
        }
        std::vector<float> vertices;
        model->texture.resize(scene->mNumMaterials, 0); 
        for (unsigned int i = 0; i < scene->mNumMaterials; ++i){
            if (BindTexture(i, scene, model->texture))
            {
                model->texture.resize(0);
                break;
            }
        }
        for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[i];
            for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
                GetVertices(j, mesh, vertices);
        }
        BindVertices(vertices, model->VAO, model->VBO);
        model->_numVertices  = SetVertexSize(vertices);
    }

    static void MoveForward(Camera& camera, const float distance) {
        glm::vec3 direction = glm::normalize(camera.cameraTarget - camera.cameraPos);
        camera.cameraPos += distance * direction;
        camera.cameraTarget += distance * direction;
    }
    static void MoveHorizontal(Camera& camera, const float distance) {
        glm::vec3 right = glm::normalize(glm::cross(camera.cameraUp, camera.cameraTarget - camera.cameraPos));
        camera.cameraPos += distance * right;
        camera.cameraTarget += distance * right;
    }
    static void MoveVertical(Camera& camera, const float distance) {
        camera.cameraPos += distance * camera.cameraUp;
        camera.cameraTarget += distance * camera.cameraUp;
    }
    static void Rotate(Camera& camera, const  float yaw, const  float pitch) {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        rotation = glm::rotate(rotation, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::vec4 newDirection = rotation * glm::vec4(camera.cameraTarget - camera.cameraPos, 0.0f);
        camera.cameraTarget = camera.cameraPos + glm::vec3(newDirection);
    }
    static void Revolve(Camera& camera, const float yaw, const float pitch) {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), camera.cameraUp);
        rotation = glm::rotate(rotation, glm::radians(pitch), glm::cross(camera.cameraUp, camera.cameraTarget - camera.cameraPos));

        glm::vec4 newDirection = rotation * glm::vec4(camera.cameraPos - camera.cameraTarget, 0.0f);
        camera.cameraPos = camera.cameraTarget + glm::vec3(newDirection);
    }

    static double scrollY;
}

namespace OPENGLSYSTEM
{
    inline void READING(const char* modelPath, const char* vertexShaderPath, const char* fragmentShaderPath, MODEL* model)
    {
        SetModel(model, modelPath);
        SetShader(model, vertexShaderPath, fragmentShaderPath);
    }
    inline void KeyControl(const float speed,Camera* camera,GLFWwindow* window)
    {
        // Camera movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            MoveForward(*camera, speed);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            MoveForward(*camera, -speed);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            MoveHorizontal(*camera, speed);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            MoveHorizontal(*camera, -speed);
        }
    }
    inline void MouseControl(const float speed, bool& pressMiddle, bool& pressRight, double& scrollY, double& lastX, double& lastY,
                            double& initialX, double& initialY,Camera* camera,GLFWwindow* window)
    {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
            if (!pressMiddle) {
                // Middle button just pressed, initialize the initial cursor position
                glfwGetCursorPos(window, &initialX, &initialY);
                lastX = initialX;
                lastY = initialY;
                pressMiddle = true;
            }
            else {
                double mouseX, mouseY;
                glfwGetCursorPos(window, &mouseX, &mouseY);
                double xOffset = lastX - mouseX;
                double yOffset = lastY - mouseY;
                lastX = mouseX;
                lastY = mouseY;
                MoveVertical(*camera, -speed * yOffset);
                MoveHorizontal(*camera, -speed * xOffset);
            }
        }
        else {
            pressMiddle = false;
        }

        if (scrollY > 0) {
            MoveForward(*camera, -speed);
        }
        else if (scrollY < 0) {
            MoveForward(*camera, speed);
        }
        scrollY = 0;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            if (!pressRight) {
                // Right button just pressed, initialize the initial cursor position
                glfwGetCursorPos(window, &initialX, &initialY);
                lastX = initialX;
                lastY = initialY;
                pressRight = true;
            }
            else {
                double mouseX, mouseY;
                glfwGetCursorPos(window, &mouseX, &mouseY);
                double xOffset = mouseX - lastX;
                double yOffset = lastY - mouseY;
                lastX = mouseX;
                lastY = mouseY;
                Rotate(*camera, speed * xOffset, speed * yOffset);
            }
        }
        else {
            pressRight = false;
        }
    }
}

#define PROJECT(A, B) (glm::dot(B, A) / glm::length(B)) * B;

inline bool CheckOBB(COLLISION& A, COLLISION& B)
{
    glm::vec3 a1 = A.points[0];
    glm::vec3 a2 = A.points[1];
    glm::vec3 a3 = A.points[2];
    glm::vec3 a4 = A.points[3];
    glm::vec3 a5 = A.points[4];
    glm::vec3 a6 = A.points[5];
    glm::vec3 a7 = A.points[6];
    glm::vec3 a8 = A.points[7];

    glm::vec3 b1 = B.points[0];
    glm::vec3 b2 = B.points[1];
    glm::vec3 b3 = B.points[2];
    glm::vec3 b4 = B.points[3];
    glm::vec3 b5 = B.points[4];
    glm::vec3 b6 = B.points[5];
    glm::vec3 b7 = B.points[6];
    glm::vec3 b8 = B.points[7];

    // save all points
    std::vector<std::pair<glm::vec3, bool>> points
    ({ 
        {a1, 0},{b1, 1},
        {a2, 0},{b2, 1},
        {a3, 0},{b3, 1},
        {a4, 0},{b4, 1},
        {a5, 0},{b5, 1},
        {a6, 0},{b6, 1},
        {a7, 0},{b7, 1},
        {a8, 0},{b8, 1}
    });
    // Set axis unit vectors
    // A
    auto v1 = glm::normalize(a4 - a1);
    auto v2 = glm::normalize(a5 - a2);
    auto v3 = glm::normalize(a6 - a3);
    // B
    auto v4 = glm::normalize(b4 - b1);
    auto v5 = glm::normalize(b5 - b2);
    auto v6 = glm::normalize(b6 - b3);
    // Set face normal vector
    // A
    auto n1 = glm::cross(v1, v2);
    auto n2 = glm::cross(v1, v3);
    auto n3 = glm::cross(v2, v3);
    // B
    auto n4 = glm::cross(v4, v5);
    auto n5 = glm::cross(v4, v6);
    auto n6 = glm::cross(v5, v6);
    // C
    auto n7 = glm::cross(v1, v4);
    auto n8 = glm::cross(v1, v5);
    auto n9 = glm::cross(v1, v6);
    // D
    auto n7 = glm::cross(v2, v4);
    auto n8 = glm::cross(v2, v5);
    auto n9 = glm::cross(v2, v6);
    // E
    auto n7 = glm::cross(v3, v4);
    auto n8 = glm::cross(v3, v5);
    auto n9 = glm::cross(v3, v6);


    std::vector<glm::vec3> normalAxis({n1, n2, n3, n4, n5, n6});
    std::vector<float> projA;
    std::vector<float> projB;
    // Check collision
    bool collision = true;
    for (auto axis : normalAxis)
    {
        if(glm::length(axis) == 0) continue;
        projA.clear();
        projB.clear();
        for (auto point : points)
        {
            glm::vec3 projPoint = PROJECT(point.first, axis);
            float dis = glm::length(projPoint);
            if (!point.second) projA.emplace_back(dis);
            if ( point.second) projB.emplace_back(dis);
        }
        std::sort(projA.begin(), projA.end());
        std::sort(projB.begin(), projB.end());        
        if ((projA.back() < projB.front() || projB.back() < projA.front()))
        {
            collision = false;
            break;
        }
    }
    return collision;
}

inline void LoadModel(uecs::Entity& e)
{
    if (!e.isHas<MODEL>() || !e.isHas<OBJEC_PATH>()) return;
    OPENGLSYSTEM::READING(e.get<OBJEC_PATH>()->_fbxPath.c_str(),
                        e.get<OBJEC_PATH>()->_shaderVertexPath.c_str(),
                        e.get<OBJEC_PATH>()->_shaderPragmentPath.c_str(),
                        e.get<MODEL>());
}

inline void ViewPort(uecs::Entity& e)
{
    if (!e.isHas<Camera>() || !e.isHas<ViewMatrix>()) return;

    auto& cameraPos                       = e.get<Camera>()->cameraPos;
    auto& cameraTarget                    = e.get<Camera>()->cameraTarget;
    auto& cameraUp                        = e.get<Camera>()->cameraUp;
    auto& cameraFov                       = e.get<Camera>()->fov;
    auto& cameraAspect                    = e.get<Camera>()->aspect;
    auto& nearPlane                       = e.get<Camera>()->nearPlane;
    auto& farPlane                        = e.get<Camera>()->farPlane;

    e.get<ViewMatrix>()->viewMatrix       = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    e.get<ViewMatrix>()->projectionMatrix = glm::perspective(glm::radians(cameraFov), cameraAspect, nearPlane, farPlane);


}

inline void Positioning(uecs::Entity& e) 
{
    if (!e.isHas<OBJECT>()) return;

    auto& position    = e.get<OBJECT>()->position;
    auto& angleX      = e.get<OBJECT>()->angle.x;
    auto& angleY      = e.get<OBJECT>()->angle.y;
    auto& angleZ      = e.get<OBJECT>()->angle.z;
    auto& modelMatrix = e.get<OBJECT>()->modelMatrix;

    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angleX), { 1.0f, 0.0f, 0.0f });
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angleY), { 0.0f, 1.0f, 0.0f });
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angleZ), { 0.0f, 0.0f, 1.0f });
}

inline void CollisionSystem(uecs::World& world)
{
    auto entities = world.find<COLLISION>();
    std::vector<std::pair<uecs::Entity, uecs::Entity>> pairCollision;
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {


            if (!entities[i].isHas<OBJECT>()) continue;
            if (!entities[j].isHas<OBJECT>()) continue;

            COLLISION A = *entities[i].get<COLLISION>();
            COLLISION B = *entities[j].get<COLLISION>();
            OBJECT objA = *entities[i].get<OBJECT>();
            OBJECT objB = *entities[j].get<OBJECT>();

            for (size_t k = 0; k < A.points.size(); ++k)
            {
                A.points[k] = A.points[k] * objA.modelMatrix;
                B.points[k] = A.points[k] * objB.modelMatrix;
            }
            if (CheckOBB(A, B)) {
                pairCollision.emplace_back(std::make_pair(entities[i], entities[j]));
                std::cout << entities[i].ID() << " and " << entities[j].ID() << " have collided\n";
            }
        }
    }
    //...do somthing!
}

inline void DrawModel(uecs::Entity& e)
{
    if (!e.isHas<OBJECT>() || !e.isHas<MODEL>()) return;

    glm::mat4& modelMatrix  = e.get<OBJECT>()->modelMatrix;
    glm::mat4& viewMatrix   = e.get_world()->entity("CAMERA").get<ViewMatrix>()->viewMatrix;
    glm::mat4& projMatrix   = e.get_world()->entity("CAMERA").get<ViewMatrix>()->projectionMatrix;
    glm::vec3& scale        = e.get<OBJECT>()->scale;
        MODEL* model        = e.get<MODEL>();

    glUseProgram(model->_shader_program);

    GLint modelLoc  = glGetUniformLocation(model->_shader_program, "model");
    GLint viewLoc   = glGetUniformLocation(model->_shader_program, "view");
    GLint projLoc   = glGetUniformLocation(model->_shader_program, "proj");
    GLint scaleLoc  = glGetUniformLocation(model->_shader_program, "scale");

    if (modelLoc < 0 || viewLoc < 0 || projLoc < 0 || scaleLoc < 0)
        std::cerr << "Uniform location error" << std::endl;

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMatrix));
    glUniform3fv(scaleLoc, 1, glm::value_ptr(scale));

    GLint numTexturesLoc = glGetUniformLocation(model->_shader_program, "numTextures");
    glUniform1i(numTexturesLoc, model->texture.size());


    glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
    glm::vec3 lightColor = glm::vec3(1.0f);

    // pass light parameters to the shaders
    GLint lightPosLoc = glGetUniformLocation(model->_shader_program, "light.position");
    GLint lightColorLoc = glGetUniformLocation(model->_shader_program, "light.color");

    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    // calculate light space transformation matrix
    float near_plane = 0.1f, far_plane = 10.0f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(10.0, 10.0, 10.0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    // pass lightSpaceMatrix to shaders
    GLint lightSpaceMatrixLoc = glGetUniformLocation(model->_shader_program, "lightSpaceMatrix");
    glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));


    // Check if model has textures
    for (std::size_t i = 0; i < model->texture.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i); // Activate proper texture unit before binding
        glBindTexture(GL_TEXTURE_2D, model->texture[i]);
        std::string textureName = "myTexture[" + std::to_string(i) + "]";
        GLint textureLoc = glGetUniformLocation(model->_shader_program, textureName.c_str());
        if (textureLoc != -1)
        {
            glUniform1i(textureLoc, i);
        }
        else
        {
            std::cerr << "Failed to locate: " << textureName << std::endl;
        }
    }

    glBindVertexArray(model->VAO);
    glDrawArrays(GL_TRIANGLES, 0, model->_numVertices);
    glBindVertexArray(0);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
    }

    glUseProgram(0);
}

struct OPENGL
{
    GLFWwindow* window;
    std::vector<OBJECT> obj_list;
    uecs::World world;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    Camera* mainCamera;
    double initialX = 0.0, initialY = 0.0;
    double lastX    = 0.0, lastY    = 0.0;
    bool IsRight = false;
    bool IsMiddle = false;

private:
    void Init() {
        // Initiation glfw
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // create window
        window = glfwCreateWindow(800, 600, "GLEW Example", nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }
        // Set Current window
        glfwMakeContextCurrent(window);
        // Init glew
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            glfwTerminate();
            return;
        }
        glEnable(GL_DEPTH_TEST);
    }

    void clearWindow(){
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        scrollY = yoffset;
    }
public:
    void CreateCamera()
    {
        world.entity("CAMERA").set<ViewMatrix>({}).set<Camera>({});
    }

    void CreateVAO(const char* fbxPath, const char* shaderVertexPath, const char* shaderPragmentPath,
                   const glm::vec3 posiiton, const glm::vec3 angle, const glm::vec3 scale,
                   const char* name = nullptr)
    {
        if (name)
        {
            world.entity(name)
                .set<OBJECT>({ posiiton, angle, scale })
                .set<MODEL>({})
                .set<COLLISION>({})
                .set<OBJEC_PATH>({ fbxPath, shaderVertexPath, shaderPragmentPath });
        }
        else
        {
            world.entity()
                .set<OBJECT>({ posiiton, angle, scale })
                .set<MODEL>({})
                .set<COLLISION>({})
                .set<OBJEC_PATH>({ fbxPath, shaderVertexPath, shaderPragmentPath });
        }
    }

    void CreateSystem()
    {
        world.system<MODEL, OBJEC_PATH > (LoadModel,    uecs::PHASE::Awake);
        world.system<MODEL, OBJECT     > (Positioning,  uecs::PHASE::Awake);
        world.system<Camera, ViewMatrix> (ViewPort,     uecs::PHASE::Update);
        world.system<MODEL, OBJEC_PATH > (DrawModel,    uecs::PHASE::Update);
    }

    void ProcessInput() {
        const float cameraSpeed = 2.5f * deltaTime; // Adjust accordingly
        const float cameraSensitivity = 0.1f; // Adjust accordingly
        glfwPollEvents();
        glfwSetScrollCallback(window, scroll_callback);
        OPENGLSYSTEM::KeyControl(cameraSpeed, mainCamera, window);
        OPENGLSYSTEM::MouseControl(cameraSpeed, IsMiddle, IsRight, scrollY, lastX, lastY, initialX, initialY, mainCamera, window);
    }

    void UpdateTime()
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame; // You need to calculate deltaTime
        lastFrame = currentFrame;
    }

    int run(){
        Init();

        CreateCamera();
        CreateVAO("resource/3d/Duck.dae", "vertex_shader.glsl", "fragment_shader.glsl", 
                   { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 0.0f}, { 0.009f, 0.009f, 0.009f}, "DUCK");
        CreateVAO("resource/3d/terrain.stl", "vertex_shader.glsl", "fragment_shader.glsl",
                   { 0.0f, 0.0f, 0.0f }, { 90.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, "TERRAIN");
        CreateSystem();

        world.once_progress();

        mainCamera = world.entity("CAMERA").get<Camera>();
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        while (!glfwWindowShouldClose(window)) {
            UpdateTime();

            ProcessInput();

            clearWindow();
            
            world.update_progress();
            CollisionSystem(world);
            glfwSwapBuffers(window);
        }
        glfwTerminate();
        return 0;
    }

    ~OPENGL(){
    }
};

