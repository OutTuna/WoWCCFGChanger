#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "app_state.h"
#include "ui.h"
#include "meta_store.h"
#include "app_config.h"
#include "utf8_utils.h"
#include "texture_loader.h"
#include "fonts/font_dejavu_sans.h"
#include <cstring>

static void GlfwErrorCallback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static const int kWindowWidth  = 1440;
static const int kWindowHeight = 900;

int main(int, char**) {
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, "WoW CFG Changer", nullptr, nullptr);
    if (!window) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (GLFWmonitor* monitor = glfwGetPrimaryMonitor()) {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (mode) {
            glfwSetWindowPos(window,
                (mode->width  - kWindowWidth)  / 2,
                (mode->height - kWindowHeight) / 2);
        }
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    {
        ImFontConfig fontCfg;
        fontCfg.OversampleH = 2;
        fontCfg.OversampleV = 2;
        fontCfg.FontDataOwnedByAtlas = true;
        const ImWchar* ranges = io.Fonts->GetGlyphRangesCyrillic();

        ImFont* loaded = io.Fonts->AddFontFromMemoryCompressedBase85TTF(
            DejaVuSans_compressed_data_base85, 18.0f, &fontCfg, ranges);

        if (!loaded) {
            io.Fonts->AddFontDefault();
        }
        io.Fonts->Build();
    }

    AppState state;
    MetaStore metaStore;
    metaStore.Load(Utf8::GetExeDir() / "wowcfgchanger_meta.txt");
    state.metaStore = &metaStore;

    AppConfig appConfig;
    appConfig.Load(Utf8::GetExeDir() / "wowcfgchanger_config.txt");
    std::string savedPath = appConfig.GetLastRootPath();
    if (!savedPath.empty()) {
        std::strncpy(state.rootPathBuf, savedPath.c_str(), sizeof(state.rootPathBuf) - 1);
    }
    state.appConfig = &appConfig;

    {
        int texW = 0, texH = 0;
        unsigned int tex = TextureLoader::LoadPngAsTexture(Utf8::GetExeDir() / "app_icon.png", texW, texH);
        if (tex != 0) {
            state.aboutIconTexture = tex;
            state.aboutIconLoaded = true;
        }
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        UI::DrawFrame(state);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.06f, 0.06f, 0.08f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}