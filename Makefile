all:
	g++ -std=c++11 MazeGenerator.cpp -o MazeGenerator -lglut -lGLU -lGL -lm
clean:
	rm MazeGenerator
run:
	./MazeGenerator 10 10
