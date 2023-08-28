#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <core/App.h>
#include <fstream>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

GLuint image = 0;

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void handleInput();

void refreshImage(uint32_t* data, uint32_t width, uint32_t height) {
    static uint32_t image_width = 0, image_height = 0;

    glBindTexture(GL_TEXTURE_2D, image);
    if (image_height != height || image_width != width) {
        glDeleteTextures(1, &image);
        glGenTextures(1, &image);
        glBindTexture(GL_TEXTURE_2D, image);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    image_height = height;
    image_width = width;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

MyPBRT::App app;

int main(int, char**)
{
    srand(time(NULL));
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 0;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    GLFWwindow* window = glfwCreateWindow(1000, 1000, "MyPBRT", NULL, NULL);
    if (window == NULL)
        return 0;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return 0;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    const char* glsl_version = "#version 130";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    glDisable(GL_DEPTH_TEST);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    double time = 0;
    double lasttime = 0;
    double dt = 0;

    int neki = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&neki);
    std::cout << neki << "\n";
    while (!glfwWindowShouldClose(window))
    {
        time = glfwGetTime();
        dt = time - lasttime;
        lasttime = time;

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Main");
        ImGui::PopStyleVar();

            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

            app.CreateIMGUI();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("Viewport");
            ImGui::PopStyleVar();

                glm::ivec2 resolution = glm::ivec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
                app.Update(dt, resolution);
                refreshImage(app.GetImage(), app.Resolution().x, app.Resolution().y);
                ImGui::Image((void*)(intptr_t)image, ImVec2(resolution.x, resolution.y), ImVec2(0,1), ImVec2(1, 0));

                handleInput();

            ImGui::End();

            ImGui::ShowDemoWindow();

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    app.ScrollMoved(yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        app.ButtonPressed(key);
    }
    else if (action == GLFW_RELEASE) {
        app.ButtonReleased(key);
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    app.MouseMoved(xpos, ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) { }

void handleInput()
{
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        app.MouseReleased(0);
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
        app.MouseReleased(1);
    }
    if (!ImGui::IsWindowHovered()) return;

    glm::vec2 mousepos = glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
    mousepos.x -= ImGui::GetWindowPos().x;
    mousepos.y -= ImGui::GetWindowPos().y;
    mousepos.x /= ImGui::GetWindowSize().x;
    mousepos.y /= ImGui::GetWindowSize().y;
    mousepos.y = 1.0f - mousepos.y;
    app.normalized_mouse_position = mousepos;
    
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        app.MousePressed(0);
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        app.MousePressed(1);
    }
}
