#ifndef GAMESTAT_H
#define GAMESTAT_H

#include <fstream>  //for shader loader
#include <sstream>  //for string stream object
#include <stdio.h>  //for c style i/o
#include <string.h>
#include <cstring>
#include <iostream>

struct Player{
	char* name;
	int ptF;
	int ptA;
	int diff;
};

class GameStat{
	public:
		GameStat(char*);
		~GameStat();
		
		std::string* draw();
		bool save();
		bool read();
		void scored(int, float, int);
		
		
	private:
		int find(int diff);
		void update(Player* p1);
		int numTop10;
		Player* top10;
		Player* play1;
};

#endif
