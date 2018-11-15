all:
	g++ -std=c++11 PacMan.cpp -o PacMan -lglut -lGLU -lGL -lm
	#Compile Apple
	#gcc  vfr.cpp -o vfr -framework OpenGL -framework GLUT -L /System/Library/Frameworks/OpenGL.framework/
clean:
	rm PacMan
run:
	./PacMan 15 15
