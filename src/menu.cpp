#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "common.h"
#include "vendor/imgui.h"
#include "vendor/imgui_impl_glfw.h"
#include "vendor/imgui_impl_opengl3.h"
#include <stdio.h>

#include <dirent.h>
#include <dynamic_array.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLSL_VERSION "#version 330"
#define MENU_TITLE "gimmesh"

typedef void (*FilterCallback)(r32, r32, s32);
typedef void (*TextureChangeSolidCallback)();
typedef void (*TextureChangeCurvatureCallback)(r32, r32);
typedef void (*TextureChangeNormalsCallback)(r32);
typedef void (*TextureChangeCustomCallback)(char*);
typedef void (*NoiseGeneratorCallback)(r32);
typedef void (*ExportWavefrontCallback)();
typedef void (*ExportPointCloudCallback)();
typedef void (*ExportGimCallback)();

static FilterCallback filterCallback;
static TextureChangeSolidCallback textureChangeSolidCallback;
static TextureChangeCurvatureCallback textureChangeCurvatureCallback;
static TextureChangeNormalsCallback textureChangeNormalsCallback;
static TextureChangeCustomCallback textureChangeCustomCallback;
static NoiseGeneratorCallback noiseGeneratorCallback;
static ExportWavefrontCallback exportWavefrontCallback;
static ExportPointCloudCallback exportPointCloudCallback;
static ExportGimCallback exportGimCallback;

static char** availableCustomTexturesPaths;

extern "C" void menuRegisterFilterCallBack(FilterCallback f)
{
    filterCallback = f;
}

extern "C" void menuRegisterTextureChangeSolidCallBack(TextureChangeSolidCallback f)
{
    textureChangeSolidCallback = f;
}

extern "C" void menuRegisterTextureChangeCurvatureCallBack(TextureChangeCurvatureCallback f)
{
    textureChangeCurvatureCallback = f;
}

extern "C" void menuRegisterTextureChangeNormalsCallBack(TextureChangeNormalsCallback f)
{
    textureChangeNormalsCallback = f;
}

extern "C" void menuRegisterTextureChangeCustomCallBack(TextureChangeCustomCallback f)
{
    textureChangeCustomCallback = f;
}

extern "C" void menuRegisterNoiseGeneratorCallBack(NoiseGeneratorCallback f)
{
    noiseGeneratorCallback = f;
}

extern "C" void menuRegisterExportWavefrontCallBack(ExportWavefrontCallback f)
{
    exportWavefrontCallback = f;
}

extern "C" void menuRegisterExportPointCloudCallBack(ExportPointCloudCallback f)
{
    exportPointCloudCallback = f;
}

extern "C" void menuRegisterExportGimCallBack(ExportGimCallback f)
{
    exportGimCallback = f;
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

static void getAvailableCustomTexturesPaths()
{
	const char dirPath[] = "./res/";
	availableCustomTexturesPaths = array_create(char*, 1);

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(dirPath)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			char* dirName = (char*)malloc((sizeof(dirPath) + strlen(ent->d_name)) * sizeof(char));
			strcpy(dirName, dirPath);
			strcpy(dirName + sizeof(dirPath) - 1, ent->d_name);
			array_push(availableCustomTexturesPaths, &dirName);
		}
		closedir (dir);
	}
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

	getAvailableCustomTexturesPaths();
}

static void drawMainWindow()
{
    static r32 filterSpatialFactor = 0.99f;//100.0f;
    static r32 filterRangeFactor = 2.0;
    static s32 filterNumberOfIterations = 3;
    static s32 filterBlurNumberOfIterations = 3;
    
    static r32 noiseIntensity = 0.0f;

    static s32 textureRadioSelection = 0;

    // Main body of the Demo window starts here.
    if (!ImGui::Begin(MENU_TITLE, 0, 0))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    ImGui::SetWindowFontScale(2.0);

    if (ImGui::CollapsingHeader("Filter"))
    {
        ImGui::DragFloat("Spatial Factor##curvature", &filterSpatialFactor, 0.1f, 0.0f, 100.0f, "%.3f");
        ImGui::DragFloat("Range Factor##curvature", &filterRangeFactor, 0.002f, 0.0f, 2.0f, "%.3f");
        ImGui::DragInt("Number of Iterations##curvature", &filterNumberOfIterations, 0.02f, 1, 10);

        if (ImGui::Button("Filter##curvature"))
        {
            if (filterCallback)
                filterCallback(filterSpatialFactor, filterRangeFactor, filterNumberOfIterations);
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
        ImGui::RadioButton("Show curvature##texture", &textureRadioSelection, 1);
        ImGui::RadioButton("Show normals##texture", &textureRadioSelection, 2);
		ImGui::RadioButton("Custom##texture", &textureRadioSelection, 3);

		static int customTextureSelectedItem = 0;
		if (textureRadioSelection == 3)
		{
            ImGui::Combo("", &customTextureSelectedItem, availableCustomTexturesPaths, array_get_length(availableCustomTexturesPaths));
		}

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
                    if (textureChangeCurvatureCallback)
                        textureChangeCurvatureCallback(filterSpatialFactor, filterRangeFactor);
                } break;
                case 2:
                {
                    if (textureChangeNormalsCallback)
                        textureChangeNormalsCallback(filterRangeFactor);
                } break;
				case 3:
				{
				    if (textureChangeCustomCallback)
						if (customTextureSelectedItem >= 0 && customTextureSelectedItem < array_get_length(availableCustomTexturesPaths))
							textureChangeCustomCallback(availableCustomTexturesPaths[customTextureSelectedItem]);
				} break;
            }
        }
    }

    if (ImGui::CollapsingHeader("Export"))
    {
        if (ImGui::Button("Export to wavefront object (.obj)##export"))
        {
            if (exportWavefrontCallback)
                exportWavefrontCallback();
        }

        if (ImGui::Button("Export to point cloud ascii file (.txt)##export"))
        {
            if (exportPointCloudCallback)
                exportPointCloudCallback();
        }

        if (ImGui::Button("Export to gim file (.gim)##export"))
        {
            if (exportGimCallback)
                exportGimCallback();
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

	for (int i = 0; i < array_get_length(availableCustomTexturesPaths); ++i) {
		free(availableCustomTexturesPaths[i]);
	}
	array_release(availableCustomTexturesPaths);
}
