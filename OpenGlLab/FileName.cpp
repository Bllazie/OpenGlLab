#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple> 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" 

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

std::tuple<int, int, int> parseFace(const std::string& face) {
    int vi = -1, ti = -1, ni = -1;
    size_t slash1 = face.find('/');
    size_t slash2 = (slash1 != std::string::npos) ? face.find('/', slash1 + 1) : std::string::npos;

    std::string viStr = face.substr(0, slash1 != std::string::npos ? slash1 : face.size());
    if (!viStr.empty()) {
        try { vi = std::stoi(viStr) - 1; }
        catch (...) {}
    }
    if (slash1 != std::string::npos) {
        size_t tiStart = slash1 + 1;
        size_t tiEnd = (slash2 != std::string::npos) ? slash2 : face.size();
        if (tiStart < tiEnd) {
            std::string tiStr = face.substr(tiStart, tiEnd - tiStart);
            if (!tiStr.empty()) {
                try { ti = std::stoi(tiStr) - 1; }
                catch (...) {}
            }
        }
    }
    if (slash2 != std::string::npos) {
        std::string niStr = face.substr(slash2 + 1);
        if (!niStr.empty()) {
            try { ni = std::stoi(niStr) - 1; }
            catch (...) {}
        }
    }
    return { vi, ti, ni };
}

bool loadOBJ(const char* path, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_texCoords;
    std::vector<glm::vec3> temp_normals;
    std::vector<std::string> lines;
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open OBJ file: " << path << std::endl;
        return false;
    }
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();
    for (const auto& l : lines) {
        std::istringstream iss(l);
        std::string prefix;
        iss >> prefix;
        if (prefix == "v") {
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            temp_vertices.push_back(pos);
        }
        else if (prefix == "vt") {
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            temp_texCoords.push_back(uv);
        }
        else if (prefix == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (prefix == "f") {
            std::vector<std::string> faceVertices;
            std::string token;
            iss.clear();
            iss.seekg(0, std::ios::beg);
            std::string dummy;
            iss >> dummy; 
            while (iss >> token) {
                faceVertices.push_back(token);
            }
            if (faceVertices.size() < 3) continue;
            std::vector<std::tuple<int, int, int>> faceIndices;
            bool valid = true;
            for (const auto& fv : faceVertices) {
                auto result = parseFace(fv);
                int vi = std::get<0>(result);
                int ti = std::get<1>(result);
                int ni = std::get<2>(result);
                if (vi < 0 || vi >= static_cast<int>(temp_vertices.size())) {
                    valid = false;
                    break;
                }
                faceIndices.emplace_back(vi, ti, ni);
            }
            if (!valid) continue;
            unsigned int baseIdx = static_cast<unsigned int>(vertices.size());
            for (size_t i = 0; i < faceIndices.size() - 2; ++i) {
                auto result0 = faceIndices[0];
                int vi0 = std::get<0>(result0);
                int ti0 = std::get<1>(result0);
                int ni0 = std::get<2>(result0);
                auto result1 = faceIndices[i + 1];
                int vi1 = std::get<0>(result1);
                int ti1 = std::get<1>(result1);
                int ni1 = std::get<2>(result1);
                auto result2 = faceIndices[i + 2];
                int vi2 = std::get<0>(result2);
                int ti2 = std::get<1>(result2);
                int ni2 = std::get<2>(result2);
                ti0 = (ti0 >= 0 && ti0 < static_cast<int>(temp_texCoords.size())) ? ti0 : -1;
                ti1 = (ti1 >= 0 && ti1 < static_cast<int>(temp_texCoords.size())) ? ti1 : -1;
                ti2 = (ti2 >= 0 && ti2 < static_cast<int>(temp_texCoords.size())) ? ti2 : -1;
                ni0 = (ni0 >= 0 && ni0 < static_cast<int>(temp_normals.size())) ? ni0 : -1;
                ni1 = (ni1 >= 0 && ni1 < static_cast<int>(temp_normals.size())) ? ni1 : -1;
                ni2 = (ni2 >= 0 && ni2 < static_cast<int>(temp_normals.size())) ? ni2 : -1;
                if (ni0 < 0) ni0 = 0;
                if (ni1 < 0) ni1 = 0;
                if (ni2 < 0) ni2 = 0;
                vertices.push_back({ temp_vertices[vi0], temp_normals[ni0], ti0 >= 0 ? temp_texCoords[ti0] : glm::vec2(0.0f) });
                vertices.push_back({ temp_vertices[vi1], temp_normals[ni1], ti1 >= 0 ? temp_texCoords[ti1] : glm::vec2(0.0f) });
                vertices.push_back({ temp_vertices[vi2], temp_normals[ni2], ti2 >= 0 ? temp_texCoords[ti2] : glm::vec2(0.0f) });
                indices.push_back(baseIdx + 3 * static_cast<unsigned int>(i));
                indices.push_back(baseIdx + 3 * static_cast<unsigned int>(i) + 1);
                indices.push_back(baseIdx + 3 * static_cast<unsigned int>(i) + 2);
            }
        }
    }
    return true;
}

unsigned int loadTexture(const char* path) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "ERROR: Failed to load texture: " << path << std::endl;
        return 0;
    }
    GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return textureID;
}

static std::string loadFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open shader file: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static GLuint compileShader(GLenum type, const char* src) {
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    GLint ok; glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char buf[1024]; glGetShaderInfoLog(id, 1024, nullptr, buf);
        std::cerr << "Shader compile error: " << buf << std::endl;
    }
    return id;
}

static GLuint createProgram(const char* vs, const char* fs) {
    GLuint p = glCreateProgram();
    GLuint a = compileShader(GL_VERTEX_SHADER, vs);
    GLuint b = compileShader(GL_FRAGMENT_SHADER, fs);
    glAttachShader(p, a); glAttachShader(p, b);
    glLinkProgram(p);

    GLint ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char buf[1024]; glGetProgramInfoLog(p, 1024, nullptr, buf);
        std::cerr << "Link error: " << buf << std::endl;
    }
    glDeleteShader(a); glDeleteShader(b);
    return p;
}

void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
    glViewport(0, 0, w, h);
}

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(dir);
}

void processInput(GLFWwindow* window, float deltaTime) {
    float speed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(800, 600, "3D Model with Lighting", nullptr, nullptr);
    if (!win) { std::cerr << "Create window failed\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(win, mouse_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed\n"; return -1;
    }

    std::string vsCode = loadFile("shaders/shader.vert");
    std::string fsCode = loadFile("shaders/shader.frag");
    GLuint prog = createProgram(vsCode.c_str(), fsCode.c_str());

    const char* objPath = "model/Замок3.obj"; 
    const char* texturePath = "model/Bricks097_1K-PNG/Bricks097_1K-PNG_Color.png";  

    const char* objPathSphere = "model/sphere1.obj";
    const char* texturePathSphere = "model/wood/wood_planks_diff_1k.jpg";
    std::vector<Vertex> modelVertices;
    std::vector<unsigned int> modelIndices;
    if (!loadOBJ(objPath, modelVertices, modelIndices)) {
        std::cerr << "Failed to load OBJ. Exiting.\n";
        glfwTerminate();
        return -1;
    }
    
    std::vector<Vertex> modelVerticesSphere;
    std::vector<unsigned int> modelIndicesSphere;
    if (!loadOBJ(objPathSphere, modelVerticesSphere, modelIndicesSphere)) {
        std::cerr << "Failed to load OBJ. Exiting.\n";
        glfwTerminate();
        return -1;
    }
    unsigned int texture = loadTexture(texturePath);
    if (texture == 0) {
        std::cerr << "Failed to load texture. Using white.\n";
    }
    unsigned int textureSphere = loadTexture(texturePathSphere);
    if (texture == 0) {
        std::cerr << "Failed to load texture. Using white.\n";
    }
    GLuint modelVAO, modelVBO, modelEBO;
    glGenVertexArrays(1, &modelVAO);
    glGenBuffers(1, &modelVBO);
    glGenBuffers(1, &modelEBO);
    glBindVertexArray(modelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
    glBufferData(GL_ARRAY_BUFFER, modelVertices.size() * sizeof(Vertex), modelVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelIndices.size() * sizeof(unsigned int), modelIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    GLuint sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, modelVerticesSphere.size() * sizeof(Vertex), modelVerticesSphere.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelIndicesSphere.size() * sizeof(unsigned int), modelIndicesSphere.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
         0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
         
         -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
         -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
         -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
         -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
         
          0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
          0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
          0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
          0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
         
          -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
           0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
           0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
          -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
          
          -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
          -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
           0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
           0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f
    };

    unsigned int cubeIndices[] = {
        
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 15, 14, 14, 13, 12,
        16, 17, 18, 18, 19, 16,
        20, 23, 22, 22, 21, 20
    };

    GLuint lightVAO, lightVBO, lightEBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glGenBuffers(1, &lightEBO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);

    float lastFrame = 0.0f;
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    int modeLoc = glGetUniformLocation(prog, "mode");
    int textureLoc = glGetUniformLocation(prog, "texture1");

    while (!glfwWindowShouldClose(win)) {
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(win, deltaTime);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(prog);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(prog, "uView"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(prog, "uProj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3f(glGetUniformLocation(prog, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(prog, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniform3f(glGetUniformLocation(prog, "lightColor"), 1.0f, 1.0f, 1.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(textureLoc, 0);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.5f)); 
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, -3.0f));
        glUniformMatrix4fv(glGetUniformLocation(prog, "uModel"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(modeLoc, 0); 
        glBindVertexArray(modelVAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(modelIndices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        //sphere
        glBindVertexArray(sphereVAO);
        glm::mat4 sphereModel = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, -3.0f));
        sphereModel = glm::scale(sphereModel, glm::vec3(0.5f));
        glUniformMatrix4fv(glGetUniformLocation(prog, "uModel"), 1, GL_FALSE, glm::value_ptr(sphereModel));

        glUniform1i(modeLoc, 0); 
        glBindTexture(GL_TEXTURE_2D, textureSphere);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(modelIndicesSphere.size()), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        // end sphere

        glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), lightPos);
        lightModel = glm::scale(lightModel, glm::vec3(0.2f));
        glUniformMatrix4fv(glGetUniformLocation(prog, "uModel"), 1, GL_FALSE, glm::value_ptr(lightModel));
        glUniform1i(modeLoc, 1);
        glBindTexture(GL_TEXTURE_2D, 0); 

        glDisable(GL_CULL_FACE); 
        glBindVertexArray(lightVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); 
        glBindVertexArray(0);
        glEnable(GL_CULL_FACE);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &modelVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &modelVBO);
    glDeleteBuffers(1, &modelEBO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteBuffers(1, &lightVBO);
    glDeleteBuffers(1, &lightEBO);
    glDeleteProgram(prog);
    if (texture) glDeleteTextures(1, &texture);
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}