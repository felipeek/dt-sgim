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
typedef void (*CurvatureFilterCallback)(r32, r32, s32, s32, r32, r32, s32, r32, r32);
typedef void (*TextureChangeSolidCallback)();
typedef void (*TextureChangeDistanceCallback)(r32, r32);
typedef void (*TextureChangeCurvatureCallback)(r32, r32, s32, r32, r32, s32, r32, r32);
typedef void (*TextureChangeNormalsCallback)(s32, r32, r32);
typedef void (*NoiseGeneratorCallback)(r32);
typedef void (*ExportWavefrontCallback)();

static RecursiveFilterCallback recursiveFilterCallback;
static DistanceFilterCallback distanceFilterCallback;
static CurvatureFilterCallback curvatureFilterCallback;
static TextureChangeSolidCallback textureChangeSolidCallback;
static TextureChangeDistanceCallback textureChangeDistanceCallback;
static TextureChangeCurvatureCallback textureChangeCurvatureCallback;
static TextureChangeNormalsCallback textureChangeNormalsCallback;
static NoiseGeneratorCallback noiseGeneratorCallback;
static ExportWavefrontCallback exportWavefrontCallback;

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

extern "C" void menuRegisterTextureChangeSolidCallBack(TextureChangeSolidCallback f)
{
    textureChangeSolidCallback = f;
}

extern "C" void menuRegisterTextureChangeDistanceCallBack(TextureChangeDistanceCallback f)
{
    textureChangeDistanceCallback = f;
}

extern "C" void menuRegisterTextureChangeCurvatureCallBack(TextureChangeCurvatureCallback f)
{
    textureChangeCurvatureCallback = f;
}

extern "C" void menuRegisterTextureChangeNormalsCallBack(TextureChangeNormalsCallback f)
{
    textureChangeNormalsCallback = f;
}

extern "C" void menuRegisterNoiseGeneratorCallBack(NoiseGeneratorCallback f)
{
    noiseGeneratorCallback = f;
}

extern "C" void menuRegisterExportWavefrontCallBack(ExportWavefrontCallback f)
{
    exportWavefrontCallback = f;
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
    static r32 recursiveSpatialFactor = 0.8f;
    static s32 recursiveNumberOfIterations = 3;

    static r32 distanceSpatialFactor = 7.0f;
    static r32 distanceRangeFactor = 0.1f;
    static s32 distanceNumberOfIterations = 3;

    static r32 curvatureSpatialFactor = 50.0f;
    static r32 curvatureRangeFactor = 0.2f;
    static s32 curvatureNumberOfIterations = 3;
    static r32 curvatureBlurSpatialFactor = 0.0f;
    static r32 curvatureBlurRangeFactor = 0.0f;
    static s32 curvatureBlurMode = 0;
    
    static s32 normalsNumberOfIterations = 3;
    static r32 normalsBlurSpatialFactor = 0.9f;
    static r32 normalsBlurRangeFactor = 0.0f;
    static s32 normalsBlurMode = 1;

    static r32 noiseIntensity = 0.0f;

    static s32 textureRadioSelection = 0;

    // Main body of the Demo window starts here.
    if (!ImGui::Begin(MENU_TITLE, 0, 0))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Recursive Filter"))
    {
        ImGui::DragFloat("Spatial Factor##recursive", &recursiveSpatialFactor, 0.002f, 0.0f, 1.0f);
        ImGui::DragInt("Number of Iterations##recursive", &recursiveNumberOfIterations, 0.02f, 1, 10);

        if (ImGui::Button("Filter##recursive"))
        {
            if (recursiveFilterCallback)
                recursiveFilterCallback(recursiveSpatialFactor, recursiveNumberOfIterations);
        }
    }

    if (ImGui::CollapsingHeader("Distance Filter"))
    {
        ImGui::DragFloat("Spatial Factor##distance", &distanceSpatialFactor, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Range Factor##distance", &distanceRangeFactor, 0.002f, 0.0f, 2.0f);
        ImGui::DragInt("Number of Iterations##distance", &distanceNumberOfIterations, 0.02f, 1, 10);


        if (ImGui::Button("Filter##distance"))
        {
            if (distanceFilterCallback)
                distanceFilterCallback(distanceSpatialFactor, distanceRangeFactor, distanceNumberOfIterations);
        }
    }

    if (ImGui::CollapsingHeader("Curvature Filter"))
    {

        ImGui::DragFloat("Spatial Factor##curvature", &curvatureSpatialFactor, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Range Factor##curvature", &curvatureRangeFactor, 0.002f, 0.0f, 2.0f);
        ImGui::DragInt("Number of Iterations##curvature", &curvatureNumberOfIterations, 0.02f, 1, 10);

        const s8* blurModes[] = { "No blur", "Recursive blur", "Distance blur" };
        ImGui::Combo("Curvature Blur Mode##curvature", &curvatureBlurMode, blurModes, IM_ARRAYSIZE(blurModes));

        if (curvatureBlurMode == 2)
        {
            ImGui::DragFloat("Curvature Blur Spatial Factor##curvature", &curvatureBlurSpatialFactor, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Curvature Blur Range Factor##curvature", &curvatureBlurRangeFactor, 0.02f, 0.0f, 2.0f);
        }
        else if (curvatureBlurMode == 1)
        {
            ImGui::DragFloat("Curvature Blur Spatial Factor##curvature", &curvatureBlurSpatialFactor, 0.001f, 0.0f, 1.0f);
        }

        ImGui::Combo("Normals Blur Mode##curvature", &normalsBlurMode, blurModes, IM_ARRAYSIZE(blurModes));

        if (normalsBlurMode == 2)
        {
            ImGui::DragFloat("Normals Blur Spatial Factor##curvature", &normalsBlurSpatialFactor, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Normals Blur Range Factor##curvature", &normalsBlurRangeFactor, 0.02f, 0.0f, 2.0f);
        }
        else if (normalsBlurMode == 1)
        {
            ImGui::DragFloat("Normals Blur Spatial Factor##curvature", &normalsBlurSpatialFactor, 0.001f, 0.0f, 1.0f);
        }

        if (ImGui::Button("Filter##curvature"))
        {
            if (curvatureFilterCallback)
                curvatureFilterCallback(curvatureSpatialFactor, curvatureRangeFactor, curvatureNumberOfIterations,
                    curvatureBlurMode, curvatureBlurSpatialFactor, curvatureBlurRangeFactor, normalsBlurMode, normalsBlurSpatialFactor,
                    normalsBlurRangeFactor);
        }
    }
    
    if (ImGui::CollapsingHeader("Noise Generator"))
    {
        ImGui::DragFloat("Intensity##noise", &noiseIntensity, 0.001f, 0.0f, 1.0f);
        if (ImGui::Button("Apply##noise"))
        {
            if (noiseGeneratorCallback)
                noiseGeneratorCallback(noiseIntensity);
        }
    }

    if (ImGui::CollapsingHeader("Texture"))
    {
        ImGui::RadioButton("Solid##texture", &textureRadioSelection, 0);
        ImGui::RadioButton("Show distance (from distance filter)##texture", &textureRadioSelection, 1);
        ImGui::RadioButton("Show curvature (from curvature filter)##texture", &textureRadioSelection, 2);
        ImGui::RadioButton("Show normals (from curvature filter)##texture", &textureRadioSelection, 3);

        if (ImGui::Button("Apply##texture"))
        {
            switch (textureRadioSelection)
            {
                case 0:
                {
                    if (textureChangeSolidCallback)
                        textureChangeSolidCallback();
                } break;
                case 1:
                {
                    if (textureChangeDistanceCallback)
                        textureChangeDistanceCallback(distanceSpatialFactor, distanceRangeFactor);
                } break;
                case 2:
                {
                    if (textureChangeCurvatureCallback)
                        textureChangeCurvatureCallback(curvatureSpatialFactor, curvatureRangeFactor, curvatureBlurMode,
                            curvatureBlurSpatialFactor, curvatureBlurRangeFactor, normalsBlurMode, normalsBlurSpatialFactor, normalsBlurRangeFactor);
                } break;
                case 3:
                {
                    if (textureChangeNormalsCallback)
                        textureChangeNormalsCallback(normalsBlurMode, normalsBlurSpatialFactor, normalsBlurRangeFactor);
                } break;
            }
        }
    }

    if (ImGui::CollapsingHeader("Manual Filtering"))
    {
        ImGui::Text("Not ready yet...");
    }

    if (ImGui::CollapsingHeader("Export"))
    {
        if (ImGui::Button("Export to wavefront object (.obj)##export"))
        {
            if (exportWavefrontCallback)
                exportWavefrontCallback();
        }
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