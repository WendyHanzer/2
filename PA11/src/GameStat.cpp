#include "GameStat.h"
#include <sstream>
#include <string>
#include <iostream>
GameStat::GameStat(char* name1){
	top10 = new Player[10];
	play1 = new Player();
	play1->name = new char[4];
	std::strcpy(play1->name, name1);
	numTop10 = 0;	
	for (int i = 0; i < 10; i++){
		top10[i].name = new char[4];
	}
}

GameStat::~GameStat(){
	delete[] top10;
	delete play1;
}

std::string* GameStat::draw(){
	
	std::string* fin = new std::string[10];
	for (int x = 0; x < numTop10; x++){
		std::ostringstream out;
		out << "Name: ";//5
		out << top10[x].name;//3
		out << " ";//1
		out << "Points: ";//7
		out << top10[x].diff;//6
		out << std::endl;
		fin[x] = out.str();
	}
	
	return fin;
}

//must be called after read
bool GameStat::save(){
	std::ofstream fout;
	fout.open("save.txt");
	update(play1);
	
	fout << numTop10 << std::endl;
	for (int x = 0; x < 10; x++){
		fout << top10[x].name;
		fout << " " << top10[x].ptF;
		fout << " " << top10[x].ptA;
		fout << " " << top10[x].diff << std::endl;
	}
	fout.close();	
	return true;
}

//read must be called before save always
//assumes file exist in right format
bool GameStat::read(){
	std::ifstream fin;
	fin.open("save.txt");
	fin >> numTop10;
	
	for (int x = 0; x < numTop10; x++){

		fin >> top10[x].name;
		fin >> top10[x].ptF;
		fin >> top10[x].ptA;
		fin >> top10[x].diff;
		
	}
	fin.close();		
	return true;
}

void GameStat::scored(int mx, float t, int score){
	play1->ptF += mx;
	play1->ptA += t;
	play1->diff = score;
	
}



//return idx if player is found in Top 10
//otherwise returns the next idx, 10 is not a valid idx 
int GameStat::find(int diff){
	int idx = 0;
	bool found = false;
	while(idx < numTop10 && !found){
		if (top10[idx].diff < diff){
			found = true;
		} else {
			idx++;
		}
	}
	return idx;
}

//updates the top 10 array 
//assumes save file is filled
void GameStat::update(Player* p1){
	int idx = find(p1->diff);
	std::cout << p1->name << std::endl;
	if (idx < 10){
		for (int i = 9; i > idx; i--){
			std::strcpy(top10[i].name, top10[i-1].name);
			top10[i].diff = top10[i-1].diff;
			top10[i].ptF = top10[i-1].ptF;
			top10[i].ptA = top10[i-1].ptA;
		}
		std::strcpy(top10[idx].name, p1->name);
		top10[idx].diff = p1->diff;
		top10[idx].ptF = p1->ptF;
		top10[idx].ptA = p1->ptA;
		
	}
}



