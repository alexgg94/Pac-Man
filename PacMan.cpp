#include <iostream>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <stack>
#include <tuple>
#include <vector>
#include <GL/glut.h>
#include <thread>

using namespace std;

#define WIDTH 500
#define HEIGHT 500

enum Direction { LEFT, RIGHT, UP, DOWN, NONE };
enum ParticleType { PACMAN, ENEMY };
enum ParticleState { MOVE, QUIET };

class Particle {
    private:
        double particle_x;
        double particle_y;
        double velocity_x;
        double velocity_y;
        ParticleType particle_type;
        ParticleState particle_state;
        long time_remaining;

    public:
        Particle(ParticleType type, int x, int y)
        {
            particle_type = type;
            particle_x = x;
            particle_y = y;
            particle_state = ParticleState::QUIET;
        }

        void SetPosition(int x, int y)
        {
            particle_x = x;
            particle_y = y;
        }

        void InitMovement(int destination_x,int destination_y,int duration)
        {
            if(particle_type == ParticleType::PACMAN)
            {
                velocity_x = (destination_x - particle_x)/duration;
                velocity_y = (destination_y - particle_y)/duration;

                particle_state = ParticleState::MOVE;
                time_remaining = duration;
            }
        }

        void InitMovement()
        {
            if(particle_type == ParticleType::ENEMY)
            {
                while(true)
                {

                }
            }
        }

        void Integrate(long t)
        {
            if(particle_state == ParticleState::MOVE && t < time_remaining)
            {
                particle_x = particle_x + velocity_x * t;
                particle_y = particle_y + velocity_y * t;
                time_remaining -= t;
            }

            else if(particle_state == ParticleState::MOVE && t >= time_remaining)
            {
                particle_x = particle_x + velocity_x * time_remaining;
                particle_y = particle_y + velocity_y * time_remaining;
                particle_state = ParticleState::QUIET;
            }
        }

        void Draw()
        {
            glColor3f(1, 1, 1);

            if(particle_type == ParticleType::PACMAN)
            {
                glColor3f(1, 1, 0);
            }

            glBegin(GL_QUADS);
            glVertex2i(particle_x-6,particle_y-6);
            glVertex2i(particle_x+6,particle_y-6);
            glVertex2i(particle_x+6,particle_y+6);
            glVertex2i(particle_x-6,particle_y+6);
            glEnd();
        }
};

class Coordinate {
    private:
        double coordinate_row;
        double coordinate_col;
    public:
        Coordinate()
        {

        }

        Coordinate(double row, double col)
        {
            coordinate_row = row;
            coordinate_col = col;
        }

        double GetRow()
        {
            return coordinate_row;
        }

        double GetCol()
        {
            return coordinate_col;
        }
};

class Node {
    private:
        Coordinate node_coordinate;
        Direction node_origin;
    public:
        Node(Coordinate coordinate, Direction origin)
        {
            node_coordinate = coordinate;
            node_origin = origin;
        }

        int GetRow()
        {
            return (int)node_coordinate.GetRow();
        }

        int GetCol()
        {
            return (int)node_coordinate.GetCol();
        }

        Direction GetOrigin()
        {
            return node_origin;
        }
};

class Maze {
    private:
        int maze_rows;
        int maze_cols;
        char** left_maze;
        char** maze;
        stack<Node> expanded;
        
        /*
            The initialization of the map as a matrix. Horizontal walls are represented by
            "-", vertical walls by "|" and the union between them by "+".
            Also, for the DPS algorithm, all the available unvisited cells are represented
            by "_".
        */
        void InitializeMaze()
        {
            left_maze = new char*[maze_rows];
            maze = new char*[maze_rows];

            for(int row = 0; row < maze_rows; row++)
            {
                maze[row] = new char[maze_cols*2];
                left_maze[row] = new char[maze_cols];
                fill_n(left_maze[row], maze_cols, '_');

                for(int col = 0; col < maze_cols; col++)
                {
                    if(row % 2 == 0)
                    {
                        if(col % 2 == 0)
                        {
                            left_maze[row][col] = '+';
                        }

                        else
                        {
                            left_maze[row][col] = '-';
                        }
                    }

                    else
                    {
                        if(col % 2 == 0)
                        {
                            left_maze[row][col] = '|';
                        }
                    }
                }
            }
        }

        void GenerateMiddleRoom()
        {
            left_maze[maze_rows/2 - 1][maze_cols -1] = ' ';
            left_maze[maze_rows/2 - 1][maze_cols -2] = ' ';
            left_maze[maze_rows/2 - 1][maze_cols -3] = ' ';
            left_maze[maze_rows/2 - 1][maze_cols -4] = ' ';
            left_maze[maze_rows/2][maze_cols -1] = ' ';
            left_maze[maze_rows/2][maze_cols -2] = ' ';
            left_maze[maze_rows/2][maze_cols -3] = ' ';
            left_maze[maze_rows/2][maze_cols -4] = ' ';
            left_maze[maze_rows/2 + 1][maze_cols -1] = ' ';
            left_maze[maze_rows/2 + 1][maze_cols -2] = ' ';
            left_maze[maze_rows/2 + 1][maze_cols -3] = ' ';
            left_maze[maze_rows/2 + 1][maze_cols -4] = ' ';
            left_maze[maze_rows/2 + 1][maze_cols -5] = ' ';
            left_maze[maze_rows/2 + 2][maze_cols -1] = ' ';
            left_maze[maze_rows/2 + 2][maze_cols -2] = ' ';
            left_maze[maze_rows/2 + 2][maze_cols -3] = ' ';
            left_maze[maze_rows/2 + 2][maze_cols -4] = ' ';
            left_maze[maze_rows/2 + 3][maze_cols -1] = ' ';
            left_maze[maze_rows/2 + 3][maze_cols -2] = ' ';
            left_maze[maze_rows/2 + 3][maze_cols -3] = ' ';
            left_maze[maze_rows/2 + 3][maze_cols -4] = ' ';
        }

        /*
            For a cell, this function will return any adjacent cells with the direction
            to follow from the current cell to arrive to them.
        */
        void GetAdjacentsCells(int current_row, int current_col)
        {
            vector<Node> adjacentNodes;

            if(current_row + 2 < maze_rows -1)
            {
                if(left_maze[current_row + 2][current_col] == '_')
                {
                    adjacentNodes.push_back(Node(Coordinate(current_row + 2, current_col), Direction::UP));
                }
            }

            if(current_row - 2 > 0)
            {
                if(left_maze[current_row - 2][current_col] == '_')
                {
                    adjacentNodes.push_back(Node(Coordinate(current_row - 2, current_col), Direction::DOWN));
                }
            }

            if(current_col + 2  < maze_cols -1)
            {
                if(left_maze[current_row][current_col + 2] == '_')
                {
                    adjacentNodes.push_back(Node(Coordinate(current_row, current_col + 2), Direction::LEFT));
                }
            }

            if(current_col - 2  > 0)
            {
                if(left_maze[current_row][current_col - 2] == '_')
                {
                    adjacentNodes.push_back(Node(Coordinate(current_row, current_col - 2), Direction::RIGHT));
                }
            }

            random_shuffle (adjacentNodes.begin(), adjacentNodes.end());

            for (vector<Node>::iterator it=adjacentNodes.begin(); it!=adjacentNodes.end(); ++it)
            {
                expanded.push(*it);
            }
        }

        /*
            The Maze is generated with a backtracking DFS(Deep First Search) algorithm.
            The first node to expand will be the exiting cell from the middle room.
            For an expanded cell, if this contains an "_" (unvisited), this cell will be
            marked as visited " " and the wall from its origin will be broken (set to " ").
        */
        void DFS()
        {
            if(!expanded.empty())
            {
                Node current_node = expanded.top();
                expanded.pop();

                if(left_maze[current_node.GetRow()][current_node.GetCol()] == '_')
                {
                    left_maze[current_node.GetRow()][current_node.GetCol()] = '.';

                    if(current_node.GetOrigin() == Direction::UP)
                    {
                        left_maze[current_node.GetRow() - 1][current_node.GetCol()] = '.';
                    }

                    else if(current_node.GetOrigin() == Direction::DOWN)
                    {
                        left_maze[current_node.GetRow() + 1][current_node.GetCol()] = '.';
                    }

                    else if(current_node.GetOrigin() == Direction::LEFT)
                    {
                        left_maze[current_node.GetRow()][current_node.GetCol() - 1] = '.';
                    }

                    else if(current_node.GetOrigin() == Direction::RIGHT)
                    {
                        left_maze[current_node.GetRow()][current_node.GetCol() + 1] = '.';
                    }
                }

                GetAdjacentsCells(current_node.GetRow(), current_node.GetCol());
                DFS();
            }
        }

        /*
            This method will break some random walls down.
        */
        void BreakWallsDown()
        {
            vector<Node> walls;
            int wall_position;
            
            for(int row = 1; row < maze_rows -1; row++)
            {
                for(int col = 1; col < maze_cols -1; col++)
                {
                    if(left_maze[row][col] == '-' || left_maze[row][col] == '|')
                    {
                        walls.push_back(Node(Coordinate(row, col), Direction::NONE));
                    }
                }

                if(walls.size() > 0)
                {
                    wall_position = rand() % walls.size();
                    left_maze[walls[wall_position].GetRow()][walls[wall_position].GetCol()] = '.';
                }

                walls.clear();
            }
            
            left_maze[1][maze_cols-1] = '.';
            left_maze[maze_rows-2][maze_cols-1] = '.';
        }

        void UnifyMazes()
        {
            for(int i = 0; i < maze_rows; i++)
            {
                for(int j = maze_cols; j > 0; j--)
                {
                    maze[i][maze_cols-j + maze_cols] = left_maze[i][j-2];
                    maze[i][maze_cols-j] = left_maze[i][maze_cols-j];
                }
            }
        }

        void PrintMaze()
        {
            for(int i = 0; i < maze_rows; i++)
            {
                for(int j = 0; j < maze_cols*2 - 1; j++)
                    cout << maze[i][j];
                cout << endl;
            }

            cout << endl;
        }

    public:
        Maze(int rows, int cols)
        {
            maze_rows = rows*2 + 1;
            maze_cols = cols*2 + 1;

            InitializeMaze();
            GenerateMiddleRoom();
            expanded.push(Node(Coordinate(1, 1), Direction::NONE));
            DFS();
            BreakWallsDown();
            UnifyMazes();
            //PrintMaze();
        }

        char** GetMaze()
        {
            return maze;
        }
};

int global_rows;
int global_cols;
int keyflag = 0;
char** maze;
long last_t = 0;
vector<Particle> Particles;

Coordinate GetCenterCoordinate(Coordinate coordinate1, Coordinate coordinate2, Coordinate coordinate3, Coordinate coordinate4)
{
    return Coordinate(coordinate1.GetRow() + (coordinate2.GetRow() - coordinate1.GetRow()) / 2, coordinate1.GetCol() + (coordinate4.GetCol() - coordinate1.GetCol()) / 2);
}

Coordinate CoordinateToScreen(int row, int col)
{
    return Coordinate(col*WIDTH/(global_cols * 4 + 1), row*HEIGHT/(global_rows * 2 + 1));
}

void display()
{
    glClearColor(0.0,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    for(int row = 0; row < global_rows * 2 + 1; row++)
    {
        for(int col = 0; col < global_cols * 4 + 1; col++)
        {
            Coordinate coordinate1 = CoordinateToScreen(row, col);
            Coordinate coordinate2 = CoordinateToScreen(row, col + 1);
            Coordinate coordinate3 = CoordinateToScreen(row + 1, col + 1);
            Coordinate coordinate4 = CoordinateToScreen(row + 1, col);
            Coordinate centerCoordinate = GetCenterCoordinate(coordinate1, coordinate2, coordinate3, coordinate4);

            if(maze[row][col] == '+' || maze[row][col] == '|' || maze[row][col] == '-' || 
                maze[row][col] == ' ' || maze[row][col] == '.')
            {
                glColor3f(0, 0, 1);

                if(maze[row][col] == ' ' || maze[row][col] == '.')
                {
                    glColor3f(1, 1, 1);
                }
                
                glBegin(GL_QUADS);

                glVertex2i(coordinate1.GetRow(), coordinate1.GetCol()); 
                glVertex2i(coordinate2.GetRow(), coordinate2.GetCol()); 
                glVertex2i(coordinate3.GetRow(), coordinate3.GetCol()); 
                glVertex2i(coordinate4.GetRow(), coordinate4.GetCol()); 

                glEnd();
            }

            if(maze[row][col] == '.')
            {
                glColor3f(0, 1, 0);
                glBegin(GL_QUADS);

                glVertex2i(centerCoordinate.GetRow() - 1, centerCoordinate.GetCol() - 1); 
                glVertex2i(centerCoordinate.GetRow() - 1, centerCoordinate.GetCol() + 1); 
                glVertex2i(centerCoordinate.GetRow() + 1, centerCoordinate.GetCol() + 1); 
                glVertex2i(centerCoordinate.GetRow() + 1, centerCoordinate.GetCol() - 1); 

                glEnd();
            }

            else if(maze[row][col] == 'p' || maze[row][col] == 'e')
            {
                glColor3f(0, 0, 0);

                if(maze[row][col] == 'p')
                {
                    glColor3f(1, 1, 0);
                }

                glBegin(GL_QUADS);

                glVertex2i(coordinate1.GetRow(), coordinate1.GetCol()); 
                glVertex2i(coordinate2.GetRow(), coordinate2.GetCol()); 
                glVertex2i(coordinate3.GetRow(), coordinate3.GetCol()); 
                glVertex2i(coordinate4.GetRow(), coordinate4.GetCol()); 

                glEnd();
            }
        }
    }
    
    glutSwapBuffers();
}

void keyboard(unsigned char c,int x,int y)
{
  if(keyflag==0)
    keyflag=1;
  else
    keyflag=0;

  glutPostRedisplay();
};

void idle()
{
    /*
  long t;

  t = glutGet(GLUT_ELAPSED_TIME); 

  if(last_t == 0)
    last_t = t;
  else
    {
      square.integrate(t-last_t);
      last_t = t;
    }

  glutPostRedisplay();
  */
}

void InitializeParticles()
{
    int number_of_enemies = global_rows * global_cols / 10;

    if( number_of_enemies == 0 )
    {
        number_of_enemies ++;
    }

    Particles.push_back(Particle(ParticleType::PACMAN, 1, 1));
    maze[1][1] = 'p';

    while(number_of_enemies > 0)
    {
        Particles.push_back(Particle(ParticleType::ENEMY, global_rows, global_cols));
        number_of_enemies --;
    }

    maze[global_rows][global_cols*2] = 'e';
}

int main(int argc,char *argv[])
{
    srand (time(NULL));

    global_rows = stoi(argv[1]);
    global_cols = stoi(argv[2]);

    maze = Maze(global_rows, global_cols).GetMaze();
    InitializeParticles();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Maze");

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    //glutIdleFunc(idle);

    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, WIDTH-1, 0, HEIGHT-1);

    glutMainLoop();
}