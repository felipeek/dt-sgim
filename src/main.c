#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "menu.h"
#include "core.h"
#include "obj.h"
#include "parametrization.h"

#define WINDOW_TITLE "gimmesh"
#define SPHERICAL_PARAM_ITERATIONS_DEFAULT 500
#define GIM_SIZE_DEFAULT 255
#define GIM_PARAMETRIZATION_DEFAULT_PATH "./export.gim"

s32 windowWidth = 1366;
s32 windowHeight = 768;
GLFWwindow* mainWindow;
static s8* gimPath;

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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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

static void printHelp(s8* app)
{
	printf("Usage:\n\n");
	printf("You can use this application with gim files (geometry images) or obj files (wavefront format).\n\n");
	printf("To load a geometry image:\n\n");
	printf("\t%s -g <example.gim>\n\n", app);
	printf("Optional parameters:\n\n");
	printf("\tNone\n\n");
	printf("To load a wavefront object:\n\n");
	printf("\t%s -o <example.obj>\n\n", app);
	printf("Optional parameters:\n\n");
	printf("\t-e <result.gim>\t: specify the path of the geometry image that will be generated (default: %s)\n", GIM_PARAMETRIZATION_DEFAULT_PATH);
	printf("\t-it <number>\t: number of iterations for spherical parametrization algorithm (default: %d)\n", SPHERICAL_PARAM_ITERATIONS_DEFAULT);
	printf("\t-s <number>\t: size of geometry image (<n> x <n>) [must be an odd number] (default: %d)\n", GIM_SIZE_DEFAULT);
}

// Returns 0 if no error, but UI should not be started
// Returns 1 if no error and UI should be started
// Returns -1 if error
static s32 parseArguments(s32 argc, s8** argv)
{
	boolean validOptionSelected = false;
	boolean convertObjToGeometryImage = false;
	s8* objPath;
	s8* exportPath = GIM_PARAMETRIZATION_DEFAULT_PATH;
	s32 sphericalParametrizationNumberOfIterations = SPHERICAL_PARAM_ITERATIONS_DEFAULT;
	s32 gimSize = GIM_SIZE_DEFAULT;

	if (argc < 2)
	{
		printHelp(argv[0]);
		return -1;
	}

	for (s32 i = 1; i < argc; ++i)
	{
		s8* arg = argv[i];
		if (!strcmp(arg, "-g"))
		{
			if (i == argc - 1)
			{
				fprintf(stderr, "-g requires an argument\n");
				return -1;
			}
			if (validOptionSelected)
			{
				fprintf(stderr, "Invalid set of arguments\n");
				return -1;
			}
			validOptionSelected = true;
			gimPath = argv[i++ + 1];
		}
		else if (!strcmp(arg, "-o"))
		{
			if (i == argc - 1)
			{
				fprintf(stderr, "-o requires an argument\n");
				return -1;
			}
			if (validOptionSelected)
			{
				fprintf(stderr, "Invalid set of arguments\n");
				return -1;
			}
			validOptionSelected = true;
			convertObjToGeometryImage = true;
			objPath = argv[i++ + 1];
		}
		else if (!strcmp(arg, "-e"))
		{
			if (i == argc - 1)
			{
				fprintf(stderr, "-e requires an argument\n");
				return -1;
			}
			exportPath = argv[i++ + 1];
		}
		else if (!strcmp(arg, "-it"))
		{
			if (i == argc - 1)
			{
				fprintf(stderr, "-it requires an argument\n");
				return -1;
			}
			sphericalParametrizationNumberOfIterations = atoi(argv[i++ + 1]);
			if (sphericalParametrizationNumberOfIterations <= 0) {
				fprintf(stderr, "Invalid number of iterations for spherical parametrization.\n");
				return -1;
			}
		}
		else if (!strcmp(arg, "-s"))
		{
			if (i == argc - 1)
			{
				fprintf(stderr, "-s requires an argument\n");
				return -1;
			}
			gimSize = atoi(argv[i++ + 1]);
			if (gimSize <= 0) {
				fprintf(stderr, "Invalid geometry image size: can't be negative.\n");
				return -1;
			} else if (gimSize % 2 == 0) {
				fprintf(stderr, "Invalid geometry image size: must be odd\n");
				return -1;
			}
		}
		else if (!strcmp(arg, "-h") || !strcmp(arg, "--help"))
		{
			printHelp(argv[0]);
			return 0;
		}
		else
		{
			printHelp(argv[0]);
			return -1;
		}
	}

	if (convertObjToGeometryImage)
	{
		GeometryImage gim;
		Vertex* vertices;
		u32* indexes;
		if (objParse(objPath, &vertices, &indexes))
		{
			fprintf(stderr, "Error parsing wavefront file.\n");
			return -1;
		}
		if (paramObjToGeometryImage(indexes, vertices, exportPath, sphericalParametrizationNumberOfIterations, gimSize))
		{
			fprintf(stderr, "Error converting wavefront to geometry image.\n");
			array_release(vertices);
			array_release(indexes);
			return -1;
		}
		array_release(vertices);
		array_release(indexes);

		gimPath = exportPath;
	}

	return validOptionSelected ? 1 : 0;
}

extern s32 main(s32 argc, s8** argv)
{
	s32 ret;
	if ((ret = parseArguments(argc, argv)) != 1)
		return ret;

    r32 deltaTime = 0.0f;
	mainWindow = initGlfw();
	initGlew();

	if (coreInit(gimPath))
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
		/* glClearColor(1,1,1,0.0f); */

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
