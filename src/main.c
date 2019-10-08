#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "menu.h"
#include "core.h"

#define WINDOW_TITLE "gimmesh"

s32 windowWidth = 1366;
s32 windowHeight = 768;
GLFWwindow* mainWindow;

static boolean keyState[1024];	// @TODO: Check range.
static boolean isMenuVisible = false;

static void glfwKeyCallback(GLFWwindow* window, s32 key, s32 scanCode, s32 action, s32 mods)
{
	if (action == GLFW_PRESS)
		keyState[key] = true;
	if (action == GLFW_RELEASE)
		keyState[key] = false;
	if (keyState[GLFW_KEY_ESCAPE])
	{
		if (isMenuVisible)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		isMenuVisible = !isMenuVisible;
		keyState[GLFW_KEY_ESCAPE] = !keyState[GLFW_KEY_ESCAPE];
	}

	if (isMenuVisible)
		menuKeyClickProcess(window, key, scanCode, action, mods);
}

static void glfwCursorCallback(GLFWwindow* window, r64 xPos, r64 yPos)
{
	static boolean resetCoreMouseMovement = true;

	if (!isMenuVisible)
	{
		coreMouseChangeProcess(resetCoreMouseMovement, xPos, yPos);
		resetCoreMouseMovement = false;
	}
	else
		resetCoreMouseMovement = true;
}

static void glfwMouseButtonCallback(GLFWwindow* window, s32 button, s32 action, s32 mods)
{
	r64 xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);
	yPos = windowHeight - yPos;
	if (!isMenuVisible)
		coreMouseClickProcess(button, action, xPos, yPos);
	else
		menuMouseClickProcess(window, button, action, mods);
}

static void glfwScrollCallback(GLFWwindow* window, r64 xOffset, r64 yOffset)
{
	if (!isMenuVisible)
		coreScrollChangeProcess(xOffset, yOffset);
	else
		menuScrollChangeProcess(window, xOffset, yOffset);
}

static void glfwResizeCallback(GLFWwindow* window, s32 width, s32 height)
{
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);
	coreWindowResizeProcess(width, height);
}

static void glfwCharCallback(GLFWwindow* window, u32 c)
{
	if (isMenuVisible)
		menuCharClickProcess(window, c);
}

static GLFWwindow* initGlfw()
{
	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, WINDOW_TITLE, 0, 0);
	glfwSetWindowPos(window, 50, 50);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, glfwKeyCallback);
	glfwSetCursorPosCallback(window, glfwCursorCallback);
	glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
	glfwSetScrollCallback(window, glfwScrollCallback);
	glfwSetWindowSizeCallback(window, glfwResizeCallback);
	glfwSetCharCallback(window, glfwCharCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	return window;
}

static void initGlew()
{
	glewExperimental = true;
	glewInit();
}

extern s32 main(s32 argc, s8** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s file.gim or %s file.obj\n", argv[0], argv[0]);
		return -1;
	}

    r32 deltaTime = 0.0f;

	mainWindow = initGlfw();
	initGlew();

	if (coreInit(argv[1]))
		return -1;

	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	r64 lastFrame = glfwGetTime();
	s32 frameNumber = (s32)lastFrame;
	u32 fps = 0;

	menuInit(mainWindow);

	while (!glfwWindowShouldClose(mainWindow))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.2074f, 0.3168f, 0.3615f, 1.0f);

		coreUpdate(deltaTime);
		coreRender();
		if (!isMenuVisible)
			coreInputProcess(keyState, deltaTime);

		if (isMenuVisible)
			menuRender();

		glfwPollEvents();
		glfwSwapBuffers(mainWindow);

		r64 currentFrame = glfwGetTime();
		if ((s32)currentFrame > frameNumber)
		{
			//printf("FPS: %u\n", fps);
			fps = 0;
			frameNumber++;
		}
		else
			++fps;

		deltaTime = (r32)(currentFrame - lastFrame);

		lastFrame = currentFrame;
	}

	menuDestroy();
	coreDestroy();
	glfwTerminate();
}
