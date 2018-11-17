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
#include <chrono>

using namespace std;

#define WIDTH 500
#define HEIGHT 500
#define MOVEMENT_DURATION 500

int global_rows;
int global_cols;
int keyflag = 0;
char** maze;
long last_t = 0;
int remaining_pellets;

class Particle;
vector<Particle> Particles;

enum Direction { LEFT, RIGHT, UP, DOWN, NONE };
enum ParticleType { PACMAN, ENEMY };
enum ParticleState { MOVE, QUIET };

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

Coordinate GetCenterCoordinate(Coordinate coordinate1, Coordinate coordinate2, Coordinate coordinate3, Coordinate coordinate4)
{
    return Coordinate(coordinate1.GetRow() + (coordinate2.GetRow() - coordinate1.GetRow()) / 2, coordinate1.GetCol() + (coordinate4.GetCol() - coordinate1.GetCol()) / 2);
}

Coordinate CoordinateToScreen(int row, int col)
{
    return Coordinate(col*WIDTH/(global_cols * 4 + 1), row*HEIGHT/(global_rows * 2 + 1));
}

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

class Particle {
    private:
        int particle_index;
        ParticleType particle_type;
        long time_remaining;

        Node GetRandomAdjacentCell(int current_row, int current_col, vector<Node> visited)
        {
            auto is_visited = [&](int row, int col) 
            {
                return std::find_if(begin(visited), end(visited), [&](Node &node) 
                { 
                    return node.GetRow() == row && node.GetCol() == col; 
                }) != end(visited);
            };

            vector<Node> adjacentNodes;

            if(current_row + 1 < global_rows*2)
            {
                if(maze[current_row + 1][current_col] == ' ' || maze[current_row + 1][current_col] == '.')
                { 
                    if(!is_visited(current_row + 1, current_col))
                    {
                        adjacentNodes.push_back(Node(Coordinate(current_row + 1, current_col), Direction::UP));
                    }
                }
            }

            if(current_row - 1 > 0)
            {
                if(maze[current_row - 1][current_col] == ' ' || maze[current_row - 1][current_col] == '.')
                {
                    if(!is_visited(current_row - 1, current_col))
                    {
                        adjacentNodes.push_back(Node(Coordinate(current_row - 1, current_col), Direction::DOWN));
                    }
                }
            }

            if(current_col + 1  < global_cols*4)
            {
                if(maze[current_row][current_col + 1] == ' ' || maze[current_row][current_col + 1] == '.')
                {
                    if(!is_visited(current_row, current_col + 1))
                    {
                        adjacentNodes.push_back(Node(Coordinate(current_row, current_col + 1), Direction::LEFT));
                    }
                }
            }

            if(current_col - 1  > 0)
            {
                if(maze[current_row][current_col - 1] == ' ' || maze[current_row][current_col - 1] == '.')
                {
                    if(!is_visited(current_row, current_col - 1))
                    {
                        adjacentNodes.push_back(Node(Coordinate(current_row, current_col - 1), Direction::RIGHT));
                    }
                }
            }

            if(adjacentNodes.empty())
            {
                return Node(Coordinate(-1, -1), Direction::NONE);
            }

            else
            {
                random_shuffle (adjacentNodes.begin(), adjacentNodes.end());
                visited.push_back(adjacentNodes[0]);
                return adjacentNodes[0];
            }
        }

        void InitMovementEnemy(int initial_x, int initial_y, vector<Node> visited, int particle_index)
        {
            int current_x = initial_x;
            int current_y = initial_y;
            int current_cell_content = ' ';

            std::this_thread::sleep_for(std::chrono::seconds(3));
            while(true)
            {
                if(Particles[particle_index].particle_state == ParticleState::QUIET)
                {
                    visited.push_back(Node(Coordinate(current_x, current_y), Direction::NONE));
                    Node random_adjacent_node = GetRandomAdjacentCell(current_x, current_y, visited);

                    if(random_adjacent_node.GetRow() == -1 && random_adjacent_node.GetCol() == -1)
                    {
                        visited.clear();
                        visited.push_back(Node(Coordinate(current_x, current_y), Direction::NONE));
                        random_adjacent_node = GetRandomAdjacentCell(current_x, current_y, visited);
                    }

                    if(current_cell_content != 'e')
                    {
                        maze[current_x][current_y] = current_cell_content;
                    }               

                    Coordinate originGraphicalCoordinate1 = CoordinateToScreen(current_x, current_y);
                    Coordinate originGraphicalCoordinate2 = CoordinateToScreen(current_x, current_y + 1);
                    Coordinate originGraphicalCoordinate3 = CoordinateToScreen(current_x + 1, current_y + 1);
                    Coordinate originGraphicalCoordinate4 = CoordinateToScreen(current_x + 1, current_y);
                    Coordinate originCenterGraphicalCoordinate = GetCenterCoordinate(originGraphicalCoordinate1, originGraphicalCoordinate2, originGraphicalCoordinate3, originGraphicalCoordinate4);

                    Coordinate destinationGraphicalCoordinate1 = CoordinateToScreen(random_adjacent_node.GetRow(), random_adjacent_node.GetCol());
                    Coordinate destinationGraphicalCoordinate2 = CoordinateToScreen(random_adjacent_node.GetRow(), random_adjacent_node.GetCol() + 1);
                    Coordinate destinationGraphicalCoordinate3 = CoordinateToScreen(random_adjacent_node.GetRow() + 1, random_adjacent_node.GetCol() + 1);
                    Coordinate destinationGraphicalCoordinate4 = CoordinateToScreen(random_adjacent_node.GetRow() + 1, random_adjacent_node.GetCol());
                    Coordinate destinationCenterGraphicalCoordinate = GetCenterCoordinate(destinationGraphicalCoordinate1, destinationGraphicalCoordinate2, destinationGraphicalCoordinate3, destinationGraphicalCoordinate4);

                    SetPosition(originCenterGraphicalCoordinate.GetRow(), originCenterGraphicalCoordinate.GetCol(), particle_index);
                    InitMovement(originCenterGraphicalCoordinate.GetRow(), originCenterGraphicalCoordinate.GetCol(), destinationCenterGraphicalCoordinate.GetRow(), destinationCenterGraphicalCoordinate.GetCol(), particle_index);

                    current_x = random_adjacent_node.GetRow();
                    current_y = random_adjacent_node.GetCol();
                    current_cell_content = maze[current_x][current_y];

                    if(current_x == Particles[0].particle_x && current_y == Particles[0].particle_y)
                    {
                        cout << "YOU LOSE!" << endl;
                        exit(0);
                    }
                }
            }
        }

    public:
        double graphical_x;
        double graphical_y;
        ParticleState particle_state;
        double velocity_x;
        double velocity_y;
        int particle_x;
        int particle_y;

        Particle(ParticleType type, int x, int y, int index)
        {
            particle_type = type;
            particle_x = x;
            particle_y = y;
            particle_index = index;

            Coordinate coordinate1 = CoordinateToScreen(particle_x, particle_y);
            Coordinate coordinate2 = CoordinateToScreen(particle_x, particle_y + 1);
            Coordinate coordinate3 = CoordinateToScreen(particle_x + 1, particle_y + 1);
            Coordinate coordinate4 = CoordinateToScreen(particle_x + 1, particle_y);
            Coordinate centerCoordinate = GetCenterCoordinate(coordinate1, coordinate2, coordinate3, coordinate4);

            graphical_x = centerCoordinate.GetRow();
            graphical_y = centerCoordinate.GetCol();

            particle_state = ParticleState::QUIET;
            vector<Node> visited;
            if(particle_type == ParticleType::ENEMY)
            {
                std::thread t(&Particle::InitMovementEnemy, this, particle_x, particle_y, visited, particle_index);
                t.detach();
            }
        }

        void SetPosition(int x, int y, int index)
        {
            Particles[index].graphical_x = x;
            Particles[index].graphical_y = y;
        }

        void InitMovement(double origin_x, double origin_y, double destination_x, double destination_y, int index)
        {
            Particles[index].velocity_x = (destination_x - origin_x) / MOVEMENT_DURATION;
            Particles[index].velocity_y = (destination_y - origin_y) / MOVEMENT_DURATION;

            Particles[index].particle_state = ParticleState::MOVE;
            Particles[index].time_remaining = MOVEMENT_DURATION;
        }

        void Integrate(long t)
        {
            if(particle_state == ParticleState::MOVE && t < time_remaining)
            {
                graphical_x = graphical_x + velocity_x * t;
                graphical_y = graphical_y + velocity_y * t;
                time_remaining -= t;
            }

            else if(particle_state == ParticleState::MOVE && t >= time_remaining)
            {
                graphical_x = graphical_x + velocity_x * time_remaining;
                graphical_y = graphical_y + velocity_y * time_remaining;
                particle_state = ParticleState::QUIET;
            }
        }

        void Draw()
        {
            glColor3f(0, 0, 0);

            if(particle_type == ParticleType::PACMAN)
            {
                glColor3f(1, 1, 0);
            }

            glBegin(GL_QUADS);
            glVertex2i(graphical_x - WIDTH/(global_cols * 4 + 1)/2,graphical_y - HEIGHT/(global_rows * 2 + 1)/2);
            glVertex2i(graphical_x + WIDTH/(global_cols * 4 + 1)/2,graphical_y - HEIGHT/(global_rows * 2 + 1)/2);
            glVertex2i(graphical_x + WIDTH/(global_cols * 4 + 1)/2 ,graphical_y + HEIGHT/(global_rows * 2 + 1)/2);
            glVertex2i(graphical_x - WIDTH/(global_cols * 4 + 1)/2,graphical_y + HEIGHT/(global_rows * 2 + 1)/2);
            glEnd();
        }
};

class Maze {
    private:
        int maze_rows;
        int maze_cols;
        char** left_maze;
        char** maze;
        stack<Node> expanded;
        int num_pellets = 0;
        
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

        void CheckNumberOfPellets()
        {
            for(int i = 0; i < maze_rows; i++)
            {
                for(int j = 0; j < maze_cols*2 - 1; j++)
                    if(maze[i][j] == '.')
                        num_pellets ++;
            }
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
            CheckNumberOfPellets();
            //PrintMaze();
        }

        char** GetMaze()
        {
            return maze;
        }

        int GetNumberOfPellets()
        {
            return num_pellets;
        }
};

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
        }
    }
    
    Particles[0].Draw();
    Particles[1].Draw();
    Particles[2].Draw();
    Particles[3].Draw();
    Particles[4].Draw();

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
  long t;

  t = glutGet(GLUT_ELAPSED_TIME); 

  if(last_t == 0)
    last_t = t;
  else
    {
      Particles[0].Integrate(t-last_t);
      Particles[1].Integrate(t-last_t);
      Particles[2].Integrate(t-last_t);
      Particles[3].Integrate(t-last_t);
      Particles[4].Integrate(t-last_t);

      last_t = t;
    }

  glutPostRedisplay();
}

int GetTotalPellets()
{

}

void InitializeParticles()
{
    Particles.push_back(Particle(ParticleType::PACMAN, 1, 1, 0));
    Particles.push_back(Particle(ParticleType::ENEMY, global_rows, global_cols*2, 1));
    Particles.push_back(Particle(ParticleType::ENEMY, global_rows, global_cols*2, 2));
    Particles.push_back(Particle(ParticleType::ENEMY, global_rows, global_cols*2, 3));
    Particles.push_back(Particle(ParticleType::ENEMY, global_rows, global_cols*2, 4));
}

void SpecialKey(int key, int x, int y){
    switch (key) {
        case GLUT_KEY_UP:
            if(maze[Particles[0].particle_x + 1][Particles[0].particle_y] == ' ' || maze[Particles[0].particle_x + 1][Particles[0].particle_y] == '.' || 
            maze[Particles[0].particle_x + 1][Particles[0].particle_y] == 'e')
            {
                if(maze[Particles[0].particle_x + 1][Particles[0].particle_y] == '.')
                {
                    maze[Particles[0].particle_x + 1][Particles[0].particle_y] = ' ';

                    if(--remaining_pellets == 0)
                    {
                        cout << "YOU WON!" << endl;
                        exit(0);
                    }
                }

                Coordinate originGraphicalCoordinate1 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y);
                Coordinate originGraphicalCoordinate2 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y + 1);
                Coordinate originGraphicalCoordinate3 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y + 1);
                Coordinate originGraphicalCoordinate4 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y);
                Coordinate originCenterGraphicalCoordinate = GetCenterCoordinate(originGraphicalCoordinate1, originGraphicalCoordinate2, originGraphicalCoordinate3, originGraphicalCoordinate4);

                Coordinate destinationGraphicalCoordinate1 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y);
                Coordinate destinationGraphicalCoordinate2 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y + 1);
                Coordinate destinationGraphicalCoordinate3 = CoordinateToScreen(Particles[0].particle_x + 2, Particles[0].particle_y + 1);
                Coordinate destinationGraphicalCoordinate4 = CoordinateToScreen(Particles[0].particle_x + 2, Particles[0].particle_y);
                Coordinate destinationCenterGraphicalCoordinate = GetCenterCoordinate(destinationGraphicalCoordinate1, destinationGraphicalCoordinate2, destinationGraphicalCoordinate3, destinationGraphicalCoordinate4);

                Particles[0].particle_x ++;
                Particles[0].SetPosition(originCenterGraphicalCoordinate.GetRow(), originCenterGraphicalCoordinate.GetCol(), 0);
                Particles[0].InitMovement(originCenterGraphicalCoordinate.GetRow(), originCenterGraphicalCoordinate.GetCol(), destinationCenterGraphicalCoordinate.GetRow(), destinationCenterGraphicalCoordinate.GetCol(), 0);
            }
            break;
        case GLUT_KEY_DOWN:
            if(maze[Particles[0].particle_x - 1][Particles[0].particle_y] == ' ' || maze[Particles[0].particle_x - 1][Particles[0].particle_y] == '.' ||
            maze[Particles[0].particle_x - 1][Particles[0].particle_y] == 'e')
            {
                if(maze[Particles[0].particle_x - 1][Particles[0].particle_y] == '.')
                {
                    maze[Particles[0].particle_x - 1][Particles[0].particle_y] = ' ';

                    if(--remaining_pellets == 0)
                    {
                        cout << "YOU WON!" << endl;
                        exit(0);
                    }
                }

                Coordinate originGraphicalCoordinate1 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y);
                Coordinate originGraphicalCoordinate2 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y + 1);
                Coordinate originGraphicalCoordinate3 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y + 1);
                Coordinate originGraphicalCoordinate4 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y);
                Coordinate originCenterGraphicalCoordinate = GetCenterCoordinate(originGraphicalCoordinate1, originGraphicalCoordinate2, originGraphicalCoordinate3, originGraphicalCoordinate4);

                Coordinate destinationGraphicalCoordinate1 = CoordinateToScreen(Particles[0].particle_x -1, Particles[0].particle_y);
                Coordinate destinationGraphicalCoordinate2 = CoordinateToScreen(Particles[0].particle_x - 1, Particles[0].particle_y + 1);
                Coordinate destinationGraphicalCoordinate3 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y + 1);
                Coordinate destinationGraphicalCoordinate4 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y);
                Coordinate destinationCenterGraphicalCoordinate = GetCenterCoordinate(destinationGraphicalCoordinate1, destinationGraphicalCoordinate2, destinationGraphicalCoordinate3, destinationGraphicalCoordinate4);

                Particles[0].particle_x --;
                Particles[0].SetPosition(originCenterGraphicalCoordinate.GetRow(), originCenterGraphicalCoordinate.GetCol(), 0);
                Particles[0].InitMovement(originCenterGraphicalCoordinate.GetRow(), originCenterGraphicalCoordinate.GetCol(), destinationCenterGraphicalCoordinate.GetRow(), destinationCenterGraphicalCoordinate.GetCol(), 0);
            }
            break;
        case GLUT_KEY_LEFT:
            if(maze[Particles[0].particle_x][Particles[0].particle_y - 1] == ' ' || maze[Particles[0].particle_x][Particles[0].particle_y - 1] == '.' || 
            maze[Particles[0].particle_x][Particles[0].particle_y - 1] == 'e')
            {
                if(maze[Particles[0].particle_x][Particles[0].particle_y - 1] == '.')
                {
                    maze[Particles[0].particle_x][Particles[0].particle_y - 1] = ' ';

                    if(--remaining_pellets == 0)
                    {
                        cout << "YOU WON!" << endl;
                        exit(0);
                    }
                }

                Coordinate originGraphicalCoordinate1 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y);
                Coordinate originGraphicalCoordinate2 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y + 1);
                Coordinate originGraphicalCoordinate3 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y + 1);
                Coordinate originGraphicalCoordinate4 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y);
                Coordinate originCenterGraphicalCoordinate = GetCenterCoordinate(originGraphicalCoordinate1, originGraphicalCoordinate2, originGraphicalCoordinate3, originGraphicalCoordinate4);

                Coordinate destinationGraphicalCoordinate1 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y - 1);
                Coordinate destinationGraphicalCoordinate2 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y);
                Coordinate destinationGraphicalCoordinate3 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y);
                Coordinate destinationGraphicalCoordinate4 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y - 1);
                Coordinate destinationCenterGraphicalCoordinate = GetCenterCoordinate(destinationGraphicalCoordinate1, destinationGraphicalCoordinate2, destinationGraphicalCoordinate3, destinationGraphicalCoordinate4);

                Particles[0].particle_y --;
                Particles[0].SetPosition(originCenterGraphicalCoordinate.GetRow(), originCenterGraphicalCoordinate.GetCol(), 0);
                Particles[0].InitMovement(originCenterGraphicalCoordinate.GetRow(), originCenterGraphicalCoordinate.GetCol(), destinationCenterGraphicalCoordinate.GetRow(), destinationCenterGraphicalCoordinate.GetCol(), 0);
            }
            break;
        case GLUT_KEY_RIGHT:
            if(maze[Particles[0].particle_x][Particles[0].particle_y + 1] == ' ' || maze[Particles[0].particle_x][Particles[0].particle_y + 1] == '.'
            || maze[Particles[0].particle_x][Particles[0].particle_y + 1] == 'e')
            {
                if(maze[Particles[0].particle_x][Particles[0].particle_y + 1] == '.')
                {
                    maze[Particles[0].particle_x][Particles[0].particle_y + 1] = ' ';

                    if(--remaining_pellets == 0)
                    {
                        cout << "YOU WON!" << endl;
                        exit(0);
                    }
                }

                Coordinate originGraphicalCoordinate1 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y);
                Coordinate originGraphicalCoordinate2 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y + 1);
                Coordinate originGraphicalCoordinate3 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y + 1);
                Coordinate originGraphicalCoordinate4 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y);
                Coordinate originCenterGraphicalCoordinate = GetCenterCoordinate(originGraphicalCoordinate1, originGraphicalCoordinate2, originGraphicalCoordinate3, originGraphicalCoordinate4);

                Coordinate destinationGraphicalCoordinate1 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y + 1);
                Coordinate destinationGraphicalCoordinate2 = CoordinateToScreen(Particles[0].particle_x, Particles[0].particle_y + 2);
                Coordinate destinationGraphicalCoordinate3 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y + 2);
                Coordinate destinationGraphicalCoordinate4 = CoordinateToScreen(Particles[0].particle_x + 1, Particles[0].particle_y + 1);
                Coordinate destinationCenterGraphicalCoordinate = GetCenterCoordinate(destinationGraphicalCoordinate1, destinationGraphicalCoordinate2, destinationGraphicalCoordinate3, destinationGraphicalCoordinate4);

                Particles[0].particle_y ++;
                Particles[0].SetPosition(originCenterGraphicalCoordinate.GetRow(), originCenterGraphicalCoordinate.GetCol(), 0);
                Particles[0].InitMovement(originCenterGraphicalCoordinate.GetRow(), originCenterGraphicalCoordinate.GetCol(), destinationCenterGraphicalCoordinate.GetRow(), destinationCenterGraphicalCoordinate.GetCol(), 0);
            }
            break;
    }
}

int main(int argc,char *argv[])
{
    srand (time(NULL));

    global_rows = stoi(argv[1]);
    global_cols = stoi(argv[2]);

    Maze m = Maze(global_rows, global_cols);
    maze = m.GetMaze();
    remaining_pellets = m.GetNumberOfPellets();

    InitializeParticles();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Maze");
    glutSpecialFunc(SpecialKey);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);

    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, WIDTH-1, 0, HEIGHT-1);

    glutMainLoop();
}