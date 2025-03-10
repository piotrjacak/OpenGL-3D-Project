#include <glad/glad.h>
#include <glfw3.h>
#include "shader.h"
#include "camera.h"
#include "stb_image.h"
#include "sphere.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void renderCube();

unsigned int loadTexture(const char* path);
unsigned int loadCubemap(std::vector<std::string> faces);


const unsigned int SCREEN_WIDTH = 1600;
const unsigned int SCREEN_HEIGHT = 1200;


glm::vec3 mainPosition = glm::vec3(0.0f, 3.0f, 8.0f);

// Camera
Camera cameraStatic(glm::vec3(0.0f, 2.0f, 8.0f));
Camera cameraStaticFollowing(glm::vec3(10.0f, 2.0f, -3.0f), mainPosition - glm::vec3(10.0f, 2.0f, -3.0f));

glm::vec3 offset = glm::vec3(0.0f, 2.0f, 11.0f);
Camera cameraTPP(offset, glm::vec3(0.0f, 0.0f, 3.0f) - offset);

Camera* currentCamera = &cameraStatic;

// Light
glm::vec3 steadyLight = glm::vec3(0.0f, 3.0f, 7.0f);
float spotLightMovingAngle = 0.0f;

// SkyBox
std::vector<std::string> facesDay
{
	"skyboxes/day/right.jpg",
	"skyboxes/day/left.jpg",
	"skyboxes/day/top.jpg",
	"skyboxes/day/bottom.jpg",
	"skyboxes/day/front.jpg",
	"skyboxes/day/back.jpg"
};
std::vector<std::string> facesNight
{
	"skyboxes/night/posx.jpg",
	"skyboxes/night/negx.jpg",
	"skyboxes/night/posy.jpg",
	"skyboxes/night/negy.jpg",
	"skyboxes/night/posz.jpg",
	"skyboxes/night/negz.jpg",
};
unsigned int cubemapTexture;
glm::vec3 dirAmbientLight(0.5f, 0.5f, 0.5f);

// Shaders
Shader* currentShader;
Shader phongShader;
Shader gourardShader;
Shader skyboxShader;

// Textures
unsigned int diffuseMap;
unsigned int specularMap;
unsigned int normalMap;
unsigned int grassTexture;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool normalMapping = false;

unsigned int cubeVAO = 0;
unsigned int cubeVBO, cubeEBO;

int main()
{
	// CONFIGURATION
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glEnable(GL_DEPTH_TEST);


	// SHADERS
	skyboxShader = Shader("skybox.vs", "skybox.fs");

	phongShader = Shader("shader.vs", "shader.fs");
	gourardShader = Shader("gourardShader.vs", "gourardShader.fs");
	currentShader = &phongShader;


	// TEXTURES
	diffuseMap = loadTexture("textures/container2.png");
	specularMap = loadTexture("textures/container2_specular.png");

	grassTexture = loadTexture("textures/grass.jpg");
	cubemapTexture = loadCubemap(facesDay);


	// DATA
	glm::vec3 cubePositions[] = {
		mainPosition,
		glm::vec3(1.2f,  2.0f, -8.0f),
		glm::vec3(-3.8f, 3.0f, -7.3f),
		glm::vec3(0.4f, 4.0f, -3.5f),
		glm::vec3(-2.7f,  1.5f, -8.5f),
		glm::vec3(3.3f, 3.2f, -3.5f),
		glm::vec3(1.7f,  2.0f, 1.0f),
		glm::vec3(1.9f,  4.0f, -1.5f),
		glm::vec3(-2.9f,  1.5f, -1.5f)
	};
	float skyboxVertices[] = {       
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	float floorVertices[] = {
		-20.0f, -0.5f, -20.0f, 0.0f, 1.0f, 0.0f, 0.0f, 10.0f,
		 20.0f, -0.5f, -20.0f, 0.0f, 1.0f, 0.0f, 10.0f, 10.0f,
		 20.0f, -0.5f,  20.0f, 0.0f, 1.0f, 0.0f, 10.0f, 0.0f,
		-20.0f, -0.5f,  20.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f
	};
	unsigned int floorIndices[] = {
		0, 1, 2,
		2, 3, 0
	};

	// FLOOR

	unsigned int floorVAO, floorVBO, floorEBO;
	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorVBO);
	glGenBuffers(1, &floorEBO);
	glBindVertexArray(floorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	// SKYBOX
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);


	// CUBES
	renderCube();


	// SPHERE
	Sphere sphere;
	unsigned int sphereVAO;
	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);

	unsigned int vboId;
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER,
		sphere.getInterleavedVertexSize(),
		sphere.getInterleavedVertices(),
		GL_STATIC_DRAW);

	unsigned int iboId;
	glGenBuffers(1, &iboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sphere.getIndexSize(),
		sphere.getIndices(),
		GL_STATIC_DRAW);

	int attribVert = 0;
	int attribNorm = 1;
	int attribTex = 2;
	glEnableVertexAttribArray(attribVert);
	glEnableVertexAttribArray(attribNorm);
	glEnableVertexAttribArray(attribTex);

	int stride = sphere.getInterleavedStride();
	glVertexAttribPointer(attribVert, 3, GL_FLOAT, false, stride, (void*)0);
	glVertexAttribPointer(attribNorm, 3, GL_FLOAT, false, stride, (void*)(sizeof(float) * 3));
	glVertexAttribPointer(attribTex, 2, GL_FLOAT, false, stride, (void*)(sizeof(float) * 6));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	// RENDERING
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		currentShader->use();

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		// Cameras set up
		glm::vec3 translation = ((float)sin(glfwGetTime()) + 1.0f) * glm::vec3(0.0f, 0.0f, -5.0f);
		cameraStaticFollowing.Front = glm::normalize(translation - cameraStaticFollowing.Position);
		cameraTPP.Position = translation - cameraTPP.Front;

		// Set up transformations
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		currentShader->setMat4("projection", projection);
		glm::mat4 view = currentCamera->GetViewMatrix();
		currentShader->setMat4("view", view);
		currentShader->setVec3("viewPos", view * glm::vec4(currentCamera->Position, 1.0));

		// Set up Material
		currentShader->setInt("material.diffuse", 0);
		currentShader->setFloat("material.shininess", 32.0f);
		currentShader->setVec3("fogColor", 0.1f, 0.1f, 0.1f);
		currentShader->setVec3("cameraPos", currentCamera->Position);
		currentShader->setVec3("lightPos", view * glm::vec4(steadyLight, 1.0f));
		currentShader->setBool("normalMapping", false);

		// Set up DirLight
		currentShader->setVec3("dirLight.direction", glm::normalize(glm::mat3(view)* glm::vec3(0.0f, -1.0f, 0.0f)));
		currentShader->setVec3("dirLight.ambient", dirAmbientLight);
		currentShader->setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
		currentShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

		// Set up steady spotlight
		currentShader->setVec3("spotLight.position", view * glm::vec4(steadyLight, 1.0f));
		currentShader->setVec3("spotLight.direction", glm::normalize(glm::mat3(view) * glm::vec3(0.0f, -3.0f, -7.0f)));
		currentShader->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		currentShader->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		currentShader->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		currentShader->setFloat("spotLight.constant", 1.0f);
		currentShader->setFloat("spotLight.linear", 0.09f);
		currentShader->setFloat("spotLight.quadratic", 0.032f);
		currentShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		currentShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));


		// DRAW OBJECTS
		
		// Draw floor
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grassTexture);
		glBindVertexArray(floorVAO);

		glm::mat4 floorModel = glm::mat4(1.0f);
		currentShader->setMat4("model", floorModel);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		if (!normalMapping)
		{
			currentShader->setBool("normalMapping", false);
			currentShader->setInt("material.specular", 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, specularMap);
		}
		else 
		{
			currentShader->setBool("normalMapping", true);
			currentShader->setInt("material.normal", 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, normalMap);
		}

		// Draw moving object
		glBindVertexArray(cubeVAO);
		glm::mat4 modelFirst = glm::mat4(1.0f);
	
		modelFirst = glm::translate(modelFirst, translation);
		modelFirst = glm::rotate(modelFirst, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		currentShader->setMat4("model", modelFirst);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Draw other objects
		for (unsigned int i = 1; i < 9; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			currentShader->setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// Draw sphere
		glBindVertexArray(sphereVAO);

		glm::mat4 sphereModel = glm::mat4(1.0f);
		sphereModel = glm::translate(sphereModel, glm::vec3(-1.0f, 2.9f, -5.5f));
		currentShader->setMat4("model", sphereModel);

		glDrawElements(GL_TRIANGLES,
			sphere.getIndexCount(),
			GL_UNSIGNED_INT,
			(void*)0);
		glBindVertexArray(0);

		// Light on moving object
		glm::vec3 normal = glm::vec3(spotLightMovingAngle, -0.3f, 1.0f);
		glm::vec3 worldNormal = glm::mat3(glm::transpose(glm::inverse(modelFirst))) * normal;
		glm::vec3 cameraDirection = glm::mat3(view) * worldNormal;

		currentShader->setVec3("spotLightMoving.position", view * glm::vec4(translation, 1.0f));
		currentShader->setVec3("spotLightMoving.direction", glm::normalize(cameraDirection));
		currentShader->setVec3("spotLightMoving.ambient", 0.5f, 0.5f, 0.5f);
		currentShader->setVec3("spotLightMoving.diffuse", 1.0f, 1.0f, 1.0f);
		currentShader->setVec3("spotLightMoving.specular", 1.0f, 1.0f, 1.0f);
		currentShader->setFloat("spotLightMoving.constant", 1.0f);
		currentShader->setFloat("spotLightMoving.linear", 0.09f);
		currentShader->setFloat("spotLightMoving.quadratic", 0.032f);
		currentShader->setFloat("spotLightMoving.cutOff", glm::cos(glm::radians(12.5f)));
		currentShader->setFloat("spotLightMoving.outerCutOff", glm::cos(glm::radians(15.0f)));

		// Draw skybox
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		view = glm::mat4(glm::mat3(currentCamera->GetViewMatrix()));
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}



void renderCube()
{
	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};
	unsigned int indices[] = {
		0, 1, 3, 
		1, 2, 3
	};

	std::vector<float> cubeVertices;
	for (int i = 0; i < 36; i += 3)
	{
		glm::vec3 pos1(vertices[i * 8], vertices[i * 8 + 1], vertices[i * 8 + 2]);
		glm::vec3 pos2(vertices[(i + 1) * 8], vertices[(i + 1) * 8 + 1], vertices[(i + 1) * 8 + 2]);
		glm::vec3 pos3(vertices[(i + 2) * 8], vertices[(i + 2) * 8 + 1], vertices[(i + 2) * 8 + 2]);

		glm::vec2 uv1(vertices[i * 8 + 6], vertices[i * 8 + 7]);
		glm::vec2 uv2(vertices[(i + 1) * 8 + 6], vertices[(i + 2) * 8 + 7]);
		glm::vec2 uv3(vertices[(i + 2) * 8 + 6], vertices[(i + 2) * 8 + 7]);

		// calculate tangent/bitangent vectors of triangle
		glm::vec3 tangent, bitangent;

		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		// Create new array
		for (int j = 0; j < 3; j++)
		{
			int index = (i + j) * 8;
			cubeVertices.insert(cubeVertices.end(), {
				vertices[index], vertices[index + 1], vertices[index + 2],
				vertices[index + 3], vertices[index + 4], vertices[index + 5],
				vertices[index + 6], vertices[index + 7],
				tangent.x, tangent.y, tangent.z,
				bitangent.x, bitangent.y, bitangent.z
			});
		}
	}

	// configure plane VAO
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);

	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Camera
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		currentCamera = &cameraStatic;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		currentCamera = &cameraStaticFollowing;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		currentCamera = &cameraTPP;

	// Day/Night
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		dirAmbientLight = glm::vec3(0.5f, 0.5f, 0.5f);
		cubemapTexture = loadCubemap(facesDay);
	}
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		dirAmbientLight = glm::vec3(0.1f, 0.1f, 0.1f);
		cubemapTexture = loadCubemap(facesNight);
	}

	// Moving spotlight
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		if (spotLightMovingAngle <= 0.7f)
			spotLightMovingAngle += 0.01f;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		if (spotLightMovingAngle >= -0.7f)
			spotLightMovingAngle -= 0.01f;
	}

	// Phong/Gourard
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{	
		currentShader = &gourardShader;
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		currentShader = &phongShader;
	}

	// Normal mapping
	if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
	{
		diffuseMap = loadTexture("textures/brickwall.jpg");
		normalMap = loadTexture("textures/brickwall_normal.jpg");
		normalMapping = true;
	}
	if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
	{
		diffuseMap = loadTexture("textures/container2.png");
		specularMap = loadTexture("textures/container2_specular.png");
		normalMapping = false;
	}
}

unsigned int loadTexture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

	if (data)
	{
		GLenum format = GL_RED;
		if (nrComponents == 3)
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

unsigned int loadCubemap(std::vector<std::string> faces)
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
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
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
