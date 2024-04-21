// TrainSimulator.cpp : Defines the entry point for the console application.
#include <filesystem>
#include <vector>

#include "Camera.h"
#include "Shader.h"
#include "Model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace fs = std::filesystem;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
unsigned int LoadTexture(const char* path);
unsigned int LoadCubemap(std::vector<std::string> faces);
glm::vec3 MoveTrain(float& X, float& Y, float& Z, float& DegreesY, float& DegreesZ);
void Menu();

// settings
constexpr unsigned int SCR_WIDTH = 1920;
constexpr unsigned int SCR_HEIGHT = 1080;
float speed = 1.0f;

bool isMoving = false;

// camera
Camera camera(glm::vec3(600.0f, -50.0f, 100.0f));
float lastX = static_cast<float>(SCR_WIDTH) / 2.0;
float lastY = static_cast<float>(SCR_HEIGHT) / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//light
enum LightAction
{
	SUNRISE,
	SUNSET
};
float ambientStrength = 0.1f;
float specularStrength = 0.5f;
float diffuseStrength = 0.5f;

//camera type
enum class CameraType
{
	FREE,
	THIRDPERSON,
	DRIVER
};
CameraType cameraType = CameraType::FREE;

/*
TODO:
- Add a train model
- Position the train model on the track in 'bucuresti'
- Implement the MoveTrain function
- Adjust the camera view according to the camera type after adding the train model and while moving the train
*/
int main()
{
	Menu();

	// glfw: initialize and configure
	// ------------------------------
	std::cout << "glfw: initialize and configure\n";
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	float rotY = 0, rotZ = 0;

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "TrainSimulator", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

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
	glm::vec3 lightPos(-10.f, 25.f, 10.0f);

	// build and compile shaders
	// -------------------------
	Shader skyboxShader("skybox.vs", "skybox.fs");

	Shader trainShader("model.vs", "model.fs");
	Shader terrainShader("model.vs", "model.fs");

	Shader bucurestiMapShader("model.vs", "model.fs");
	Shader ploiestiMapShader("model.vs", "model.fs");
	Shader bucegiShader("model.vs", "model.fs");
	Shader brasovShader("model.vs", "model.fs");

	Shader lightingShader("PhongLight.vs", "PhongLight.fs");
	Shader lightCubeShader("Lamp.vs", "Lamp.fs");

	Shader shadowShader("ShadowMapping.vs", "ShadowMapping.fs");
	Shader simpleDepthShader("ShadowMappingDepth.vs", "ShadowMappingDepth.fs");

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
		nullptr);
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
	shadowShader.Use();
	shadowShader.SetInt("diffuseTexture", 0);
	shadowShader.SetInt("shadowMap", 1);

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

	float startX = 400.0f;
	float startY = 100.0f;
	float startZ = 100.0f;

	float degreesY = 0.0f;
	float degreesZ = 0.0f;


	unsigned int lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(0);

	//lights
	LightAction light_action = SUNRISE;

	//shadows
	glGenFramebuffers(1, &depthMapFBO);

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

		lightingShader.Use();
		lightingShader.SetVec3("objectColor", 1.0f, 0.5f, 0.31f);
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", camera.Position);
		lightingShader.SetFloat("ambientStrength", ambientStrength);
		lightingShader.SetFloat("specularStrength", specularStrength);
		lightingShader.SetFloat("diffuseStrength", diffuseStrength);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
			static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f,
			3000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.SetMat4("projection", projection);
		lightingShader.SetMat4("view", view);

		// world transformation
		auto model = glm::mat4(1.0f);
		lightingShader.SetMat4("model", model);

		// also draw the lamp object
		lightCubeShader.Use();
		lightCubeShader.SetMat4("projection", projection);
		lightCubeShader.SetMat4("view", view);
		model = glm::mat4(1.0f);
		model = translate(model, lightPos);
		model = scale(model, glm::vec3(10.0f));
		lightCubeShader.SetMat4("model", model);

		glBindVertexArray(lightCubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw scene as normal
		bucurestiMapShader.Use();
		trainShader.Use();
		terrainShader.Use();

		trainShader.SetMat4("projection", projection);
		trainShader.SetMat4("view", view);
		terrainShader.SetMat4("projection", projection);
		terrainShader.SetMat4("view", view);
		bucurestiMapShader.SetMat4("projection", projection);
		bucurestiMapShader.SetMat4("view", view);

		// render the loaded model
		auto train = glm::mat4(1.0f);
		auto _terrain = glm::mat4(1.0f);
		auto _bucuresti = glm::mat4(1.0f);
		auto _brasov = glm::mat4(1.0f);

		if (!isMoving)
			train = translate(train, glm::vec3(startX, startY, startZ));
		else
			train = translate(train, MoveTrain(startX, startY, startZ, rotY, rotZ));

		train = scale(train, glm::vec3(0.3f, 0.3f, 0.3f));
		train = glm::rotate(train, glm::radians(rotY), glm::vec3(0, 1, 0));
		train = glm::rotate(train, glm::radians(0.0f + rotZ), glm::vec3(0, 0, 1));
		trainShader.SetMat4("model", train);
		driverWagon.Draw(trainShader);

		// terrain
		_terrain = translate(_terrain, glm::vec3(-80.0f, -350.0f, 1000.0f));
		_terrain = scale(_terrain, glm::vec3(250.0f, 250.0f, 250.0f));
		terrainShader.SetMat4("model", _terrain);
		terrain.Draw(terrainShader);

		// bucuresti
		_bucuresti = translate(_bucuresti, glm::vec3(800.0f, -280.0f, -935.0f));
		_bucuresti = scale(_bucuresti, glm::vec3(100.0f, 100.0f, 100.0f));
		bucurestiMapShader.SetMat4("model", _bucuresti);
		bucuresti.Draw(bucurestiMapShader);

		// brasov
		_brasov = translate(_brasov, glm::vec3(-3105.0f, -225.0f, -375.0f));
		_brasov = scale(_brasov, glm::vec3(25.0f, 25.0f, 25.0f));
		_brasov = glm::rotate(_brasov, glm::radians(-50.0f), glm::vec3(0, 1, 0));
		brasovShader.SetMat4("model", _brasov);
		brasov.Draw(brasovShader);


		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) // day
		{
			cubemapTexture = LoadCubemap(daySkybox);
			ambientStrength = 0.1f;
			specularStrength = 0.5f;
			diffuseStrength = 0.5f;
		}
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) // night
		{
			cubemapTexture = LoadCubemap(sunsetSkybox);
			ambientStrength = 0.5f;
			specularStrength = 0.1f;
			diffuseStrength = 0.1f;
		}

		switch (cameraType)
		{
		case CameraType::FREE:
			break;
		case CameraType::THIRDPERSON:
			camera.SetViewMatrix(glm::vec3(startX - 15, startY + 50, startZ + 100)); // 3rd person camera (dummy for now) TBI
			break;
		case CameraType::DRIVER:
			camera.SetViewMatrix(glm::vec3(startX, startY + 2, startZ - 9.5)); // driver camera (dummy for now) TBI
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
		if (speed <= 3.5)
			speed += 0.5;
	if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) // decrease speed
		if (speed > 1.5)
			speed -= 0.5;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int LoadTexture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
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

glm::vec3 MoveTrain(float& X, float& Y, float& Z, float& DegreesY, float& DegreesZ)
{
	// to be implemented
	return glm::vec3(X, Y, Z);
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