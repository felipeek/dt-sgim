#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "common.h"
#include "vendor/imgui.h"
#include "vendor/imgui_impl_glfw.h"
#include "vendor/imgui_impl_opengl3.h"
#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLSL_VERSION "#version 130"
#define MENU_TITLE "gimmesh"

typedef void (*RecursiveFilterCallback)(r32, s32);
typedef void (*DistanceFilterCallback)(r32, r32, s32);
typedef void (*CurvatureFilterCallback)(r32, r32, s32, s32, r32, r32);
typedef void (*TextureChangeCallback)(s32);
typedef void (*NoiseGeneratorCallback)(r32);

static RecursiveFilterCallback recursiveFilterCallback;
static DistanceFilterCallback distanceFilterCallback;
static CurvatureFilterCallback curvatureFilterCallback;
static TextureChangeCallback textureChangeCallback;
static NoiseGeneratorCallback noiseGeneratorCallback;

extern "C" void menuRegisterRecursiveFilterCallBack(RecursiveFilterCallback f)
{
    recursiveFilterCallback = f;
}

extern "C" void menuRegisterDistanceFilterCallBack(DistanceFilterCallback f)
{
    distanceFilterCallback = f;
}

extern "C" void menuRegisterCurvatureFilterCallBack(CurvatureFilterCallback f)
{
    curvatureFilterCallback = f;
}

extern "C" void menuRegisterTextureChangeCallBack(TextureChangeCallback f)
{
    textureChangeCallback = f;
}

extern "C" void menuRegisterNoiseGeneratorCallBack(NoiseGeneratorCallback f)
{
    noiseGeneratorCallback = f;
}

extern "C" void menuCharClickProcess(GLFWwindow* window, u32 c)
{
    ImGui_ImplGlfw_CharCallback(window, c);
}

extern "C" void menuKeyClickProcess(GLFWwindow* window, s32 key, s32 scanCode, s32 action, s32 mods)
{
    ImGui_ImplGlfw_KeyCallback(window, key, scanCode, action, mods);
}

extern "C" void menuMouseClickProcess(GLFWwindow* window, s32 button, s32 action, s32 mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}

extern "C" void menuScrollChangeProcess(GLFWwindow* window, s64 xoffset, s64 yoffset)
{
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

extern "C" void menuInit(GLFWwindow* window)
{
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);

    // Setup style
    ImGui::StyleColorsDark();
}

static void drawMainWindow()
{
    // Main body of the Demo window starts here.
    if (!ImGui::Begin(MENU_TITLE, 0, 0))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Recursive Filter"))
    {
        static r32 recursiveSpatialFactor = 0.0f;
        static r32 recursiveRangeFactor = 0.0f;
        static s32 recursiveNumberOfIterations = 3;
        ImGui::DragFloat("Spatial Factor##recursive", &recursiveSpatialFactor, 0.001f, 0.0f, 1.0f);
        ImGui::DragInt("Number of Iterations##recursive", &recursiveNumberOfIterations, 0.02f, 1, 10);

        if (ImGui::Button("Filter##recursive"))
        {
            if (recursiveFilterCallback)
                recursiveFilterCallback(recursiveSpatialFactor, recursiveNumberOfIterations);
        }
    }

    if (ImGui::CollapsingHeader("Distance Filter"))
    {
        static r32 distanceSpatialFactor = 0.0f;
        static r32 distanceRangeFactor = 0.0f;
        static s32 distanceNumberOfIterations = 3;
        ImGui::DragFloat("Spatial Factor##distance", &distanceSpatialFactor, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Range Factor##distance", &distanceRangeFactor, 0.02f, 0.0f, 2.0f);
        ImGui::DragInt("Number of Iterations##distance", &distanceNumberOfIterations, 0.02f, 1, 10);


        if (ImGui::Button("Filter##distance"))
        {
            if (distanceFilterCallback)
                distanceFilterCallback(distanceSpatialFactor, distanceRangeFactor, distanceNumberOfIterations);
        }
    }

    if (ImGui::CollapsingHeader("Curvature Filter"))
    {
        static r32 curvatureSpatialFactor = 0.0f;
        static r32 curvatureRangeFactor = 0.0f;
        static s32 curvatureNumberOfIterations = 3;
        static r32 curvatureBlurSpatialFactor = 0.0f;
        static r32 curvatureBlurRangeFactor = 0.0f;
        static s32 curvatureBlurMode = 0;
        
        ImGui::DragFloat("Spatial Factor##curvature", &curvatureSpatialFactor, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Range Factor##curvature", &curvatureRangeFactor, 0.02f, 0.0f, 2.0f);
        ImGui::DragInt("Number of Iterations##curvature", &curvatureNumberOfIterations, 0.02f, 1, 10);

        const s8* items[] = { "No blur", "Recursive blur", "Distance blur" };
        ImGui::Combo("Blur Mode##curvature", &curvatureBlurMode, items, IM_ARRAYSIZE(items));
        ImGui::DragFloat("Blur Spatial Factor##curvature", &curvatureBlurSpatialFactor, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Blur Range Factor##curvature", &curvatureBlurRangeFactor, 0.02f, 0.0f, 2.0f);

        if (ImGui::Button("Filter##curvature"))
        {
            if (curvatureFilterCallback)
                curvatureFilterCallback(curvatureSpatialFactor, curvatureRangeFactor, curvatureNumberOfIterations,
                    curvatureBlurMode, curvatureBlurSpatialFactor, curvatureBlurRangeFactor);
        }
    }
    
    if (ImGui::CollapsingHeader("Noise Generator"))
    {
        static r32 noiseIntensity = 0.0f;
        ImGui::DragFloat("Intensity##noise", &noiseIntensity, 0.001f, 0.0f, 1.0f);
        if (ImGui::Button("Apply##noise"))
        {
            if (noiseGeneratorCallback)
                noiseGeneratorCallback(noiseIntensity);
        }
    }

    if (ImGui::CollapsingHeader("Texture"))
    {
        static s32 radioSelection = 0;
        ImGui::RadioButton("Solid", &radioSelection, 0); ImGui::SameLine();
        ImGui::RadioButton("Curvature (w/ blur)", &radioSelection, 1); ImGui::SameLine();
        ImGui::RadioButton("Curvature (wo/ blur)", &radioSelection, 2);

        if (ImGui::Button("Apply##texture"))
        {
            if (textureChangeCallback)
                textureChangeCallback(radioSelection);
        }
    }

    if (ImGui::CollapsingHeader("Manual Filtering"))
    {
        ImGui::Text("Not ready yet...");
    }

    ImGui::End();
}

extern "C" void menuRender()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

    drawMainWindow();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

extern "C" void menuDestroy()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}