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
bool DefaultChams;
bool ShadingChams;
bool WireframeChams;
bool RainbowChams;
bool OutlineChams;
bool GlowChams;
bool ACTIVEALL;
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
ImGui::CreateContext();
	ImGuiStyle& style = ImGui::GetStyle();
	//style.Colors[ImGuiCol_WindowBg] = ImColor(1.0f, 0.84f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green background color
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 1.0f, 10.0f);
	//style.Colors[ImGuiCol_WindowBg] = ImVec4(15, 255, 253, 255);
	// Blue border color
	style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.843f, 0.0f, 1.0f); // Golden color
    ImGuiIO &io = ImGui::GetIO();
	io.Fonts -> AddFontFromFileTTF("Rubik-Regular.ttf",22.0f);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(g_GlWidth, g_GlHeight);
    ImGui::NewFrame();

    
		 
    ImGui::EndFrame();
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
       if (DefaultChams){
		//	GLint (*old_glGetUniformLocation)(GLuint program, const GLchar *name);
		SetWallhack(true);
	}else{
		if (ACTIVEALL){
		SetWallhack(true);
		} else{
			SetWallhack(false);
		}
	}
	if (ShadingChams){
		SetWallhackS(true);
	}else{
		if (ACTIVEALL){
		SetWallhackS(true);
		} else{
			SetWallhackS(false);
		}
	}
	if (WireframeChams){
		SetWallhackW(true);  
	}else{ 
		if (ACTIVEALL){
		SetWallhackW(true);  
		} else{
			SetWallhackW(false);  
		} 
	}
	if (GlowChams){
		SetWallhackG(true);
	}else{
		if (ACTIVEALL){
		SetWallhackG(true);
		} else{
			SetWallhackG(false);  
		} 
	}
	if (OutlineChams){
		SetWallhackO(true); 
	}else{
		if (ACTIVEALL){
		SetWallhackO(true); 
		} else{
			SetWallhackO(false); 
		} 
	}
	if (RainbowChams){
		SetRainbow(true);
	}else{
		if (ACTIVEALL){
		SetRainbow(true);
		} else{
			SetRainbow(false);
		} 
	}
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
    mlovinit();
    hack_start(_game_data_dir);
}
