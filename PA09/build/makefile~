# Linux
CC=g++
LIBS= -lglut -lGLEW -lGL -lassimp -LLIBDIR -lIL -L/usr/local/lib -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath  

# For Macs uncomment the next line and comment out the previous one
#CC=clang++
#LIBS=  -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -stdlib=libc++
# Assuming you want to use a recent compiler

# Compiler flags
CXXFLAGS= -g -Wall -std=c++0x -Wl,-rpath -Wl,LIBDIR -I/usr/local/include/bullet

all: ../bin/Matrix

../bin/Matrix: ../src/main.cpp
	$(CC) $(CXXFLAGS) ../src/main.cpp ../src/model.h ../src/model.cpp -o ../bin/Matrix $(LIBS) 
#model.o:
#	$(CC) -c ../src/model.h ../src/model.cpp

