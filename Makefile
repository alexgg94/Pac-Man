all:
	g++ -std=c++11 PacMan.cpp -o PacMan -lglut -lGLU -lGL -lm
clean:
	rm PacMan
run:
	./PacMan 15 15
