#include <iostream>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <stack>
#include <tuple>
#include <vector>
#include <GL/glut.h>

#define WIDTH 500
#define HEIGHT 500

int global_rows;
int global_cols;
int keyflag = 0;

using namespace std;

enum Direction { LEFT, RIGHT, UP, DOWN, NONE };

class Node {
    private:
        int node_row;
        int node_col;
        Direction node_origin;
    public:
        Node(int row, int col, Direction origin)
        {
            node_row = row;
            node_col = col;
            node_origin = origin;
        }

        int GetRow()
        {
            return node_row;
        }

        int GetCol()
        {
            return node_col;
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

        void GetAdjacentsCells(int current_row, int current_col)
        {
            vector<Node> adjacentNodes;

            if(current_row + 2 < maze_rows -1)
            {
                if(left_maze[current_row + 2][current_col] == '_')
                {
                    adjacentNodes.push_back(Node(current_row + 2, current_col, Direction::UP));
                }
            }

            if(current_row - 2 > 0)
            {
                if(left_maze[current_row - 2][current_col] == '_')
                {
                    adjacentNodes.push_back(Node(current_row - 2, current_col, Direction::DOWN));
                }
            }

            if(current_col + 2  < maze_cols -1)
            {
                if(left_maze[current_row][current_col + 2] == '_')
                {
                    adjacentNodes.push_back(Node(current_row, current_col + 2, Direction::LEFT));
                }
            }

            if(current_col - 2  > 0)
            {
                if(left_maze[current_row][current_col - 2] == '_')
                {
                    adjacentNodes.push_back(Node(current_row, current_col - 2, Direction::RIGHT));
                }
            }

            random_shuffle (adjacentNodes.begin(), adjacentNodes.end());

            for (vector<Node>::iterator it=adjacentNodes.begin(); it!=adjacentNodes.end(); ++it)
            {
                expanded.push(*it);
            }
        }

        void DFS()
        {
            if(!expanded.empty())
            {
                Node current_node = expanded.top();
                expanded.pop();

                if(left_maze[current_node.GetRow()][current_node.GetCol()] == '_')
                {
                    left_maze[current_node.GetRow()][current_node.GetCol()] = ' ';

                    if(current_node.GetOrigin() == Direction::UP)
                    {
                        left_maze[current_node.GetRow() - 1][current_node.GetCol()] = ' ';
                    }

                    else if(current_node.GetOrigin() == Direction::DOWN)
                    {
                        left_maze[current_node.GetRow() + 1][current_node.GetCol()] = ' ';
                    }

                    else if(current_node.GetOrigin() == Direction::LEFT)
                    {
                        left_maze[current_node.GetRow()][current_node.GetCol() - 1] = ' ';
                    }

                    else if(current_node.GetOrigin() == Direction::RIGHT)
                    {
                        left_maze[current_node.GetRow()][current_node.GetCol() + 1] = ' ';
                    }
                }

                GetAdjacentsCells(current_node.GetRow(), current_node.GetCol());
                DFS();
            }
        }

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
                        walls.push_back(Node(row, col, Direction::NONE));
                    }
                }

                if(walls.size() > 0)
                {
                    wall_position = rand() % walls.size();
                    left_maze[walls[wall_position].GetRow()][walls[wall_position].GetCol()] = ' ';
                }

                walls.clear();
            }
            
            left_maze[1][maze_cols-1] = ' ';
            left_maze[maze_rows-2][maze_cols-1] = ' ';
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
            expanded.push(Node(maze_rows/2 + 1, maze_cols -6, Direction::NONE));
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

void display()
{
    char** maze = Maze(global_rows, global_cols).GetMaze();

    glClearColor(0.0,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    for(int row = 0; row < global_rows * 2 + 1; row++)
    {
        for(int col = 0; col < global_cols * 4 + 1; col++)
        {
            if(maze[row][col] == '+' || maze[row][col] == '|' || maze[row][col] == '-')
            {
                glColor3f(0.2,0.2,0.2);
            }

            else
            {
                glColor3f(0.8,0.8,0.8);
            }

            glBegin(GL_QUADS);

            glVertex2i(col*WIDTH/(global_cols * 4 + 1), row*HEIGHT/(global_rows * 2 + 1)); 
            glVertex2i((col+1)*WIDTH/(global_cols * 4 + 1), row*HEIGHT/(global_rows * 2 + 1)); 
            glVertex2i((col+1)*WIDTH/(global_cols * 4 + 1), (row+1)*HEIGHT/(global_rows * 2 + 1)); 
            glVertex2i(col*WIDTH/(global_cols * 4 + 1), (row+1)*HEIGHT/(global_rows * 2 + 1)); 

            glEnd();
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

int main(int argc,char *argv[])
{
    srand (time(NULL));

    global_rows = stoi(argv[1]);
    global_cols = stoi(argv[2]);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Maze");

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, WIDTH-1, 0, HEIGHT-1);

    glutMainLoop();
}