all:
	g++ -std=c++11 Pacman3D.cpp -o PacMan3D -lglut -lGLU -lGL -lm -w -ljpeg -lpthread

mac:	
	#Compile Apple
	g++  Pacman3D.cpp -o PacMan3D -framework OpenGL -framework GLUT -std=c++11 -w -ljpeg

clean:
	rm Pacman3D
run:
	./PacMan3D 10 10
