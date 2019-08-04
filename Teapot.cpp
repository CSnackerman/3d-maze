#include "Teapot.h"

void Teapot::render()
{
	glPushMatrix();
		glTranslatef(xPos, yPos, zPos);
		glRotatef(angle, xRot, yRot, zRot);
		glutSolidTeapot(.1);
	glPopMatrix();
}

Teapot::Teapot()
	:
	xPos(0.0f),
	yPos(0.0f),
	zPos(0.0f),
	xRot(0),
	yRot(0),
	zRot(0),
	angle(0),
	rotDirection(1.0f),
	r((float)((random1() % 255 + 1) / 255.0f)),
	g((float)((random1() % 255 + 1) / 255.0f)),
	b((float)((random1() % 255 + 1) / 255.0f))
{
	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 rand(seed1);
	random1 = rand;

	int directionSelect = random1() % 2;
	if (directionSelect)
		rotDirection = 1.0f;

	else
		rotDirection = -1.0f;

	int rotationAxisSelect = random1() % 6;
	switch (rotationAxisSelect)
	{
	case 0:
		xRot = 1.0f;
		break;
	case 1:
		yRot = 1.0f;
		break;
	case 2:
		zRot = 1.0f;
		break;
	case 3:
		xRot = 1.0f;
		yRot = 1.0f;
		break;
	case 4:
		xRot = 1.0f;
		zRot = 1.0f;
		break;
	case 5:
		yRot = 1.0f;
		zRot = 1.0f;
		break;
	}

}

//Only this constructor is called.
Teapot::Teapot(GLfloat x, GLfloat y, GLfloat z)
	:
	xPos(x),
	yPos(y),
	zPos(z),
	xRot(0),
	yRot(0),
	zRot(0),
	angle(0),
	rotDirection(1.0f),
	r((float)((random1() % 255 + 1) / 255.0f)),
	g((float)((random1() % 255 + 1) / 255.0f)),
	b((float)((random1() % 255 + 1) / 255.0f))
{
	//Create random object here and in hw2.cpp to dodge circular dependency. Not sure how else to avoid.
	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 rand(seed1);
	random1 = rand;

	//Randomly assign rotation direction.
	int directionSelect = random1() % 2;
	if (directionSelect)
		rotDirection = 1.0f;
	else
		rotDirection = -1.0f;

	//Randomly assign axis of rotation.
	int rotationAxisSelect = random1() % 6;
	switch (rotationAxisSelect)
	{
	case 0:
		xRot = 1.0f;
		break;
	case 1:
		yRot = 1.0f;
		break;
	case 2:
		zRot = 1.0f;
		break;
	case 3:
		xRot = 1.0f;
		yRot = 1.0f;
		break;
	case 4:
		xRot = 1.0f;
		zRot = 1.0f;
		break;
	case 5:
		yRot = 1.0f;
		zRot = 1.0f;
		break;
	}
}

