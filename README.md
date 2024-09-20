# 3DMaze

UMSL CS4410 - Computer Graphics (Final Project)

**Description:** Program that generates a unique 3D maze of customizeable size, then allows the user to walk around within the maze.

![](./maze-demo.gif 'Maze Demo GIF')

### Linux Build Instructions (Debian-based)

**Open the terminal and run these commands:**

`sudo apt-get install freeglut3*`

Navigate to the directory containing the cloned repository.

`g++ -o Maze hw3.cpp Teapot.cpp -lGL -lGLU -lglut`

`./Maze`
