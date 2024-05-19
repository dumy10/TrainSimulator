// TrainSimulator.cpp : Defines the entry point for the console application.
#include <filesystem>
#include <vector>

#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "LightAction.h"
#include "CameraType.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <irrKlang.h>

namespace fs = std::filesystem;
namespace irr = irrklang;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
unsigned int LoadCubemap(std::vector<std::string> faces);
void RenderScene(Shader& shader, Model& driverWagon, Model& terrain, Model& brasov, Model& bucharest);
glm::vec3 MoveTrain(glm::vec3& trainPosition, float& degreesX, float& degreesY, float& degreesZ);
void Menu();
void PlaySounds();

// settings
constexpr unsigned int SCR_WIDTH = 1920;
constexpr unsigned int SCR_HEIGHT = 1080;
float speed = 1.0f;

bool isDay = true;
bool isMoving = false;

// camera
Camera camera(glm::vec3(800.0f, -100.0f, -935.0f));
float lastX = static_cast<float>(SCR_WIDTH) / 2.0;
float lastY = static_cast<float>(SCR_HEIGHT) / 2.0;
bool firstMouse = true;

// Sound Engine
irr::ISoundEngine* soundEngine = irr::createIrrKlangDevice();



// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

CameraType cameraType = CameraType::FREE;

// Original position and rotation of the train in 'bucuresti'
glm::vec3 trainPosition(1373.0f, -231.5f, -1482.0f);
glm::vec3 trainRotation(0.0f, 315.3f, 0.0f);

glm::vec3 prevPosition(0.0f, 0.0f, 0.0f);
glm::vec3 prevRotation(0.0f, 0.0f, 0.0f);

glm::vec3 lightPos(-987.766f, 1075.97f, 1217.16f);

float rotationAngleInDegrees = 0.0f;
float rotationSpeed = 0.1f;

glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);

glm::mat4 calculateRotationMatrix(float angleInRadians, const glm::vec3& axis)
{
	return glm::rotate(glm::mat4(1.0f), angleInRadians, axis);
}

void updateRotationAngle()
{
	rotationAngleInDegrees += rotationSpeed;
	if (rotationAngleInDegrees >= 360.0f)
	{
		rotationAngleInDegrees -= 360.0f;
	}
}

int main()
{
	Menu();

	if (!soundEngine)
	{
		std::cout << "Error: Could not initialize sound engine" << std::endl;
		return 1;
	}

	soundEngine->setSoundVolume(0.5f);

	// glfw: initialize and configure
	// ------------------------------
	std::cout << "glfw: initialize and configure\n";
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "TrainSimulator", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// make the window full screen borderless
	glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	float skyboxVertices[] = {
		// positions          
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	};
	std::cout << "Create skybox\n";

	// build and compile shaders
	// -------------------------
	Shader skyboxShader("skybox.vs", "skybox.fs");

	Shader shadowMappingShader("ShadowMapping.vs", "ShadowMapping.fs");
	Shader shadowMappingDepthShader("ShadowMappingDepth.vs", "ShadowMappingDepth.fs");

	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));

	// load textures
	// -------------
	fs::path localPath = fs::current_path();
	std::string textureFolder = localPath.string() + "/Resources/textures";

	Model driverWagon(localPath.string() + "/Resources/train/train.obj");
	Model terrain(localPath.string() + "/Resources/terrain/terrain.obj");
	std::cout << "Loaded terrain\n";

	Model bucuresti(localPath.string() + "/Resources/stations/bucurestiMap/bucuresti.obj");
	Model brasov(localPath.string() + "/Resources/stations/brasovMap/brasov.obj");

	// configure depth map FBO
	// -----------------------
	constexpr unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shader configuration
	// --------------------
	shadowMappingShader.Use();
	shadowMappingShader.SetInt("diffuseTexture", 0);
	shadowMappingShader.SetInt("shadowMap", 1);

	std::vector<std::string> daySkybox
	{
		textureFolder + "/right.jpg",
		textureFolder + "/left.jpg",
		textureFolder + "/top.jpg",
		textureFolder + "/bottom.jpg",
		textureFolder + "/front.jpg",
		textureFolder + "/back.jpg"
	};

	std::vector<std::string> sunsetSkybox
	{
		textureFolder + "/right2.jpg",
		textureFolder + "/left2.jpg",
		textureFolder + "/top2.jpg",
		textureFolder + "/bottom2.jpg",
		textureFolder + "/front2.jpg",
		textureFolder + "/back2.jpg"
	};
	unsigned int cubemapTexture = LoadCubemap(daySkybox);

	skyboxShader.Use();
	skyboxShader.SetInt("skybox", 0);

	// lighting info
	float ambientStrength = 0.7f;
	float specularStrength = 2.1f;
	float diffuseStrength = 2.0f;

	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(0);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);
		PlaySounds();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
			static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f,
			3000.0f);
		glm::mat4 view = camera.GetViewMatrix();

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. render depth of scene to texture (from light's perspective)
		// --------------------------------------------------------------
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		float near_plane = 1.0f, far_plane = 7.5f;

		glm::mat4 lightRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngleInDegrees), rotationAxis);
		glm::vec4 rotatedLightPos = lightRotationMatrix * glm::vec4(lightPos, 1.0f);
		glm::vec3 finalLightPos = glm::vec3(rotatedLightPos);
		lightView = glm::lookAt(finalLightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightSpaceMatrix = lightProjection * lightView;

		// render scene from light's point of view
		shadowMappingDepthShader.Use();
		shadowMappingDepthShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		RenderScene(shadowMappingDepthShader, driverWagon, terrain, brasov, bucuresti);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 2. render scene as normal using the generated depth/shadow map
		// --------------------------------------------------------------
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shadowMappingShader.Use();
		shadowMappingShader.SetMat4("projection", projection);
		shadowMappingShader.SetMat4("view", view);
		// set light uniforms
		shadowMappingShader.SetVec3("viewPos", camera.Position);
		shadowMappingShader.SetVec3("lightPos", lightPos);
		shadowMappingShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

		shadowMappingShader.SetFloat("ambientStrength", ambientStrength);
		shadowMappingShader.SetFloat("specularStrength", specularStrength);
		shadowMappingShader.SetFloat("diffuseStrength", diffuseStrength);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		RenderScene(shadowMappingShader, driverWagon, terrain, brasov, bucuresti);

		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) // day
		{
			cubemapTexture = LoadCubemap(daySkybox);
			ambientStrength = 0.7f;
			specularStrength = 2.1f;
			diffuseStrength = 2.0f;
			isDay = true;
		}
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) // night
		{
			cubemapTexture = LoadCubemap(sunsetSkybox);
			specularStrength = 1.0f;
			diffuseStrength = 1.4f;
			ambientStrength = 0.2f;
			isDay = false;
		}
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		{
			trainPosition = { 1373.0f, -231.5f, -1482.0f };
			trainRotation = { 0.0f, 315.3f, 0.0f };
		}

		switch (cameraType)
		{
		case CameraType::FREE:
			break;
		case CameraType::THIRDPERSON:
			camera.SetViewMatrix(glm::vec3(trainPosition.x - 250, trainPosition.y + 650, trainPosition.z + 1000));
			break;
		case CameraType::DRIVER:
			if (trainRotation.y == 315.3f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 214.65f, trainPosition.y + 67.816f, trainPosition.z + 242.25f));
			else if (trainRotation.y > 304.5f && trainRotation.y < 305.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 252.007f, trainPosition.y + 60.304f, trainPosition.z + 196.588f));
			else if (trainRotation.y > 301.5f && trainRotation.y < 302.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 263.708, trainPosition.y + 71.482f, trainPosition.z + 181.847f));
			else if (trainRotation.y > 295.0f && trainRotation.y < 296.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 287.367f, trainPosition.y + 70.428f, trainPosition.z + 148.222f));
			else if (trainRotation.y > 294.5f && trainRotation.y < 295.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 294.6749, trainPosition.y + 69.428f, trainPosition.z + 151.084));
			else if (trainRotation.y > 287.5f && trainRotation.y < 288.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 303.012f, trainPosition.y + 72.3578f, trainPosition.z + 120.723f));
			else if (trainRotation.y > 279.5f && trainRotation.y < 280.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 314.121f, trainPosition.y + 72.9759f, trainPosition.z + 71.957f));
			else if (trainRotation.y > 274.0f && trainRotation.y < 275.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 318.02f, trainPosition.y + 72.9759f, trainPosition.z + 42.362f));
			else if (trainRotation.y > 272.0f && trainRotation.y < 273.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 319.234f, trainPosition.y + 72.9759f, trainPosition.z + 34.315f));
			else if (trainRotation.y > 269.0f && trainRotation.y < 270.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 325.334f, trainPosition.y + 72.9759f, trainPosition.z - 2.1813f));
			else if (trainRotation.y > 263.0f && trainRotation.y < 265.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 332.735f, trainPosition.y + 70.4831f, trainPosition.z - 13.1938f));
			else if (trainRotation.y > 261.5f && trainRotation.y < 262.8f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 328.09f, trainPosition.y + 71.1831f, trainPosition.z - 30.4746f));
			else if (trainRotation.y > 259.5f && trainRotation.y < 260.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 319.723f, trainPosition.y + 74.9258f, trainPosition.z - 39.3813f));
			else if (trainRotation.y > 255.0f && trainRotation.y < 256.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 314.439f, trainPosition.y + 75.757f, trainPosition.z - 67.524f));
			else if (trainRotation.y > 252.0f && trainRotation.y < 253.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 312.557f, trainPosition.y + 64.6057f, trainPosition.z - 81.3223f));
			else if (trainRotation.y > 249.0f && trainRotation.y < 250.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 315.634f, trainPosition.y + 64.7594f, trainPosition.z - 101.576f));
			else if (trainRotation.y > 247.0f && trainRotation.y < 248.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 304.35f, trainPosition.y + 71.836f, trainPosition.z - 109.488f));
			else if (trainRotation.y > 257.5f && trainRotation.y < 260.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 322.22f, trainPosition.y + 70.9501f, trainPosition.z - 56.2729f));
			else if (trainRotation.y > 268.5f && trainRotation.y < 269.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 319.1f, trainPosition.y + 69.8779f, trainPosition.z - 5.87732f));
			else if (trainRotation.y > 270.5f && trainRotation.y < 270.8f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 319.13f, trainPosition.y + 74.1785f, trainPosition.z - 22.6402f));
			else if (trainRotation.y > 270.8f && trainRotation.y < 271.0f)
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 326.26f, trainPosition.y + 66.0428f, trainPosition.z - 13.5864f));
			else
				camera.SetViewMatrix(glm::vec3(trainPosition.x - 189.463f, trainPosition.y + 63.3484f, trainPosition.z - 8.42737f));

			break;
		default:;
		}

		// draw skybox as last
		glDepthFunc(GL_LEQUAL);
		// change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.Use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		skyboxShader.SetMat4("view", view);
		skyboxShader.SetMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	soundEngine->drop();

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		camera.PrintPosition();

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) // driver camera
		cameraType = CameraType::DRIVER;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) // 3rd person camera
		cameraType = CameraType::THIRDPERSON;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) // free camera
		cameraType = CameraType::FREE;
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) // start train
		isMoving = true;
	if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) // stop train
		isMoving = false;
	if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) // increase speed
		if (speed <= 4.5)
			speed += 0.5;
	if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) // decrease speed
		if (speed >= 1.5)
			speed -= 0.5;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// skybox
// -------------------------------------------------------
unsigned int LoadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
				data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void RenderScene(Shader& shader, Model& driverWagon, Model& terrain, Model& brasov, Model& bucharest)
{
	// render the loaded model
	auto train = glm::mat4(1.0f);
	auto _terrain = glm::mat4(1.0f);
	auto _bucuresti = glm::mat4(1.0f);
	auto _brasov = glm::mat4(1.0f);

	train = (isMoving ? translate(train, MoveTrain(trainPosition, trainRotation.x, trainRotation.y, trainRotation.z)) : translate(train, trainPosition));

	train = scale(train, glm::vec3(10.0f, 10.0f, 10.0f));
	train = glm::rotate(train, glm::radians(trainRotation.x), glm::vec3(1, 0, 0));
	train = glm::rotate(train, glm::radians(trainRotation.y), glm::vec3(0, 1, 0));
	train = glm::rotate(train, glm::radians(trainRotation.z), glm::vec3(0, 0, 1));
	shader.SetMat4("model", train);
	driverWagon.Draw(shader);

	// terrain
	_terrain = translate(_terrain, glm::vec3(-80.0f, -350.0f, 1000.0f));
	_terrain = scale(_terrain, glm::vec3(250.0f, 250.0f, 250.0f));
	shader.SetMat4("model", _terrain);
	terrain.Draw(shader);

	// bucuresti
	_bucuresti = translate(_bucuresti, glm::vec3(800.0f, -300.0f, -930.0f));
	_bucuresti = scale(_bucuresti, glm::vec3(150.0f, 150.0f, 150.0f));
	shader.SetMat4("model", _bucuresti);
	bucharest.Draw(shader);

	// brasov
	_brasov = translate(_brasov, glm::vec3(-3550.0f, -210.0f, -350.0f));
	_brasov = scale(_brasov, glm::vec3(50.0f, 50.0f, 50.0f));
	_brasov = glm::rotate(_brasov, glm::radians(-75.0f), glm::vec3(0, 1, 0));
	shader.SetMat4("model", _brasov);
	brasov.Draw(shader);
}

glm::vec3 MoveTrain(glm::vec3& trainPosition, float& degreesX, float& degreesY, float& degreesZ) {
	if (trainPosition.x > 1032.92f && trainPosition.z < -1084.51f) {
		trainPosition.x -= 0.5f * speed;
		trainPosition.z += 0.5f * speed;
	}
	else if (trainPosition.x > 1061.01f && trainPosition.z < -1170.01f) {
		if (trainPosition.y < -180.5f)
			trainPosition.y += 0.1f * speed;
		if (degreesY > 308.0f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.7f * speed;
		trainPosition.z += 0.89f * speed;
	}
	else if (trainPosition.x > 903.919f && trainPosition.z < -1010.01f) {
		if (degreesY > 305.0f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.7f * speed;
		trainPosition.z += 0.65f * speed;
	}
	else if (trainPosition.x > 794.919f && trainPosition.z < -916.012f) {
		if (trainPosition.y < -180.5f)
			trainPosition.y += 0.1f * speed;
		trainPosition.x -= 0.6f * speed;
		trainPosition.z += 0.4f * speed;
	}
	else if (trainPosition.x > 697.419f && trainPosition.z < -859.512f) {
		trainPosition.x -= 0.6f * speed;
		trainPosition.z += 0.3f * speed;
	}
	else if (trainPosition.x > 658.919f && trainPosition.z < -843.012f) {
		if (degreesY > 302.1f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.52f * speed;
		trainPosition.z += 0.3f * speed;
	}
	else if (trainPosition.x > 603.919f && trainPosition.z < -819.512f) {
		trainPosition.x -= 0.53f * speed;
		trainPosition.z += 0.3f * speed;
	}
	else if (trainPosition.x > 536.919f && trainPosition.z < -766.012f) {
		if (trainPosition.y < -170.5f)
			trainPosition.y += 0.1f * speed;
		trainPosition.x -= 0.5f * speed;
		trainPosition.z += 0.4f * speed;
	}
	else if (trainPosition.x > 468.919f && trainPosition.z < -716.512f) {
		if (degreesY > 295.4f)
			degreesY -= 0.5f;
		if (trainPosition.y < -162.5f)
			trainPosition.y += 0.1f * speed;
		trainPosition.x -= 0.61f * speed;
		trainPosition.z += 0.59f * speed;
	}
	else if (trainPosition.x > 319.419f && trainPosition.z < -647.012f) {
		if (trainPosition.y < -160.0f)
			trainPosition.y += 0.1f * speed;
		trainPosition.x -= 0.5f * speed;
		trainPosition.z += 0.3f * speed;
	}
	else if (trainPosition.x > 247.419f && trainPosition.z < -600.012f) {
		trainPosition.x -= 0.4f * speed;
		trainPosition.z += 0.4f * speed;
	}
	else if (trainPosition.x > 110.919f && trainPosition.z < -543.512f) {
		trainPosition.x -= 0.6f * speed;
		trainPosition.z += 0.4f * speed;
	}
	else if (trainPosition.x > 6.41943f && trainPosition.z < -508.012f) {
		if (degreesY > 300.4f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.5f * speed;
		trainPosition.z += 0.3f * speed;
	}
	else if (trainPosition.x > -107.081f && trainPosition.z < -455.512f) {
		if (degreesY > 298.2f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.5f * speed;
		trainPosition.z += 0.25f * speed;
	}
	else if (trainPosition.x > -138.081f && trainPosition.z < -444.012f) {
		if (degreesY > 295.1f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.5f * speed;
		trainPosition.z += 0.3f * speed;
	}
	else if (trainPosition.x > -153.081f && trainPosition.z < -434.012f) {
		trainPosition.x -= 0.6f * speed;
		trainPosition.z += 0.4f * speed;
	}
	else if (trainPosition.x > -150.476)
	{
		trainPosition.x -= 0.5f * speed;
	}
	else if (trainPosition.x > -193.058f && trainPosition.z < -439.204f) {
		if (degreesY > 292.7f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.6f * speed;
		trainPosition.z += 0.65f * speed;
	}
	else if (trainPosition.x > -299.95f && trainPosition.z < -393.248f) {
		if (degreesY > 288.0f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.6f * speed;
		trainPosition.z += 0.58f * speed;
	}
	else if (trainPosition.x > -326.45f && trainPosition.z < -372.248f) {
		trainPosition.x -= 0.7f * speed;
		trainPosition.z += 0.55f * speed;
	}
	else if (trainPosition.x > -365.45f && trainPosition.z < -372.248f) {
		if (degreesY > 283.799f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.55f * speed;
	}
	else if (trainPosition.x > -427.45f && trainPosition.z < -340.248f) {
		if (degreesY > 279.899f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.65f * speed;
		trainPosition.z += 0.5f * speed;
	}
	else if (trainPosition.x > -484.95f && trainPosition.z < -329.748f) {
		if (degreesY > 274.599f)
			degreesY -= 0.5f;
		if (trainPosition.y > -160.9f)
			trainPosition.y -= 0.1f * speed;
		trainPosition.x -= 0.6f * speed;
		trainPosition.z += 0.6f * speed;
	}
	else if (trainPosition.x > -541.45f && trainPosition.z < -298.248f) {
		if (degreesY > 272.599f)
			degreesY -= 0.5f;
		if (trainPosition.y > -165.9f)
			trainPosition.y -= 0.1f * speed;
		trainPosition.x -= 0.6f * speed;
		trainPosition.z += 0.5f * speed;
	}
	else if (trainPosition.x > -609.95f && trainPosition.z < -294.748f) {
		if (degreesY > 269.898f)
			degreesY -= 0.5f;
		if (trainPosition.y > -168.5f)
			trainPosition.y -= 0.1f * speed;
		trainPosition.x -= 0.6f * speed;
		trainPosition.z += 0.35 * speed;
	}
	else if (trainPosition.x > -645.95f && trainPosition.z < -294.748f) {
		if (degreesY > 268.598f)
			degreesY -= 0.3f;
		trainPosition.x -= 0.6f * speed;
	}
	else if (trainPosition.x > -707.45f && trainPosition.z < -290.748f) {
		if (degreesY > 266.198f)
			degreesY -= 0.3f;
		trainPosition.x -= 0.6f * speed;
		trainPosition.z += 0.4f * speed;
	}
	else if (trainPosition.x > -744.95f && trainPosition.z < -288.248f) {
		if (degreesY > 264.198f)
			degreesY -= 0.3f;
		trainPosition.x -= 0.6f * speed;
		trainPosition.z += 0.3f * speed;
	}
	else if (trainPosition.x > -825.95f && trainPosition.z < -286.748f) {
		if (degreesY > 262.798f)
			degreesY -= 0.3f;
		trainPosition.x -= 0.6f * speed;
		trainPosition.z += 0.2f * speed;
	}
	else if (trainPosition.x > -877.95f && trainPosition.z < -284.748f) {
		if (degreesY > 261.998f)
			degreesY -= 0.3f;
		if (trainPosition.y > -183.5f)
			trainPosition.y -= 0.1f * speed;
		trainPosition.x -= 0.6f * speed;
		trainPosition.z += 0.29f * speed;
	}
	else if (trainPosition.x > -1025.95f && trainPosition.z > -313.748f) {
		if (degreesY > 259.998f)
			degreesY -= 0.3f;
		trainPosition.x -= 0.6f * speed;
		trainPosition.z -= 0.29f * speed;
	}
	else if (trainPosition.x > -1252.95f && trainPosition.z > -362.748f) {
		trainPosition.x -= 0.69f * speed;
		trainPosition.z -= 0.15f * speed;
	}
	else if (trainPosition.x > -1375.45f && trainPosition.z > -388.248f) {
		if (degreesY > 255.498f)
			degreesY -= 0.3f;
		trainPosition.x -= 0.657 * speed;
		trainPosition.z -= 0.08f * speed;
	}
	else if (trainPosition.x > -1494.45f && trainPosition.z > -413.248) {
		if (degreesY > 252.797f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.987f * speed;
		trainPosition.z -= 0.201f * speed;
	}
	else if (trainPosition.x > -1666.77f && trainPosition.z > -469.355f)
	{
		if (degreesY > 252.401f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.7f * speed;
		trainPosition.z -= 0.25f * speed;
	}
	else if (trainPosition.x > -1778.27f && trainPosition.z > -507.355f)
	{
		if (degreesY > 249.7f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.7f * speed;
		trainPosition.z -= 0.22f * speed;
	}
	else if (trainPosition.x > -1913.27f && trainPosition.z > -545.855f)
	{
		trainPosition.x -= 0.7f * speed;
		trainPosition.z -= 0.27f * speed;
	}
	else if (trainPosition.x > -1991.77f && trainPosition.z > -581.855f)
	{
		if (degreesY > 247.9f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.7f * speed;
		trainPosition.z -= 0.20f * speed;
	}
	else if (trainPosition.x > -2101.77f && trainPosition.z > -614.355f)
	{
		trainPosition.x -= 0.7f * speed;
		trainPosition.z -= 0.21f * speed;
	}
	else if (trainPosition.x > -2160.71f && trainPosition.z > -651.319f)
	{
		if (trainPosition.y > -188.5f)
			trainPosition.y -= 0.3f * speed;
		if (degreesY > 251.901f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.5f * speed;
		trainPosition.z -= 0.3f * speed;
	}
	else if (trainPosition.x > -2187.71f && trainPosition.z > -655.319f)
	{
		if (trainPosition.y > -192.5f)
			trainPosition.y -= 0.3f * speed;
		if (degreesY > 253.001f)
			degreesY -= 0.5f;
		trainPosition.x -= 0.7f * speed;
		trainPosition.z -= 0.4f * speed;
	}
	else if (trainPosition.x > -2242.21f && trainPosition.z > -681.319f)
	{
		if (degreesY < 257.602f)
			degreesY += 0.5f;
		trainPosition.x -= 0.8f * speed;
		trainPosition.z -= 0.5f * speed;
	}
	else if (trainPosition.x > -2273.71f && trainPosition.z > -704.819f)
	{
		if (trainPosition.y > -204.5)
			trainPosition.y -= 0.3f * speed;
		if (degreesY < 261.602f)
			degreesY += 0.5f;
		trainPosition.x -= 0.7f * speed;
		trainPosition.z -= 0.4f * speed;
	}
	else if (trainPosition.x > -2340.71f && trainPosition.z > -747.319f)
	{
		if (trainPosition.y > -205.5)
			trainPosition.y -= 0.3f * speed;
		if (degreesY < 263.002f)
			degreesY += 0.5f;
		trainPosition.x -= 0.7f * speed;
		trainPosition.z -= 0.10f * speed;
	}
	else if (trainPosition.x > -2420.71f && trainPosition.z > -757.819f)
	{
		if (trainPosition.y > -224.5)
			trainPosition.y -= 0.3f * speed;
		if (degreesY < 268.602f)
			degreesY += 0.5f;
		trainPosition.x -= 0.7f * speed;
		trainPosition.z -= 0.08f * speed;
	}
	else if (trainPosition.x > -2466.71 && trainPosition.z > -760.819)
	{
		if (trainPosition.y > -231.5)
			trainPosition.y -= 0.3f * speed;
		if (degreesY < 270.803f)
			degreesY += 0.5f;
		trainPosition.x -= 0.7f * speed;
		trainPosition.z -= 0.10f * speed;
	}
	else if (trainPosition.x > -2506.71 && trainPosition.z > -765.319)
	{
		if (trainPosition.y > -235.5)
			trainPosition.y -= 0.3f * speed;
		trainPosition.x -= 0.6f * speed;
		trainPosition.z -= 0.4f * speed;
	}
	else if (trainPosition.x > -2712.21 && trainPosition.z < -730.819)
	{
		if (degreesY < 270.803f)
			degreesY += 0.5f;
		if (trainPosition.y > -236.5)
			trainPosition.y -= 0.3f * speed;
		trainPosition.x -= 0.9f * speed;
		trainPosition.z += 0.08f * speed;
	}
	else if (trainPosition.x > -2902.21 && trainPosition.z < -727.319)
	{
		trainPosition.x -= 0.999f * speed;
		trainPosition.z += 0.08f * speed;
	}
	else if (trainPosition.x > -2934.71 && trainPosition.z < -724.319)
	{
		trainPosition.x -= 0.999f * speed;
		trainPosition.z += 0.065f * speed;

		isMoving = false;
	}

	return glm::vec3(trainPosition.x, trainPosition.y, trainPosition.z);
}

void Menu()
{
	std::cout << "<ENTER> Start the train movement\n"
		"<BACKSPACE> Stop the train movement\n"
		"<1> Driver Camera\n"
		"<2> Outside Camera\n"
		"<3> Free Camera\n"
		"<4> Day Mode\n"
		"<5> Night Mode\n"
		"<+> Increase train speed\n"
		"<-> Decrease train speed\n";
}

void PlaySounds()
{
	fs::path localPath = fs::current_path();
	std::string soundsFolder = localPath.string() + "/Resources/sounds";
	std::string nightSound = soundsFolder + "/nightsound.mp3";
	std::string daySound = soundsFolder + "/daysound.mp3";
	std::string trainSound = soundsFolder + "/trainsound.mp3";
	// If the train is moving and the sound is not playing, play the sound
	if (isMoving && !soundEngine->isCurrentlyPlaying(trainSound.c_str()))
	{
		soundEngine->play2D(trainSound.c_str(), true);
	}
	// If the train is not moving and the sound is playing, stop the sound
	else if (!isMoving && soundEngine->isCurrentlyPlaying(trainSound.c_str()))
	{
		soundEngine->stopAllSounds();
	}

	// If it's night and the night sound is not playing, play the night sound
	if (!isDay && !soundEngine->isCurrentlyPlaying(nightSound.c_str()))
	{
		soundEngine->play2D(nightSound.c_str(), true);
	}
	// If it's day and the night sound is playing, stop the night sound
	else if (isDay && soundEngine->isCurrentlyPlaying(nightSound.c_str()))
	{
		soundEngine->stopAllSounds();
	}

	// If it's day and the day sound is not playing, play the day sound
	if (isDay && !soundEngine->isCurrentlyPlaying(daySound.c_str()))
	{
		soundEngine->play2D(daySound.c_str(), true);
	}
	// If it's night and the day sound is playing, stop the day sound
	else if (!isDay && soundEngine->isCurrentlyPlaying(daySound.c_str()))
	{
		soundEngine->stopAllSounds();
	}
}