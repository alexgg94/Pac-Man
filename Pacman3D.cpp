//
//  Pacman3D.cpp
//  Pacman3D
//
//  Created by Josep & Alex on 22/11/18.
//  Copyright © 2018 Universitat de Lleida. All rights reserved.
//

#ifdef __APPLE__
#include <GLUT/glut.h> // Apple
#else
#include <GL/glut.h>   // Linux
#endif

#include <iostream>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <stack>
#include <tuple>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>

#include <math.h>
#include "jpeglib.h"

using namespace std;

#define WIDTH 640
#define HEIGHT 400
#define MOVEMENT_DURATION 500

#define PI 3.1416
#define SIZE 8.0

float zoom = 0.6;
float wall_height = 10;

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

void ReadJPEG(char *filename,unsigned char **image,int *width, int *height);
void LoadTexture(char *filename,int dim);
class Coordinate;
class Coord;
Coord GetCenterCoordinate(Coord coordinate1, Coord coordinate2, Coord coordinate3, Coord coordinate4);
Coord CoordinateToScreen2(int row, int col);

Coordinate CoordinateToScreen(int row, int col);
void drawSphere(int v,int x,int z);
/*--- Global variables that determine the viewpoint location ---*/

int anglealpha = 0;
int anglebeta = 0;

//-----------------------------------------------
//-----------------------------------------------

void PositionObserver(float alpha,float beta,int radi);

void display();
void keyboard(unsigned char c,int x,int y);
void specialkey(int key, int x, int y);

//-----------------------------------------------
//-----------------------------------------------

void PositionObserver(float alpha,float beta,int radi)
{
    float x,y,z;        //Eye position
    float upx,upy,upz;  //Up vector (unitary)
    float modul;
    
    x = (float)radi*cos(alpha*2*PI/360.0)*cos(beta*2*PI/360.0);
    y = (float)radi*sin(beta*2*PI/360.0);
    z = (float)radi*sin(alpha*2*PI/360.0)*cos(beta*2*PI/360.0);
//    cout << x << endl;
//    cout << y << endl;
//    cout << z << endl;
    
    if (beta>0)
    {
        upx=-x;
        upz=-z;
        upy=(x*x+z*z)/y;
    }
    else if(beta==0)
    {
        upx=0;
        upy=1;
        upz=0;
    }
    else
    {
        upx=x;
        upz=z;
        upy=-(x*x+z*z)/y;
    }
    
    modul=sqrt(upx*upx+upy*upy+upz*upz);
    
    upx=upx/modul;
    upy=upy/modul;
    upz=upz/modul;
    
    // x-y-z Eye position
    // ref1-ref2-ref3 Point the viewer is looking at
    // upx-upy-upz Up vector (unitary)
    
    //gluLookAt(x,y,z,refx,refy,refz,upx,upy,upz);
    gluLookAt(x,y,z,0.0, 0.0, 0.0,upx,upy,upz);
    
}

class Coord {
    
private:
    double coord_row;
    double coord_col;
public:
    Coord(){}
    Coord(double row, double col)
    {
        coord_row = row;
        coord_col = col;
    }
    double GetRow(){return coord_row;}
    double GetCol(){return coord_col;}
};

void drawSphere(int v,int x,int z){ //Game characters as a sphere.
    
    glPushMatrix();
    
    glTranslatef(x,wall_height/2,z);
    glBegin(GL_QUADS);
    gluSphere(gluNewQuadric(),v,100,20);
    glEnd();
    
    glPopMatrix();
    
    // Esfera
    // gluSphere(quad,radius,slices,stacks) draw a sphere
    //   quad : Specifies the quadrics object (created with gluNewQuadric).
    //   radius :Specifies the radius of the sphere.
    //  slices Specifies the number of subdivisions around the z axis(similar to lines of longitude).
    //  stacks Specifies the number of subdivisions along the z axis (similar to lines of latitude).
    
    
    //GLUquadric *quad;
    //quad = gluNewQuadric();
    //glTranslatef(2,2,2);
    //gluSphere(quad,10,100,20);
    
    
}

//Dibuixem el laberint en 3D
void drawMaze3D (){
    
    GLfloat material[4];
    
    // Centrem el mapa als eixos de coordenades
    glTranslatef(-WIDTH/2,0,-HEIGHT/2);
    
    //pintem el laberint
    for(int row = 0; row < global_rows * 2 + 1; row++)
    {
        for(int col = 0; col < global_cols * 4 + 1; col++)
        {
            Coord coordinate1 = CoordinateToScreen2(row, col);
            Coord coordinate2 = CoordinateToScreen2(row, col + 1);
            Coord coordinate3 = CoordinateToScreen2(row + 1, col + 1);
            Coord coordinate4 = CoordinateToScreen2(row + 1, col);
        
            
            if(maze[row][col] == '+' || maze[row][col] == '|' || maze[row][col] == '-' ||
               maze[row][col] == ' ' || maze[row][col] == '.')
            {
                glColor3f(1, 0, 0); // blau mur
                
                
                if(maze[row][col] == ' ' || maze[row][col] == '.')
                {
                    glColor3f(1, 1, 1); //blanc passadis
                    
                }
                
                glBindTexture(GL_TEXTURE_2D,0);
                glEnable(GL_TEXTURE_2D);
                glEnable (GL_LIGHTING);
                
                // Poligon inferior
                glBegin(GL_QUADS);
                glNormal3f(0,1,0);
                glVertex3f(coordinate4.GetRow(),0,coordinate4.GetCol());
                glVertex3f(coordinate3.GetRow(),0,coordinate3.GetCol());
                glVertex3f(coordinate2.GetRow(),0,coordinate2.GetCol());
                glVertex3f(coordinate1.GetRow(),0, coordinate1.GetCol());
                glEnd();
                

                
                // Si es paret
                if(maze[row][col] == '+' || maze[row][col] == '|' || maze[row][col] == '-' ){
                    

                    material[0]=0; material[1]=0.5; material[2]=0.5; material[3]=1.0;
                    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,material);
                    
                    // Poligon superior
                    glBegin(GL_QUADS);
                    glNormal3f(0,1,0);
                    glTexCoord2f(-4.0,4.0);
                    glVertex3f(coordinate4.GetRow(),wall_height,coordinate4.GetCol());
                    glTexCoord2f(4.0,4.0);
                    glVertex3f(coordinate3.GetRow(),wall_height,coordinate3.GetCol());
                    glTexCoord2f(4.0,0.0);
                    glVertex3f(coordinate2.GetRow(),wall_height,coordinate2.GetCol());
                    glTexCoord2f(-4.0,0.0);
                    glVertex3f(coordinate1.GetRow(),wall_height,coordinate1.GetCol());
                    glEnd();
                    
                    
                    //                        //Poligon frontal
                    glBegin(GL_QUADS);
                    glNormal3f(0,0,1);
                    glTexCoord2f(-4.0,4.0);
                    glVertex3f(coordinate4.GetRow(),0,coordinate4.GetCol());
                    glTexCoord2f(4.0,4.0);
                    glVertex3f(coordinate3.GetRow(),0,coordinate3.GetCol());
                    glTexCoord2f(4.0,0.0);
                    glVertex3f(coordinate3.GetRow(),wall_height,coordinate3.GetCol());
                    glTexCoord2f(-4.0,0.0);
                    glVertex3f(coordinate4.GetRow(),wall_height,coordinate4.GetCol());
                    glEnd();
                    
                    //                        //Poligon posterior
                    glBegin(GL_QUADS);
                    glNormal3f(0,0,-1);
                    glTexCoord2f(-4.0,4.0);
                    glVertex3f(coordinate2.GetRow(),0,coordinate2.GetCol());
                    glTexCoord2f(4.0,4.0);
                    glVertex3f(coordinate1.GetRow(),0,coordinate1.GetCol());
                    glTexCoord2f(4.0,0.0);
                    glVertex3f(coordinate1.GetRow(),wall_height,coordinate1.GetCol());
                    glTexCoord2f(-4.0,0.0);
                    glVertex3f(coordinate2.GetRow(),wall_height,coordinate2.GetCol());
                    glEnd();
                    
                    //Lateral dret
                    glBegin(GL_QUADS);
                    glNormal3f(1,0,0);
                    glTexCoord2f(-4.0,4.0);
                    glVertex3f(coordinate3.GetRow(),0,coordinate3.GetCol());
                    glTexCoord2f(4.0,4.0);
                    glVertex3f(coordinate2.GetRow(),0,coordinate2.GetCol());
                    glTexCoord2f(4.0,0.0);
                    glVertex3f(coordinate2.GetRow(),wall_height,coordinate2.GetCol());
                    glTexCoord2f(-4.0,0.0);
                    glVertex3f(coordinate3.GetRow(),wall_height,coordinate3.GetCol());
                    glEnd();
                    
                    //Lateral esquerra
                    
                    glBegin(GL_QUADS);
                    glNormal3f(-1,0,0);
                    glTexCoord2f(-4.0,4.0);
                    glVertex3f(coordinate1.GetRow(),0,coordinate1.GetCol());
                    glTexCoord2f(4.0,4.0);
                    glVertex3f(coordinate4.GetRow(),0,coordinate4.GetCol());
                    glTexCoord2f(4.0,0.0);
                    glVertex3f(coordinate4.GetRow(),wall_height,coordinate4.GetCol());
                    glTexCoord2f(-4.0,0.0);
                    glVertex3f(coordinate1.GetRow(),wall_height,coordinate1.GetCol());
                    glEnd();
                    
                    glDisable(GL_TEXTURE_2D);
                    glDisable (GL_LIGHTING);

                }
                
            }
            
            if(maze[row][col] == '.')  // si es un pellet
            {
                glDisable(GL_TEXTURE_2D);
                glDisable (GL_LIGHTING);
                //dibuixa el menjar de color vermell
                glColor3f(1, 1, 0);
                
                Coord coordinate1 = CoordinateToScreen2(row, col);
                Coord coordinate2 = CoordinateToScreen2(row, col + 1);
                Coord coordinate3 = CoordinateToScreen2(row + 1, col + 1);
                Coord coordinate4 = CoordinateToScreen2(row + 1, col);
                
                Coord centerCoordinate = GetCenterCoordinate(coordinate1,coordinate2, coordinate3, coordinate4);
                // Coord3D getCenter = GetCenterCoordinate3D(coordinate1,coordinate2, coordinate3, coordinate4);
                
                // Blue Pellets
                //glColor3f(0,200,200);
                drawSphere(2,centerCoordinate.GetRow(),centerCoordinate.GetCol());
                
                //                glBegin(GL_QUADS);
                //                    glVertex3f(centerCoordinate.GetRow() - 2,MAZE_HEIGHT, centerCoordinate.GetCol() - 2);
                //                    glVertex3f(centerCoordinate.GetRow() - 2,MAZE_HEIGHT, centerCoordinate.GetCol() + 2);
                //                    glVertex3f(centerCoordinate.GetRow() + 2,MAZE_HEIGHT, centerCoordinate.GetCol() + 2);
                //                    glVertex3f(centerCoordinate.GetRow() + 2,MAZE_HEIGHT, centerCoordinate.GetCol() - 2);
                //                glEnd();
                //
                //                glBegin(GL_QUADS);
                //                    glVertex3f(centerCoordinate.GetRow() - 2,MAZE_HEIGHT/2, centerCoordinate.GetCol() - 2);
                //                    glVertex3f(centerCoordinate.GetRow() - 2,MAZE_HEIGHT/2, centerCoordinate.GetCol() + 2);
                //                    glVertex3f(centerCoordinate.GetRow() + 2,MAZE_HEIGHT/2, centerCoordinate.GetCol() + 2);
                //                    glVertex3f(centerCoordinate.GetRow() + 2,MAZE_HEIGHT/2, centerCoordinate.GetCol() - 2);
                //                glEnd();
                
                glEnable(GL_TEXTURE_2D);
                glEnable (GL_LIGHTING);
            }
        }
    }
    
    //imprimim les particules : pacman i fantasmes
//    Particles[0].Draw();
//    Particles[1].Draw();
//    Particles[2].Draw();
//    Particles[3].Draw();
//    Particles[4].Draw();
    
}

void drawAxis(){ //Game 3D Axis
    // pintem els eixos x,y,z
    glBegin(GL_LINES);
    // eix x
    glColor4f(1.0, 0.0, 0.0, 1.0);
    glVertex3f(0.0,0.0,0.0);
    glVertex3f(100*SIZE,0.0,0.0);
    // eix y
    glColor4f(0.0, 1.0, 0.0, 1.0);
    glVertex3f(0.0,0.0,0.0);
    glVertex3f(0.0,50*SIZE,0.0);
    // eix z
    glColor4f(0.0, 0.0, 1.0, 1.0);
    glVertex3f(0.0,0.0,0.0);
    glVertex3f(0.0,0.0,50*SIZE);
    glEnd();
}


Coord GetCenterCoordinate(Coord coordinate1, Coord coordinate2, Coord coordinate3, Coord coordinate4){
    
    return Coord(coordinate1.GetRow() + (coordinate2.GetRow() - coordinate1.GetRow()) / 2, coordinate1.GetCol() + (coordinate4.GetCol() - coordinate1.GetCol()) / 2);
}


Coord CoordinateToScreen2(int row, int col)
{
    return Coord(col*WIDTH/(global_cols * 4 + 1), row*HEIGHT/(global_rows * 2 + 1));
}

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
    
    double Utility(Node node1, Node node2)
    {
        double manhattan_distance = abs(node1.GetRow() - node2.GetRow()) + abs(node1.GetCol() - node2.GetCol());

        if(manhattan_distance == 0)
            return 100;
        return 1.0/(manhattan_distance * manhattan_distance);
    }
/*
    bool IsTerminalState(int depth, int remaining_pellets, Node node1, Node node2)
    {
        if(depth == 0 || remaining_pellets == 0 || Utility(node1, node2) == 100)
            return true;
        return false;
    }
*/
    Node GetRandomUnvisitedAdjacentNode(int current_row, int current_col, vector<Node> visited)
    {
        vector<Node> adjacentNodes;

        auto is_visited = [&](int row, int col)
        {
            return std::find_if(begin(visited), end(visited), [&](Node &node)
                                {
                                    return node.GetRow() == row && node.GetCol() == col;
                                }) != end(visited);
        };
        
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
    
    vector<Node> GetAllAdjacentNodes(int current_row, int current_col)
    {
        vector<Node> adjacentNodes;

        if(current_row + 1 < global_rows*2)
        {
            if(maze[current_row + 1][current_col] == ' ' || maze[current_row + 1][current_col] == '.')
            {
                adjacentNodes.push_back(Node(Coordinate(current_row + 1, current_col), Direction::UP));
            }
        }
        
        if(current_row - 1 > 0)
        {
            if(maze[current_row - 1][current_col] == ' ' || maze[current_row - 1][current_col] == '.')
            {
                adjacentNodes.push_back(Node(Coordinate(current_row - 1, current_col), Direction::DOWN));
            }
        }
        
        if(current_col + 1  < global_cols*4)
        {
            if(maze[current_row][current_col + 1] == ' ' || maze[current_row][current_col + 1] == '.')
            {
                adjacentNodes.push_back(Node(Coordinate(current_row, current_col + 1), Direction::LEFT));
            }
        }
        
        if(current_col - 1  > 0)
        {
            if(maze[current_row][current_col - 1] == ' ' || maze[current_row][current_col - 1] == '.')
            {
                adjacentNodes.push_back(Node(Coordinate(current_row, current_col - 1), Direction::RIGHT));
            }
        }

        return adjacentNodes;
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
                Node random_adjacent_node = GetRandomUnvisitedAdjacentNode(current_x, current_y, visited);
                
                if(random_adjacent_node.GetRow() == -1 && random_adjacent_node.GetCol() == -1)
                {
                    visited.clear();
                    visited.push_back(Node(Coordinate(current_x, current_y), Direction::NONE));
                    random_adjacent_node = GetRandomUnvisitedAdjacentNode(current_x, current_y, visited);
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

    void InitMovementEnemy_Expectimax(int initial_x, int initial_y, int particle_index)
    {
        int current_x = initial_x;
        int current_y = initial_y;
        int current_cell_content = ' ';
        int initial_depth = 5;

        std::this_thread::sleep_for(std::chrono::seconds(3));
        while(true)
        {
            int total_pellets = remaining_pellets;

            if(Particles[particle_index].particle_state == ParticleState::QUIET)
            {
                vector<Node> ghost_adjacentNodes = GetAllAdjacentNodes(current_x, current_y);
                vector<Node> actions;
                double max_value = -INFINITY;
                double current_value;

                for(int i = 0; i < ghost_adjacentNodes.size(); i++)
                {
                    current_value = pacman_agent(Node(Coordinate(Particles[0].particle_x, Particles[0].particle_y), Direction::NONE), ghost_adjacentNodes[i], initial_depth);

                    if(current_value == max_value)
                    {
                        actions.push_back(ghost_adjacentNodes[i]);
                    }

                    else if(current_value > max_value)
                    {
                        actions.clear();
                        actions.push_back(ghost_adjacentNodes[i]);
                        max_value = current_value;
                    }
                }

                random_shuffle (actions.begin(), actions.end());
                Node adjacent_node = actions[0];

                if(current_cell_content != 'e')
                {
                    maze[current_x][current_y] = current_cell_content;
                }
                
                Coordinate originGraphicalCoordinate1 = CoordinateToScreen(current_x, current_y);
                Coordinate originGraphicalCoordinate2 = CoordinateToScreen(current_x, current_y + 1);
                Coordinate originGraphicalCoordinate3 = CoordinateToScreen(current_x + 1, current_y + 1);
                Coordinate originGraphicalCoordinate4 = CoordinateToScreen(current_x + 1, current_y);
                Coordinate originCenterGraphicalCoordinate = GetCenterCoordinate(originGraphicalCoordinate1, originGraphicalCoordinate2, originGraphicalCoordinate3, originGraphicalCoordinate4);
                
                Coordinate destinationGraphicalCoordinate1 = CoordinateToScreen(adjacent_node.GetRow(), adjacent_node.GetCol());
                Coordinate destinationGraphicalCoordinate2 = CoordinateToScreen(adjacent_node.GetRow(), adjacent_node.GetCol() + 1);
                Coordinate destinationGraphicalCoordinate3 = CoordinateToScreen(adjacent_node.GetRow() + 1, adjacent_node.GetCol() + 1);
                Coordinate destinationGraphicalCoordinate4 = CoordinateToScreen(adjacent_node.GetRow() + 1, adjacent_node.GetCol());
                Coordinate destinationCenterGraphicalCoordinate = GetCenterCoordinate(destinationGraphicalCoordinate1, destinationGraphicalCoordinate2, destinationGraphicalCoordinate3, destinationGraphicalCoordinate4);
                
                SetPosition(originCenterGraphicalCoordinate.GetRow(), originCenterGraphicalCoordinate.GetCol(), particle_index);
                InitMovement(originCenterGraphicalCoordinate.GetRow(), originCenterGraphicalCoordinate.GetCol(), destinationCenterGraphicalCoordinate.GetRow(), destinationCenterGraphicalCoordinate.GetCol(), particle_index);
                
                current_x = adjacent_node.GetRow();
                current_y = adjacent_node.GetCol();
                current_cell_content = maze[current_x][current_y];
                
                if(current_x == Particles[0].particle_x && current_y == Particles[0].particle_y)
                {
                    cout << "YOU LOSE!" << endl;
                    exit(0);
                }
            }
        }
    }

    double ghost_agent(Node pacman_node, Node ghost_node, int depth)
    {
        if(depth == 0)
        {
            return Utility(pacman_node, ghost_node);
        }

        vector<Node> adjacentNodes = GetAllAdjacentNodes(ghost_node.GetRow(), ghost_node.GetCol());

        double value = -INFINITY;
        for(int i = 0; i < adjacentNodes.size(); i++)
        {
            value = max(value, pacman_agent(pacman_node, adjacentNodes[i], depth));
        }

        return value;
    }

    double pacman_agent(Node pacman_node, Node ghost_node, int depth)
    {
        int i;
        if(depth == 0)
        {
            return Utility(pacman_node, ghost_node);
        }

        vector<Node> adjacentNodes = GetAllAdjacentNodes(pacman_node.GetRow(), pacman_node.GetCol());
        double value = 0;
        for(i = 0; i < adjacentNodes.size(); i++)
        {
            value += ghost_agent(adjacentNodes[i], ghost_node, depth-1);
        }

        return value/i;
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
            //std::thread t(&Particle::InitMovementEnemy, this, particle_x, particle_y, visited, particle_index);
            std::thread t(&Particle::InitMovementEnemy_Expectimax, this, particle_x, particle_y, particle_index);
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
    
    void Draw3D()
    {
        glColor3f(1, 0, 0);
        
        if(particle_type == ParticleType::PACMAN)
        {
            glColor3f(1, 1, 0);
        }
        
//        Coordinate coordinate1 = CoordinateToScreen(graphical_x - WIDTH/(global_cols * 4 + 1)/2, graphical_y - HEIGHT/(global_rows * 2 + 1)/2);
//        Coordinate coordinate2 = CoordinateToScreen(graphical_x + WIDTH/(global_cols * 4 + 1)/2, graphical_y - HEIGHT/(global_rows * 2 + 1)/2+1);
//        Coordinate coordinate3 = CoordinateToScreen(graphical_x + WIDTH/(global_cols * 4 + 1)/2+1,graphical_y + HEIGHT/(global_rows * 2 + 1)/2+1);
//        Coordinate coordinate4 = CoordinateToScreen(graphical_x - WIDTH/(global_cols * 4 + 1)/2+1, graphical_y + HEIGHT/(global_rows * 2 + 1)/2);
//        Coordinate centerCoordinate = GetCenterCoordinate(coordinate1, coordinate2, coordinate3, coordinate4);

        drawSphere(7,graphical_x - WIDTH/(global_cols * 4 + 1)/4,graphical_y - HEIGHT/(global_rows * 2 + 1)/4);
        //drawSphere(4,centerCoordinate.GetCol(),centerCoordinate.GetRow());

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

//Display procedure 2D
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

//-----------------------------------------------
//-----------------------------------------------
void display3D()
{
    
    GLfloat position[4];
    GLfloat direction[3];
    GLfloat color[4];
    
    // Clear the screen
    glClearColor(1.0,1.0,1.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    // Specify the observer position
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); //replace the current matrix with the identity matrix
    
    PositionObserver(anglealpha,anglebeta,450);
    
    //set projection parameters
    glMatrixMode(GL_PROJECTION);
    //GL_PROJECTION Applies subsequent matrix operations to the projection matrix stack.
    glLoadIdentity();
    
    //wmin,wmax,hmin,hmax,near,far
    glOrtho(-WIDTH*zoom,WIDTH*zoom,-HEIGHT*zoom,HEIGHT*zoom,10,2000);
    
    
    // glMatrixMode : specify which matrix is the current matrix
    glMatrixMode(GL_MODELVIEW);
    // GL_MODELVIEW : Applies subsequent matrix operations to the modelview matrix stack.
    
    
    //To draw a surface with outlined polygons, call
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    //-- Ambient light
    
    position[0]=0;
    position[1]=0;
    position[2]=0;
    position[3]=1;

    glLightfv(GL_LIGHT0,GL_POSITION,position);
    //
    color[0]=0; color[1]=0; color[2]=0; color[3]=1;
    glLightfv(GL_LIGHT0,GL_AMBIENT,color);
    glEnable(GL_LIGHT0);
    
    //-- Spot light

    for(int i = 0; i < 5; i++)
    {
        position[0]=Particles[i].graphical_x-WIDTH/2;
        position[1]= wall_height * 5;
        position[2]=Particles[i].graphical_y-HEIGHT/2;
        position[3]=1;
        
        direction[0]=Particles[i].velocity_x;
        direction[1]= 0;
        direction[2]=Particles[i].velocity_y;
        
        if(i == 0)
        {
            glLightfv(GL_LIGHT1,GL_POSITION,position);
            glLightfv(GL_LIGHT1,GL_SPOT_DIRECTION,direction);
            
            color[0]=1; color[1]=1; color[2]=0; color[3]=0;
            glLightfv(GL_LIGHT1,GL_DIFFUSE,color);
            glLightf(GL_LIGHT1,GL_SPOT_CUTOFF,30);
            
            glLightf(GL_LIGHT1,GL_CONSTANT_ATTENUATION,1.0);
            glLightf(GL_LIGHT1,GL_LINEAR_ATTENUATION,0.0);
            glLightf(GL_LIGHT1,GL_QUADRATIC_ATTENUATION,0.0);
            
            glEnable(GL_LIGHT1);
        }

        else if(i == 1)
        {
            glLightfv(GL_LIGHT2,GL_POSITION,position);
            glLightfv(GL_LIGHT2,GL_SPOT_DIRECTION,direction);
            
            color[0]=1; color[1]=1; color[2]=0; color[3]=0;
            glLightfv(GL_LIGHT2,GL_DIFFUSE,color);
            glLightf(GL_LIGHT2,GL_SPOT_CUTOFF,30);
            
            glLightf(GL_LIGHT2,GL_CONSTANT_ATTENUATION,1.0);
            glLightf(GL_LIGHT2,GL_LINEAR_ATTENUATION,0.0);
            glLightf(GL_LIGHT2,GL_QUADRATIC_ATTENUATION,0.0);
            
            glEnable(GL_LIGHT2);
        }

        else if(i == 2)
        {
            glLightfv(GL_LIGHT3,GL_POSITION,position);
            glLightfv(GL_LIGHT3,GL_SPOT_DIRECTION,direction);
            
            color[0]=1; color[1]=1; color[2]=0; color[3]=0;
            glLightfv(GL_LIGHT3,GL_DIFFUSE,color);
            glLightf(GL_LIGHT3,GL_SPOT_CUTOFF,30);
            
            glLightf(GL_LIGHT3,GL_CONSTANT_ATTENUATION,1.0);
            glLightf(GL_LIGHT3,GL_LINEAR_ATTENUATION,0.0);
            glLightf(GL_LIGHT3,GL_QUADRATIC_ATTENUATION,0.0);
            
            glEnable(GL_LIGHT3);
        }

        else if(i == 3)
        {
            glLightfv(GL_LIGHT4,GL_POSITION,position);
            glLightfv(GL_LIGHT4,GL_SPOT_DIRECTION,direction);
            
            color[0]=1; color[1]=1; color[2]=0; color[3]=0;
            glLightfv(GL_LIGHT4,GL_DIFFUSE,color);
            glLightf(GL_LIGHT4,GL_SPOT_CUTOFF,30);
            
            glLightf(GL_LIGHT4,GL_CONSTANT_ATTENUATION,1.0);
            glLightf(GL_LIGHT4,GL_LINEAR_ATTENUATION,0.0);
            glLightf(GL_LIGHT4,GL_QUADRATIC_ATTENUATION,0.0);
            
            glEnable(GL_LIGHT4);
        }

        else if(i == 4)
        {
            glLightfv(GL_LIGHT5,GL_POSITION,position);
            glLightfv(GL_LIGHT5,GL_SPOT_DIRECTION,direction);
            
            color[0]=1; color[1]=1; color[2]=0; color[3]=0;
            glLightfv(GL_LIGHT5,GL_DIFFUSE,color);
            glLightf(GL_LIGHT5,GL_SPOT_CUTOFF,30);
            
            glLightf(GL_LIGHT5,GL_CONSTANT_ATTENUATION,1.0);
            glLightf(GL_LIGHT5,GL_LINEAR_ATTENUATION,0.0);
            glLightf(GL_LIGHT5,GL_QUADRATIC_ATTENUATION,0.0);
            
            glEnable(GL_LIGHT5);
        }

        else if(i == 5)
        {
            glLightfv(GL_LIGHT6,GL_POSITION,position);
            glLightfv(GL_LIGHT6,GL_SPOT_DIRECTION,direction);
            
            color[0]=1; color[1]=1; color[2]=0; color[3]=0;
            glLightfv(GL_LIGHT6,GL_DIFFUSE,color);
            glLightf(GL_LIGHT6,GL_SPOT_CUTOFF,30);
            
            glLightf(GL_LIGHT6,GL_CONSTANT_ATTENUATION,1.0);
            glLightf(GL_LIGHT6,GL_LINEAR_ATTENUATION,0.0);
            glLightf(GL_LIGHT6,GL_QUADRATIC_ATTENUATION,0.0);
            
            glEnable(GL_LIGHT6);
        }
    }
    
    // glPolygonMode(face,mode) Select a polygon rasterization mode
    // face Specifies the polygons that mode applies to : GL_FRONT,GL_BACK,GL_FRONT_AND_BACK
    // mode Specifies how polygons will be rasterized :  GL_POINT, GL_LINE, GL_FILL
    
    // GL_FILL The interior of the polygon is filled.
    //S'omplen les parets del polígon, sinó quda l'esquelet
    glPolygonMode(GL_FRONT,GL_FILL);
    
    //GL_LINE Boundary edges of the polygon are drawn as line segments.
    //Les vores del límit del polígon es dibuixen com segments de línia
    glPolygonMode(GL_RED,GL_LINE); // no pinta les parets interiors
    
    drawAxis();
    drawMaze3D();
    
    // Murs alts cantoners
    glEnable(GL_TEXTURE_2D);
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(-4.0,0.0);
    glVertex3i(0,0,0);
    glTexCoord2f(4.0,0.0);
    glVertex3i(WIDTH,0,0);
    
    glTexCoord2f(4.0,4.0);
    glVertex3i(WIDTH,200,0);
    glTexCoord2f(-4.0,4.0);
    glVertex3i(0,200,0);
    glEnd();
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(-4.0,4.0);
    glVertex3i(0,0,HEIGHT);
    glTexCoord2f(4.0,4.0);
    glVertex3i(0,0,0);
    glTexCoord2f(4.0,0.0);
    glVertex3i(0,200,0);
    glTexCoord2f(-4.0,0.0);
    glVertex3i(0,200,HEIGHT);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    
//    // Centrem el mapa als eixos de coordenades
//    glTranslatef(-WIDTH/2,0,-HEIGHT/2);
//
//
//    //pintem el laberint
//    for(int row = 0; row < global_rows * 2 + 1; row++)
//    {
//        for(int col = 0; col < global_cols * 4 + 1; col++)
//        {
//            Coordinate coordinate4 = CoordinateToScreen(row, col);
//            Coordinate coordinate3 = CoordinateToScreen(row, col + 1);
//            Coordinate coordinate2 = CoordinateToScreen(row + 1, col + 1);
//            Coordinate coordinate1 = CoordinateToScreen(row + 1, col);
//            Coordinate centerCoordinate = GetCenterCoordinate(coordinate1, coordinate2, coordinate3, coordinate4);
//
//            if(maze[row][col] == '+' || maze[row][col] == '|' || maze[row][col] == '-' ||
//               maze[row][col] == ' ' || maze[row][col] == '.')
//            {
//                glColor3f(0, 0, 1);
//
//                if(maze[row][col] == ' ' || maze[row][col] == '.')
//                {
//                    glColor3f(1, 1, 1);
//                }
//
////                glBegin(GL_QUADS);
////                    glVertex3f(coordinate1.GetRow(),wall_height, coordinate1.GetCol());
////                    glVertex3f(coordinate2.GetRow(),wall_height, coordinate2.GetCol());
////                    glVertex3f(coordinate3.GetRow(),wall_height, coordinate3.GetCol());
////                    glVertex3f(coordinate4.GetRow(),wall_height, coordinate4.GetCol());
////                glEnd();
//
//                // Poligon inferior
//                glBegin(GL_QUADS);
//                glVertex3f(coordinate1.GetRow(),0,coordinate1.GetCol());
//                glVertex3f(coordinate2.GetRow(),0,coordinate2.GetCol());
//                glVertex3f(coordinate3.GetRow(),0,coordinate3.GetCol());
//                glVertex3f(coordinate4.GetRow(),0, coordinate4.GetCol());
//                glEnd();
//
//                // Si es paret
//                if(maze[row][col] == '+' || maze[row][col] == '|' || maze[row][col] == '-' ){
//
//                    // Poligon superior
//                    glBegin(GL_QUADS);
//                    glVertex3f(coordinate1.GetRow(),wall_height,coordinate1.GetCol());
//                    glVertex3f(coordinate2.GetRow(),wall_height,coordinate2.GetCol());
//                    glVertex3f(coordinate3.GetRow(),wall_height,coordinate3.GetCol());
//                    glVertex3f(coordinate4.GetRow(),wall_height,coordinate4.GetCol());
//                    glEnd();
//
//                    //                        //Poligon frontal
//                    glBegin(GL_QUADS);
//                    glVertex3f(coordinate1.GetRow(),0,coordinate1.GetCol());
//                    glVertex3f(coordinate2.GetRow(),0,coordinate2.GetCol());
//                    glVertex3f(coordinate2.GetRow(),wall_height,coordinate2.GetCol());
//                    glVertex3f(coordinate1.GetRow(),wall_height,coordinate1.GetCol());
//                    glEnd();
//
//                    //Poligon posterior
//                    glBegin(GL_QUADS);
//
//                    glVertex3f(coordinate3.GetRow(),0,coordinate3.GetCol());
//                    glVertex3f(coordinate4.GetRow(),0,coordinate4.GetCol());
//                    glVertex3f(coordinate4.GetRow(),wall_height,coordinate4.GetCol());
//                    glVertex3f(coordinate3.GetRow(),wall_height,coordinate3.GetCol());
//                    glEnd();
//
//                    //Lateral dret
//                    glBegin(GL_QUADS);
//
//                    glVertex3f(coordinate3.GetRow(),0,coordinate3.GetCol());
//                    glVertex3f(coordinate2.GetRow(),0,coordinate2.GetCol());
//                    glVertex3f(coordinate2.GetRow(),wall_height,coordinate2.GetCol());
//                    glVertex3f(coordinate3.GetRow(),wall_height,coordinate3.GetCol());
//                    glEnd();
//
//                    //Lateral esquerra
//
//                    glBegin(GL_QUADS);
//
//                    glVertex3f(coordinate1.GetRow(),0,coordinate1.GetCol());
//                    glVertex3f(coordinate4.GetRow(),0,coordinate4.GetCol());
//                    glVertex3f(coordinate4.GetRow(),wall_height,coordinate4.GetCol());
//                    glVertex3f(coordinate1.GetRow(),wall_height,coordinate1.GetCol());
//                    glEnd();
//                }
//            }
//
//            if(maze[row][col] == '.')
//            {
////                glColor3f(0, 1, 0);
////                glBegin(GL_QUADS);
////                    glVertex3f(centerCoordinate.GetRow() - 1,wall_height, centerCoordinate.GetCol() - 1);
////                    glVertex3f(centerCoordinate.GetRow() - 1,wall_height, centerCoordinate.GetCol() + 1);
////                    glVertex3f(centerCoordinate.GetRow() + 1,wall_height, centerCoordinate.GetCol() + 1);
////                    glVertex3f(centerCoordinate.GetRow() + 1,wall_height, centerCoordinate.GetCol() - 1);
////                glEnd();
//
//                Coord coordinate1 = CoordinateToScreen2(row, col);
//                Coord coordinate2 = CoordinateToScreen2(row, col + 1);
//                Coord coordinate3 = CoordinateToScreen2(row + 1, col + 1);
//                Coord coordinate4 = CoordinateToScreen2(row + 1, col);
//
//                Coord centerCoordinate = GetCenterCoordinate(coordinate1,coordinate2, coordinate3, coordinate4);
//                // Coord3D getCenter = GetCenterCoordinate3D(coordinate1,coordinate2, coordinate3, coordinate4);
//
//                // Blue Pellets
//                glColor3f(0,200,200);
//                drawSphere(2,centerCoordinate.GetRow(),centerCoordinate.GetCol());
//            }
//        }
//    }
    
    Particles[0].Draw3D();
    Particles[1].Draw3D();
    Particles[2].Draw3D();
    Particles[3].Draw3D();
    Particles[4].Draw3D();
    
    glutSwapBuffers();
}


void keyboard(unsigned char c,int x,int y)
{
    if(keyflag==0)
        keyflag=1;
    else
        keyflag=0;
    
    if (c=='i' && anglebeta<=(90-4))
        anglebeta=(anglebeta+3);
    else if (c=='k' && anglebeta>=(-90+4))
        anglebeta=anglebeta-3;
    else if (c=='j')
        anglealpha=(anglealpha+3)%360;
    else if (c=='l')
        anglealpha=(anglealpha-3+360)%360;
    else if (c=='z') // zoom in
        zoom = zoom  - 0.02;
    else if(c=='x') // zoom out
        zoom = zoom  + 0.02;
    else if (c ==27)
        exit (0); // ESC Key pressed
    else if (c ==32)
        std::cout<<"SPACE Key pressed key:"<<c<<std::endl;
    
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
        case GLUT_KEY_DOWN:
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
        case GLUT_KEY_UP:
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
    
    if (argc > 2) {
        // to put argument in xCode : CMD + ">" > Run> Arguments > + > 10 10
        global_rows = stoi(argv[1]);
        global_cols = stoi(argv[2]);
    }else{
        global_rows = 10; //stoi(argv[1]);
        global_cols = 10; //stoi(argv[2]);
    }
    
    anglealpha=45;
    anglebeta=30;
    
    Maze m = Maze(global_rows, global_cols);
    maze = m.GetMaze();
    remaining_pellets = m.GetNumberOfPellets();
    
    InitializeParticles();
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Maze - zoom(z,x) rotateAxis(i,j,k,l) movePacman(arrows)");
    
    glEnable(GL_DEPTH_TEST);
    
    glutSpecialFunc(SpecialKey);
    glutKeyboardFunc(keyboard);
    
    // glutDisplayFunc(display);
    glutDisplayFunc(display3D);
    
    glutIdleFunc(idle);
    
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, WIDTH-1, 0, HEIGHT-1);
    
    glBindTexture(GL_TEXTURE_2D,0);
    
    // /Users/josep/Projects/Xcode/PacmanNew3D/PacmanNew3D/pared.jpg
    char* p = (char*)"pared.jpg"; // valid and safe in either C or C++.
    LoadTexture(p,64);
    
    glutMainLoop();
}



/*--------------------------------------------------------*/
/*--------------------------------------------------------*/
void ReadJPEG(char *filename,unsigned char **image,int *width, int *height)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * infile;
    unsigned char **buffer;
    int i,j;
    
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    
    
    if ((infile = fopen(filename, "rb")) == NULL) {
        printf("Unable to open file %s\n",filename);
        exit(1);
    }
    
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_calc_output_dimensions(&cinfo);
    jpeg_start_decompress(&cinfo);
    
    *width = cinfo.output_width;
    *height  = cinfo.output_height;
    
    
    *image=(unsigned char*)malloc(cinfo.output_width*cinfo.output_height*cinfo.output_components);
    
    buffer=(unsigned char **)malloc(1*sizeof(unsigned char **));
    buffer[0]=(unsigned char *)malloc(cinfo.output_width*cinfo.output_components);
    
    
    i=0;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        
        for(j=0;j<cinfo.output_width*cinfo.output_components;j++)
        {
            (*image)[i]=buffer[0][j];
            i++;
        }
        
    }
    
    free(buffer);
    jpeg_finish_decompress(&cinfo);
}



/*--------------------------------------------------------*/
/*--------------------------------------------------------*/
void LoadTexture(char *filename,int dim)
{
    unsigned char *buffer;
    unsigned char *buffer2;
    int width,height;
    long i,j;
    long k,h;
    
    ReadJPEG(filename,&buffer,&width,&height);
    
    buffer2=(unsigned char*)malloc(dim*dim*3);
    
    //-- The texture pattern is subsampled so that its dimensions become dim x dim --
    for(i=0;i<dim;i++)
        for(j=0;j<dim;j++)
        {
            k=i*height/dim;
            h=j*width/dim;
            
            buffer2[3*(i*dim+j)]=buffer[3*(k*width +h)];
            buffer2[3*(i*dim+j)+1]=buffer[3*(k*width +h)+1];
            buffer2[3*(i*dim+j)+2]=buffer[3*(k*width +h)+2];
            
        }
    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,dim,dim,0,GL_RGB,GL_UNSIGNED_BYTE,buffer2);
    
    free(buffer);
    free(buffer2);
}
