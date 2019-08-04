//AUTHOR:	COLBY ACKERMAN
//CLASS	:	COMPUTER GRAPHICS - CS4410
//DATE	:	11/29/18
//===================================================

#include <GL/glut.h>
#include <Windows.h>
#include <cmath>
#include <vector>
#include <random>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <stack>
#include <array>
#include "RGBpixmap.h"
#include "Teapot.h"

#define MOVE_SPEED_MODIFIER 1;
#define TURN_SPEED_MODIFIER 1.5;

//Teapots.
std::vector<Teapot> teapots;

//===================TEXTURE STUFF=============================
RGBpixmap pix[3];
GLint topTexture = 0;
GLint bottomTexture = 1;
GLint sideTexture = 2;

//========================MOVEMENT/CAMERA CONTROL===========================
// angle of rotation for the camera direction
float angle = 0.0f;

// actual vector representing the camera's direction
float lx = 0.0f, lz = -1.0f;

// XZ position of the camera
float x = 5.5f, z = 5.5f;

// the key states. These variables will be zero
//when no key is being presses
float deltaAngle = 0.0f;
float deltaMove = 0;
int xOrigin = -1;

void changeSize(int w, int h) {

	// Prevent a divide-by-zero, when window is too short
	if (h == 0)
		h = 1;

	float ratio = w * 1.0 / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(70.0f, ratio, 0.05f, 90.0f);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

void computePos(float deltaMove) {

	x += deltaMove * lx * 0.1f;
	z += deltaMove * lz * 0.1f;
}

void processNormalKeys(unsigned char key, int xx, int yy) {

	if (key == 27)
		exit(0);
}

void pressKey(int key, int xx, int yy) {

	switch (key) {
	case GLUT_KEY_UP:
		deltaMove = 0.5f * MOVE_SPEED_MODIFIER;
		break;
	case GLUT_KEY_DOWN:
		deltaMove = -0.5f * MOVE_SPEED_MODIFIER;
		break;
	case GLUT_KEY_LEFT:
		deltaAngle = -0.03f * TURN_SPEED_MODIFIER;
		break;
	case GLUT_KEY_RIGHT:
		deltaAngle = 0.03f * TURN_SPEED_MODIFIER;
		break;
	}
}

void releaseKey(int key, int x, int y) {

	switch (key) 
	{
	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN:
		deltaMove = 0.0f;
		break;

	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT:
		deltaAngle = 0.0f;
		break;
	}
}

//=========================================================
//===================MAZE GENERATION=======================
//=========================================================

//	Cell Representation:
//
//		   6-------7
//		  /|      /|
//		 / |     / |
//		2--|-----3 |
//		|  5-----|-8
//		| /      |/
//		1--------4

//FRONT FACE	= {1, 2, 3, 4}
//LEFT FACE		= {1, 2, 6, 5}
//RIGHT FACE	= {4, 3, 7, 8}
//BACK FACE		= {5, 6, 7, 8}
//BOTTOM FACE	= {1, 5, 8, 4}
//TOP FACE		= {2, 6, 7, 3}

//NOTE: Maze orientation will never change. cell (0,0) will be at the origin.

//ENUMS
enum Walls //All combinations of walls.
{
	F, B, L, R,
	FB, FL, FR,
	BL, BR,
	LR,
	FBL, FBR, FLR, BLR,
	ALL
};

enum Direction
{
	UP, DOWN, LEFT, RIGHT, NONE
};

//-------------------------------------------------

struct Vertex
{
	GLint x;
	GLint y;
	GLint z;
};

struct Cell
{
	//Container for the 8 vertices of the cell 
		//to be transferred to intertwined vertex array later.
	Vertex vertices[8];

	GLubyte indices[24] = { 4, 5, 6, 7, //front
				1, 2, 6, 5, //right
				0, 1, 5, 4, //bottom
				0, 3, 2, 1, //back
				0, 4, 7, 3, //left
				2, 3, 7, 6 }; //top

	//Active faces depending on where in the maze the cell is.
	bool FRONT, BACK, LEFT, RIGHT;


	bool isBarrier;
	bool gotTunneled = false;

	void render()
	{

		
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

		//top
		glBindTexture(GL_TEXTURE_2D, topTexture);
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex3i(vertices[1].x, vertices[1].y, vertices[1].z);
		glTexCoord2i(0, 1);
		glVertex3i(vertices[5].x, vertices[5].y, vertices[5].z);
		glTexCoord2i(1, 1);
		glVertex3i(vertices[6].x, vertices[6].y, vertices[6].z);
		glTexCoord2i(1, 0);
		glVertex3i(vertices[2].x, vertices[2].y, vertices[2].z);
		glEnd();

		//bottom
		glBindTexture(GL_TEXTURE_2D, bottomTexture);
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex3i(vertices[0].x, vertices[0].y, vertices[0].z);
		glTexCoord2i(0, 1);
		glVertex3i(vertices[4].x, vertices[4].y, vertices[4].z);
		glTexCoord2i(1, 1);
		glVertex3i(vertices[7].x, vertices[7].y, vertices[7].z);
		glTexCoord2i(1, 0);
		glVertex3i(vertices[3].x, vertices[3].y, vertices[3].z);
		glEnd();
		//back
		glBindTexture(GL_TEXTURE_2D, sideTexture);
		if (BACK)
		{
			glBegin(GL_QUADS);
			glTexCoord2i(0, 0);
			glVertex3i(vertices[0].x, vertices[0].y, vertices[0].z);
			glTexCoord2i(0, 1);
			glVertex3i(vertices[1].x, vertices[1].y, vertices[1].z);
			glTexCoord2i(1, 1);
			glVertex3i(vertices[2].x, vertices[2].y, vertices[2].z);
			glTexCoord2i(1, 0);
			glVertex3i(vertices[3].x, vertices[3].y, vertices[3].z);
			glEnd();
		}

		//front
		if (FRONT)
		{
			glBegin(GL_QUADS);
			glTexCoord2i(1, 0);
			glVertex3i(vertices[4].x, vertices[4].y, vertices[4].z);
			glTexCoord2i(1, 1);
			glVertex3i(vertices[5].x, vertices[5].y, vertices[5].z);
			glTexCoord2i(0, 1);
			glVertex3i(vertices[6].x, vertices[6].y, vertices[6].z);
			glTexCoord2i(0, 0);
			glVertex3i(vertices[7].x, vertices[7].y, vertices[7].z);
			glEnd();
		}
		//left
		if (LEFT)
		{
			glBegin(GL_QUADS);
			glTexCoord2i(1, 0);
			glVertex3i(vertices[1].x, vertices[1].y, vertices[1].z);
			glTexCoord2i(0, 0);
			glVertex3i(vertices[0].x, vertices[0].y, vertices[0].z);
			glTexCoord2i(0, 1);
			glVertex3i(vertices[4].x, vertices[4].y, vertices[4].z);
			glTexCoord2i(1, 1);
			glVertex3i(vertices[5].x, vertices[5].y, vertices[5].z);
			glEnd();
		}

		//Right
		if (RIGHT)
		{
			glBegin(GL_QUADS);
			glTexCoord2i(1, 0);
			glVertex3i(vertices[2].x, vertices[2].y, vertices[2].z);
			glTexCoord2i(0, 0);
			glVertex3i(vertices[6].x, vertices[6].y, vertices[6].z);
			glTexCoord2i(0, 1);
			glVertex3i(vertices[7].x, vertices[7].y, vertices[7].z);
			glTexCoord2i(1, 1);
			glVertex3i(vertices[3].x, vertices[3].y, vertices[3].z);
			glEnd();
		}



	}

	//Constructors.
	Cell()
		: FRONT(true), BACK(true), LEFT(true), RIGHT(true), isBarrier(false)
	{
		//Initialize all vertices to 0.
		for (auto &i : vertices)
		{
			i.x = 0;
			i.y = 0;
			i.z = 0;
		}

	}

	Cell(bool f, bool b, bool l, bool r)
		: FRONT(f), BACK(b), LEFT(l), RIGHT(r), isBarrier(false)
	{
		//Initialize all vertices to 0.
		for (auto &i : vertices)
		{
			i.x = 0;
			i.y = 0;
			i.z = 0;
		}
	}

	//Barrier wall cell constructor.
	Cell(bool isBarrier)
		: FRONT(true), BACK(true), LEFT(true), RIGHT(true), isBarrier(true)
	{
		//Initialize all vertices to 0.
		for (auto &i : vertices)
		{
			i.x = 0;
			i.y = 0;
			i.z = 0;
		}
	}



	void removeWalls(Walls w)
	{
		switch (w)
		{
		case F:		FRONT = false; 
			break;
		case B:		BACK = false;
			break;
		case L:		LEFT = false;
			break;
		case R:		RIGHT = false;
			break;
		case FB:	FRONT = false; BACK = false;
			break;
		case FL:	FRONT = false; LEFT = false;
			break;
		case FR:	FRONT = false; RIGHT = false;
			break;
		case BL:	BACK = false; LEFT = false;
			break;
		case BR:	BACK = false; RIGHT = false;
			break;
		case LR:	LEFT = false; RIGHT = false;
			break;
		case FBL:	FRONT = false; LEFT = false; BACK = false;
			break;
		case FBR:	FRONT = false; BACK = false; RIGHT = false;
			break;
		case FLR:	FRONT = false; LEFT = false; RIGHT = false;
			break;
		case BLR:	BACK = false; LEFT = false; RIGHT = false;
			break;
		case ALL:	FRONT = false; LEFT = false; BACK = false; RIGHT = false;
			break;
		default:
			std::cerr << "No wall removal performed." << std::endl;
			break;
		}
		return;
	}
};

//==========================MAZE STRUCT============================

//GLOBALS
#define MAZE_ROWS 12
#define MAZE_COLS 12 //Just keep it square please. 

struct Maze
{
	const unsigned int rows, columns;
	const unsigned int startX = 5;
	const unsigned int startY = MAZE_COLS - 1;
	int rollCount = 0;
	int updateCounter = 0;
	int phase = 1;

	//Pathfinding grid.
	std::vector<std::vector<int>> pathGrid;

	//3d wall manager grid.
	std::vector<std::vector<Cell>> cellGrid;

	//Stack of historical moves for backtracking.
	std::stack<Direction> moveStack;


	//-----------------MAZE CONSTRUCTORs-------------------------------------
	Maze()
		: rows(MAZE_ROWS), columns(MAZE_COLS)
	{
		//Create 2d boolean path layout grid which will be "carved" out.
			//Initially fill with "false".
		pathGrid.resize(rows);
		for (auto &row : pathGrid)
		{
			row.resize(columns);
			for (int i = 0; i < columns; ++i)
			{
				row.at(i) = 0;
			}
		}


		//Create 2d default cell vector.
		cellGrid.resize(rows);
		for (auto& row : cellGrid)
		{
			row.resize(columns);
			for (auto &cell : row)
			{
				cell = Cell();
			}
		}
	}

	//-----------------------MAZE MEMBER FUNCTIONS---------------------------------

	void twoPhaseMazeCarve()
	{
		int numValid = 0;
		bool leftValid = false;
		bool rightValid = false;
		bool upValid = false;
		bool downValid = false;

		bool firstMove = true;
		bool exitCreated = false;
		int wallTouches = 0;

		int stackSize;

		//Position tracking. (starts at bottom.)  
		int x = startX;
		int y = startY;

		//START
		//Create exterior barrier walls only in cellGrid since visited checking takes place in pathGrid.
		for(int i = 0; i < MAZE_ROWS; ++i)
		{
			cellGrid[i][0] = Cell(true);			 //top row
			cellGrid[i][MAZE_COLS - 1] = Cell(true); //bottom row
		}

		for (int i = 0; i < MAZE_COLS; ++i)
		{
			cellGrid[0][i] = Cell(true);			 //left col
			cellGrid[MAZE_ROWS - 1][i] = Cell(true); //right col
		}

		//Mark starting position as visited and create walls in correct spots. (left, right)
		pathGrid[x][y] = 1;
		cellGrid[x][y] = Cell(false, false, true, true); //will break if start position is moved from bottom.
		--y; //move up.
		pathGrid[x][y] = 1;
		cellGrid[x][y].removeWalls(F);
		moveStack.push(UP); //push up.

		//---------------------------------------------------------------------------------------
		//Valid move checking for phase 1 is done in this fashion:
		//
		//	  X		|	In this case, the chosen direction is up, so to check it, all relative spots to 
		//	 XXX	|	position 'C' marked by 'X' must be not visited
		//	  C		|	{'C' = current position; 'X' = check visited positions}
		//---------------------------------------------------------------------------------------
		
		//Loop until phase 1 complete.
		while (phase == 1)
		{
			++updateCounter;
			stackSize = moveStack.size();			

			//BEGIN PHASE 2 if the moveStack is empty and it is not the first move.
			if (moveStack.empty() && !firstMove)
			{
				phase = 2;
				break;
			}
				
			//----------CHECK VALID DIRECTIONS----------------------------
			//Left:
			if (x - 2 >= 0) //stay in bounds. (can touch outer barrier wall, but not enter it)
			{
				//Visited Check.
				if (	pathGrid[x - 1][y]		== 0		 //immediate left.
					&&	pathGrid[x - 2][y]		== 0		 //one beyond immediate left.
					&&	pathGrid[x - 1][y - 1]	== 0	 //up-left
					&&	pathGrid[x - 1][y + 1]	== 0)	 //down-left
				{
					leftValid = true;
					++numValid;

					if (isTouchingLeftWall(x))
					{
						leftValid = false;
						--numValid;
					}
				}
			}

			//Right:
			if (x + 2 < MAZE_COLS) //stay in bounds. (can touch outer barrier wall, but not enter it)
			{
				//Visited Check.
				if (	pathGrid[x + 1][y]		== 0			//immediate right.
					&&	pathGrid[x + 2][y]		== 0		//one beyond immediate right.
					&&	pathGrid[x + 1][y - 1]	== 0	//up-right
					&&	pathGrid[x + 1][y + 1]	== 0)	//down-right
				{
					rightValid = true;
					++numValid;

					if (isTouchingRightWall(x))
					{
						rightValid = false;
						--numValid;
					}
				}
			}

			//Up:
			if (y - 2 >= 0) //stay in bounds. (can touch outer barrier wall, but not enter it)
			{
				//Visited Check.
				if (	pathGrid[x][y - 1]		== 0			 //immediate up.
					&&	pathGrid[x][y - 2]		== 0		 //one beyond immediate up.
					&&	pathGrid[x - 1][y - 1]	== 0	 //up-left
					&&	pathGrid[x + 1][y - 1]	== 0)	 //up-right
				{
					upValid = true;
					++numValid;

					if (isTouchingUpWall(y))
					{
						upValid = false;
						--numValid;
					}
				}
			}

			//Down:
			if (y + 2 < MAZE_ROWS) //stay in bounds. (can touch outer barrier wall, but not enter it)
			{
				//Visited Check.
				if (	pathGrid[x][y + 1]		== 0			//immediate down.
					&&	pathGrid[x][y + 2]		== 0		//one beyond immediate down.
					&&	pathGrid[x + 1][y + 1]	== 0	//down-right.
					&&	pathGrid[x - 1][y + 1]	== 0)	//down-left.
				{
					downValid = true;
					++numValid;

					if (isTouchingDownWall(y))
					{
						downValid = false;
						--numValid;
					}
				}
			}

			//Check for wall touch. Doesn't seem to matter to much where this is
			//placed within the loop. Using it to delay exit creation.
			if (isTouchingWall(x, y))
				++wallTouches;

			//Create exit when wallTouches is high enough.
			createExit(wallTouches, x, y);

			//For first move, disable left & right.
			if (firstMove)
				firstMove = false;

			//-------------UPDATE GRIDS-------------------------------

			Direction d = NONE;
			if(!moveStack.empty())
				d = moveStack.top();

			switch (numValid)
			{
			case 0: 
				//BACKTRACK to a position where valid poisitions are available.
				switch (d)
				{
				case UP: ++y; //move down
					break;

				case DOWN: --y; //move up
					break;

				case LEFT: ++x; //move right
					break;

				case RIGHT: --x; //move left
					break;

				case NONE:
					std::cerr << "NONE on the stack." << std::endl;
					break;
				}

				//pop stack.
				if (!moveStack.empty())
					moveStack.pop();

				//Redraw
				Sleep((DWORD)0);
				system("CLS");
				displayCells(x, y, phase);
				printCellWalls(x, y);
				std::cerr << "TOTAL ROLLS: " << rollCount << std::endl << std::endl;

				continue;
	
			case 1:
			case 2:
			case 3:
				updateGrids(upValid, rightValid, downValid, leftValid, numValid, x, y);
				break;

			default:
				std::cerr << "Grid Update Error." << std::endl;
				break;
			}
		}

		//Clear the stack.
		while (!moveStack.empty())
			moveStack.pop();

		//PHASE 2:
		//Going to try and use a simpler method here. 
		//Scan from top to bottom, left to right for unvisited cells,
		//then enter from only one point to avoid creating loops.
		//Delete only entrance wall, and cross-sectional walls of unvisited tunnels.
		while (phase == 2)
		{
			//iterate through the pathGrid and search for no-visits.
			//Stay off the barrier walls.
			for (int i = 1; i < MAZE_COLS - 1; ++i)
			{
				for (int j = 1; j < MAZE_ROWS - 1; ++j)
				{
					int x = j;
					int y = i;

					//Create entrance to tunnel.
					if (pathGrid[x][y] == 0)
					{
						pathGrid[x][y] = 1;
						cellGrid[x][y].gotTunneled = true;
						Direction tunnelEntrance = checkValidMoves(x, y, 1);
						carve(x, y, tunnelEntrance);
					}

					//Reset  x, y for tunneling.
					x = j;
					y = i;
					
					Direction tunnelDirection = checkValidMoves(x, y, 0);
					while (tunnelDirection != NONE)
					{
						carve(x, y, tunnelDirection);
						tunnelDirection = checkValidMoves(x, y, 0);
						system("CLS");
						p2DisplayCells(i, j, x, y);
						printCellWalls(x, y);
						
					}
					
					
				}
			}

			//End maze Generation.

			phase = 0;
		}
		
	}

	void setVertices()
	{
		//iterate through all cells.
		for (int i = 0; i < MAZE_COLS; ++i)
		{
			for (int j = 0; j < MAZE_ROWS; ++j)
			{
				//Set x values based on indices shown above.
				cellGrid[i][j].vertices[0].x = i;
				cellGrid[i][j].vertices[1].x = i;
				cellGrid[i][j].vertices[2].x = i + 1;
				cellGrid[i][j].vertices[3].x = i + 1;
				cellGrid[i][j].vertices[4].x = i;
				cellGrid[i][j].vertices[5].x = i;
				cellGrid[i][j].vertices[6].x = i + 1;
				cellGrid[i][j].vertices[7].x = i + 1;

				//Set y
				cellGrid[i][j].vertices[0].y = 0;
				cellGrid[i][j].vertices[1].y = 1;
				cellGrid[i][j].vertices[2].y = 1;
				cellGrid[i][j].vertices[3].y = 0;
				cellGrid[i][j].vertices[4].y = 0;
				cellGrid[i][j].vertices[5].y = 1;
				cellGrid[i][j].vertices[6].y = 1;
				cellGrid[i][j].vertices[7].y = 0;

				//Set z
				cellGrid[i][j].vertices[0].z = j;
				cellGrid[i][j].vertices[1].z = j;
				cellGrid[i][j].vertices[2].z = j;
				cellGrid[i][j].vertices[3].z = j;
				cellGrid[i][j].vertices[4].z = j + 1;
				cellGrid[i][j].vertices[5].z = j + 1;
				cellGrid[i][j].vertices[6].z = j + 1;
				cellGrid[i][j].vertices[7].z = j + 1;
			}
		}
		return;
	}

	void updateGrids(bool &uValid, bool &rValid, bool &dValid, bool &lValid,
					 int &nValid, 
					 int &currentX, int &currentY)
	{
		int roll = rand() % 4;
		bool selecting = true;

		while(selecting)
		{
			//Two cases:
				// 1. nValid == 1	Only one path is valid. 
				//					The second argument in each if statement ({u,d,l,r}Valid && nValid == 1) return true 
				//					based on which path is valid regardless of roll.
				//
				// 2. nValid > 1	The selection should be based solely one the roll.
				//					The first argument will only return true if the roll and path validation bool are true.
				//					
				// ORing these two arguments together causes the program to automatically select what to do correctly.
				//	**forgot an edge case. Had to && selecting == true to first argument.

			//UP
			if ((uValid && roll == 0 && selecting == true) || (uValid && nValid == 1))
			{
				//Update moveStack.
				moveStack.push(Direction::UP);

				//Tear down UP (back) wall before updating position value.
				cellGrid[currentX][currentY].removeWalls(Walls::B);
				displayCells(currentX, currentY, 1);
				printCellWalls(currentX, currentY);

				//Update position.
				--currentY;

				//Now that grid position is updated, tear down wall opposite direction of travel.
				cellGrid[currentX][currentY].removeWalls(Walls::F);

				//Update pathGrid.
				pathGrid[currentX][currentY] = 1;

				//Remove validity.
				uValid = false;

				//Break the loop.
				selecting = false;
				
			}
			//RIGHT
			else if ((rValid && roll == 1 && selecting == true) || (rValid && nValid == 1))
			{
				moveStack.push(Direction::RIGHT);
				cellGrid[currentX][currentY].removeWalls(Walls::R);
				displayCells(currentX, currentY, 1);
				printCellWalls(currentX, currentY);

				++currentX;
				cellGrid[currentX][currentY].removeWalls(Walls::L);
				pathGrid[currentX][currentY] = 1;
				selecting = false;
			}
			//DOWN
			else if ((dValid && roll == 2 && selecting == true) || (dValid && nValid == 1))
			{
				moveStack.push(Direction::DOWN);
				cellGrid[currentX][currentY].removeWalls(Walls::F);
				displayCells(currentX, currentY, 1);
				printCellWalls(currentX, currentY);
				++currentY;
				cellGrid[currentX][currentY].removeWalls(Walls::B);
				pathGrid[currentX][currentY] = 1;
				selecting = false;
			}
			//LEFT
			else if ((lValid && roll == 3 && selecting == true) || (lValid && nValid == 1))
			{
				moveStack.push(Direction::LEFT);
				cellGrid[currentX][currentY].removeWalls(Walls::L);
				displayCells(currentX, currentY, 1);
				printCellWalls(currentX, currentY);
				--currentX;
				cellGrid[currentX][currentY].removeWalls(Walls::R);
				pathGrid[currentX][currentY] = 1;
				selecting = false;
			}
			else
			{
				//Reroll.
				++rollCount;
				roll = rand() % 4;
			}
				
		}
		//Stat Display.
		displayCells(currentX, currentY, 1);
		printCellWalls(currentX, currentY);
		std::cerr << "TOTAL ROLLS: " << rollCount << std::endl << std::endl;
		system("CLS");

		//Reset numValid.
		nValid = 0;

		//Reset directional validities.
		lValid = false;
		rValid = false;
		uValid = false;
		dValid = false;
	}

	bool isTouchingWall(int currentX, int currentY) { return (currentX == 1 || currentY == 1 || currentX == MAZE_COLS - 2 || currentY == MAZE_ROWS - 2); }
	bool isTouchingLeftWall(int currentX) { return (currentX == 1); }
	bool isTouchingUpWall(int currentY) { return (currentY == 1); }
	bool isTouchingRightWall(int currentX) { return (currentX == MAZE_COLS - 2); }
	bool isTouchingDownWall(int currentY) { return (currentY == MAZE_ROWS - 2); }

	void carve(int &x, int &y, Direction d)
	{
		if (d == NONE)
			return;
		else
		{
			switch (d)
			{
			case UP:
				cellGrid[x][y].gotTunneled = true;
				cellGrid[x][y].removeWalls(B);
				pathGrid[x][y] = 1;
				--y;
				cellGrid[x][y].removeWalls(F);
				pathGrid[x][y] = 1;
				break;

			case DOWN:
				cellGrid[x][y].gotTunneled = true;
				cellGrid[x][y].removeWalls(F);
				pathGrid[x][y] = 1;
				++y;
				cellGrid[x][y].removeWalls(B);
				pathGrid[x][y] = 1;
				break;

			case LEFT:
				cellGrid[x][y].gotTunneled = true;
				cellGrid[x][y].removeWalls(L);
				pathGrid[x][y] = 1;
				--x;
				cellGrid[x][y].removeWalls(R);
				pathGrid[x][y] = 1;
				break;

			case RIGHT:
				cellGrid[x][y].gotTunneled = true;
				cellGrid[x][y].removeWalls(R);
				pathGrid[x][y] = 1;
				++x;
				cellGrid[x][y].removeWalls(L);
				pathGrid[x][y] = 1;
				break;

			default:
				std::cerr << "Carve Error.\n";
			}
		}
	}

	void displayCells(const int cx, const int cy, const int currentPhase)
	{
		for (int i = 0; i < MAZE_COLS; ++i)
		{
			for (int j = 0; j < MAZE_COLS; ++j)
			{

				if ((j != cx || i != cy) && pathGrid[j][i] == 1 && !(j == cx && i == cy))
					std::cerr << "X ";
				else if (j == cx && i == cy)
					std::cerr << "C ";
				else
					std::cerr << ". ";
			}
			std::cerr << "\n";
		}
		std::cerr << "\n";
	}

	void p2DisplayCells(const int ci, const int cj, const int cx, const int cy)
	{
		for (int i = 0; i < MAZE_COLS; ++i)
		{
			for (int j = 0; j < MAZE_COLS; ++j)
			{
				//Iterator, tunneler, unvisited, visted, tunneled.

				if (i == ci && j == cj) //phase 2 iterator
				{
					std::cerr << "I ";
					continue;
				}
				else if (cellGrid[j][i].gotTunneled) //tunneled
				{
					std::cerr << "* ";
					continue;
				}
				else if (pathGrid[j][i] == 1) //visited
				{
					std::cerr << "X ";
					continue;
				}
				else
					std::cerr << ". "; //unvisited
			}
			std::cerr << "\n";
		}
		std::cerr << "\n";
	}

	void createExit(const int numAgainstWall, int tempX, int tempY)
	{
		//this condiditional will automatically become true.
		if (numAgainstWall == 7) 
		{
			//Determine which wall up against.
			if (isTouchingLeftWall(tempX))
			{
				cellGrid[tempX][tempY].removeWalls(Walls::L); //cell next to barrier wall
				cellGrid[tempX - 1][tempY].removeWalls(Walls::LR); //barrier cell itself
				pathGrid[tempX - 1][tempY] = 1;
			}
			else if (isTouchingRightWall(tempX))
			{
				cellGrid[tempX][tempY].removeWalls(Walls::R); //cell next to barrier wall
				cellGrid[tempX + 1][tempY].removeWalls(Walls::LR); //barrier cell itself
				pathGrid[tempX + 1][tempY] = 1;
			}
			else if (isTouchingDownWall(tempY))
			{
				cellGrid[tempX][tempY].removeWalls(Walls::F); //cell next to barrier wall
				cellGrid[tempX][tempY + 1].removeWalls(Walls::FB); //barrier cell itself
				pathGrid[tempX][tempY + 1] = 1;
			}
			else if (isTouchingUpWall(tempY))
			{
				cellGrid[tempX][tempY].removeWalls(Walls::B); //cell next to barrier wall
				cellGrid[tempX][tempY - 1].removeWalls(Walls::FB); //barrier cell itself
				pathGrid[tempX][tempY - 1] = 1;
			}
			else
			{
				std::cerr << "Exit creation failed." << std::endl;
			}
		}

		return;
		
	}

	Direction checkValidMoves(int tempX, int tempY, int visitCheck)
	{										//			up		down	left	right
		static std::array<std::array<int, 2>, 4> moveList = { {{0, -1}, {0, 1}, {-1, 0}, {1, 0}} };

		for (int move = 0; move < 4; ++move)
		{
			//Check for unvisited and not external barrier.
			if (pathGrid[tempX + moveList[move][0]][tempY + moveList[move][1]] == visitCheck
				&& !cellGrid[tempX + moveList[move][0]][tempY + moveList[move][1]].isBarrier)
			{
				return (Direction)(move);
			}
		}
		return Direction::NONE;


		
	}

	void printCellWalls(const int tempX, const int tempY)
	{
		//print the state of each side wall.
		std::cerr	<< "\nFront: " << cellGrid[tempX][tempY].FRONT << "\n"
					<< "Back: " << cellGrid[tempX][tempY].BACK << "\n"
					<< "Left: " << cellGrid[tempX][tempY].LEFT << "\n"
					<< "Right: " << cellGrid[tempX][tempY].RIGHT << "\n"
					<< std::endl;
	}
};


//Create Global Vertex Array.
void createVertexArray(GLint arr[], Maze* m)
{
	//Generate maze and set the vertex values.
	m->twoPhaseMazeCarve();
	m->setVertices();

	//Transfer vertex values of each cell into one array.

	//3 ints per vertex * 8 vertices per cell * area of maze.
	std::vector<GLint> temp;
	for (int i = 0; i < MAZE_ROWS; ++i)
	{
		for (int j = 0; j < MAZE_COLS; ++j)
		{
			for (int k = 0; k < 8; ++k)
			{
				temp.push_back(m->cellGrid[i][j].vertices[k].x);
				temp.push_back(m->cellGrid[i][j].vertices[k].y);
				temp.push_back(m->cellGrid[i][j].vertices[k].z);
			}
		}
	}

	//Transfer from vector to vertex array.
	int index = 0;
	for (auto &i : temp)
	{
		arr[index] = i;
		++index;
	}

}


void initTextures()
{
	//init textures.
	pix[0].readBMPFile("floor.bmp");
	pix[0].setTexture(bottomTexture);

	pix[1].readBMPFile("wall.bmp");
	pix[1].setTexture(sideTexture);

	pix[2].readBMPFile("ceiling.bmp");
	pix[2].setTexture(topTexture);
}

//--------------------------------------------------------

const int vertexArraySize = 3 * 8 * MAZE_ROWS*MAZE_COLS;
const int indexArraySize = 24 * MAZE_COLS * MAZE_ROWS; 
static Maze* hw3Maze = new Maze();
static GLint vertices[vertexArraySize];

void renderScene(void) {

	//Camera update.
	if (deltaMove)
		computePos(deltaMove);

	if (deltaAngle)
	{
		lx = sin(angle + deltaAngle);
		lz = -cos(angle + deltaAngle);

		angle += deltaAngle;
	}

	// Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto &teapot : teapots)
	{

		glDisable(GL_TEXTURE_2D);
		glColor3f(teapot.r, teapot.g, teapot.b);
		teapot.render();
	}

	//enable texturing
	glEnable(GL_TEXTURE_2D);

	// Reset transformations
	glLoadIdentity();
	// Set the camera
	gluLookAt(x, 0.5f, z,
		x + lx, 0.5f, z + lz,
		0.0f, 1.0f, 0.0f);

	//Draw axes at origin.
	//Draw x-Axis.
	glColor3f(1, 0, 0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 0.0, 0.0);

	//Draw y-Axis.
	glColor3f(0, 1, 0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 1.0, 0.0);

	//Draw z-Axis.
	glColor3f(0, 0, 1);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 1.0);
	glEnd();

	//Render maze.
	glColor3f(1, .5, 1);

	for (auto &i : hw3Maze->cellGrid)
	{
		for (auto &cell : i)
		{
			cell.render();
		}
	}

	

	glFlush();
	glutSwapBuffers();
}

void idleFunc(void)
{
	GLfloat rotationSpeed = 2.0f;
	for (auto &teapot : teapots)
	{
		teapot.angle += rotationSpeed * teapot.rotDirection;
	}

	glutPostRedisplay();
}

//=============================MAIN===================================


int main(int argc, char **argv) {

	//Seed rand()
	srand(time(NULL));	

	createVertexArray(vertices, hw3Maze);

	//Generate Teapots
	for (int i = 0; i < 10; ++i)
	{
		teapots.push_back(Teapot(rand() % MAZE_ROWS + 0.5, 0.5, rand() % MAZE_ROWS + 0.5));
	}
	
	
	// init GLUT and create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Maze");

	initTextures();

	std::cerr << "I noticed that movement speed varies from system to system, so you can adjust the modifiers at the top of hw3.cpp\n";

	// register callbacks
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(idleFunc);

	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(pressKey);
	glutSpecialUpFunc(releaseKey);

	// OpenGL init
	glEnable(GL_DEPTH_TEST);
	//glCullFace(GL_BACK);

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}