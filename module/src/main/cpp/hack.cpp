#include <cstdint>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <dlfcn.h>
#include <string>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2.h>
#include "Includes/Chams.h"
#include "hack.h"
#include "log.h"
#include "game.h"
#include "utils.h"
#include "xdl.h"
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include "MemoryPatch.h"

static int                  g_GlHeight, g_GlWidth;
static bool                 g_IsSetup = false;
static std::string          g_IniFileName = "";
static utils::module_info   g_TargetModule{};

HOOKAF(void, Input, void *thiz, void *ex_ab, void *ex_ac) {
    origInput(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *)thiz);
    return;
}

void SetupImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.IniFilename = g_IniFileName.c_str();
    io.DisplaySize = ImVec2((float)g_GlWidth, (float)g_GlHeight);

    ImGui_ImplAndroid_Init(nullptr);
    ImGui_ImplOpenGL3_Init("#version 300 es");
    ImGui::StyleColorsLight();

    ImFontConfig font_cfg;
    font_cfg.SizePixels = 22.0f;
    io.Fonts->AddFontDefault(&font_cfg);

    ImGui::GetStyle().ScaleAllSizes(3.0f);
}

EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &g_GlWidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &g_GlHeight);

    if (!g_IsSetup) {
      SetupImGui();
      g_IsSetup = true;
    }

    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(g_GlWidth, g_GlHeight);
    ImGui::NewFrame();

    style.FrameRounding = 17.0f;
	 ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20,20)); // Increase the button height
     static int tab = 0;
	 ImGui::SameLine();
	 if (ImGui::Button("CHAMS MENU")){
		 tab = 0;
	 }
	 ImGui::SameLine();
	 if (ImGui::Button("HACKS MENU")){
		 tab = 1;
	 }
	 ImGui::SameLine();
	 if (ImGui::Button("MENU STYLE")){
		 tab = 2;
		 }
	 ImGui::SameLine();
	 if (ImGui::Button("ABOUT ME")){
		 tab = 3;
		/*
		
	ImVec2 startPos(50, 50);  // Starting point of the line
    ImVec2 endPos(200, 50);   // Ending point of the line
    ImU32 color = IM_COL32(255, 0, 0, 255);  // Color of the line (red in this example)
    float thickness = 2.0f;  // Thickness of the line
	// ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10,10));*/
    }

    ImGui::EndFrame();
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return old_eglSwapBuffers(dpy, surface);
}

void hack_start(const char *_game_data_dir) {
//    LOGI("hack start | %s", _game_data_dir);
    do {
        sleep(1);
        g_TargetModule = utils::find_module(TargetLibName);
    } while (g_TargetModule.size <= 0 && mlovinit());
    LOGI("%s: %p - %p",TargetLibName, g_TargetModule.start_address, g_TargetModule.end_address);
    setShader("_SpecColor");
    LogShaders();
    Wallhack(); 
    SetWallhackS(true);

    // TODO: hooking/patching here
    
}

void hack_prepare(const char *_game_data_dir) {
//    LOGI("hack thread: %d", gettid());
    int api_level = utils::get_android_api_level();
//    LOGI("api level: %d", api_level);
    g_IniFileName = std::string(_game_data_dir) + "/files/imgui.ini";
    sleep(5);

    void *sym_input = DobbySymbolResolver("/system/lib/libinput.so", "_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE");
    if (NULL != sym_input){
        DobbyHook((void *)sym_input, (dobby_dummy_func_t) myInput, (dobby_dummy_func_t*)&origInput);
    }
    
    void *egl_handle = xdl_open("libEGL.so", 0);
    void *eglSwapBuffers = xdl_sym(egl_handle, "eglSwapBuffers", nullptr);
    if (NULL != eglSwapBuffers) {
        utils::hook((void*)eglSwapBuffers, (func_t)hook_eglSwapBuffers, (func_t*)&old_eglSwapBuffers);
    }
    xdl_close(egl_handle);

    hack_start(_game_data_dir);
}
