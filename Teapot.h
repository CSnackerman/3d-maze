#pragma once

#include <GL/glut.h>
#include <random>
#include <chrono>



class Teapot
{
public:
	//Member Variables
	GLfloat xPos, yPos, zPos;
	GLfloat xRot, yRot, zRot, angle;
	GLfloat rotDirection;
	GLfloat r, g, b;
	GLfloat mat_diffuse[3] = { r, g, b };

	std::mt19937 random1;

	//Methods
	Teapot();
	Teapot(GLfloat x, GLfloat y, GLfloat z);

	void render();
};

