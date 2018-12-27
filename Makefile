all:
	g++ -std=c++11 PacMan3D.cpp -o PacMan3D -lglut -lGLU -lGL -lm -w -ljpeg

mac:	
	#Compile Apple
	g++  Pacman3D.cpp -o PacMan3D -framework OpenGL -framework GLUT -std=c++11 -w -ljpeg

clean:
	rm Pacman3D
run:
	./Pacman3D 10 10
