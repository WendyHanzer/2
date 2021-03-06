#include "GameStat.h"

GameStat::GameStat(char* name1, char* name2){
	top10 = new Player[10];
	play1 = new Player();
	play1->name = new char[4];
	std::strcpy(play1->name, name1);
	play2 = new Player();
	play2->name = new char[4];	
	std::strcpy(play2->name, name2);
	numTop10 = 0;
	for (int i = 0; i < 10; i++){
		top10[i].name = new char[4];
	}
}

GameStat::~GameStat(){
	delete[] top10;
	delete play1;
	delete play2;
}

void GameStat::draw(){
	//doesn't do anything yet
	std::cout << "Top 10 Players" std::endl;
	for (int x = 0; x < 10; x++){
		std::cout << top10[x].name;
		std::cout << " " << top10[x].ptF;
		std::cout << " " << top10[x].ptA;
		std::coutout << " " << top10[x].diff << std::endl;
	}
}

//must be called after read
bool GameStat::save(){
	std::ofstream fout;
	fout.open("save.txt");
	update(play1);
	update(play2);
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

void GameStat::scored(bool isP1){
	if (isP1){
		play1->ptF++;
		play2->ptA++;
		
	} else {
		play2->ptF++;
		play1->ptA++;
	}
	play1->diff = play1->ptF - play1->ptA;
	play2->diff = play2->ptF - play2->ptA;
}



//return idx if player is found in Top 10
//otherwise returns the next idx, 10 is not a valid idx 
int GameStat::find(char* name){
	int idx = 0;
	bool found = false;
	while(idx < numTop10 && !found){
		if (std::strcmp(top10[idx].name, name) == 0){
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
	int idx = find(p1->name);
	std::cout << p1->name << std::endl;
	if (idx < 10){
		Player* tmp = new Player[10];	
		for (int i = 0; i < 10; i++){
			tmp[i].name = new char[4];
		}
		bool inserted = false;
		//player is in top 10, update
		std::cout << top10[idx].ptF << " " << top10[idx].ptA << std::endl;
		std::cout << p1->ptF << " " << p1->ptA << std::endl;
		top10[idx].ptF+=p1->ptF;
		top10[idx].ptA+=p1->ptA;
		std::cout << top10[idx].ptF << " " << top10[idx].ptA << std::endl;
		
		if (top10[idx].diff == top10[idx].ptF - top10[idx].ptA){
		 	std::cout << "update 2" << std::endl;
		} else {
			std::cout << "update 1" << std::endl;
			top10[idx].diff = top10[idx].ptF - top10[idx].ptA;
		
			int x = 0;
			for (int i = 0; i < 10; i++){	
				if( idx != i ){
					if (top10[idx].diff > top10[i].diff && !inserted ){			
						//insert to temp
						std::strcpy(tmp[x].name, top10[idx].name);
						tmp[x].ptF = top10[idx].ptF;
						tmp[x].ptA = top10[idx].ptA;
						tmp[x].diff = top10[idx].diff;
						x++;
						inserted = true;
					}
					std::strcpy(tmp[x].name, top10[i].name);
					tmp[x].ptF = top10[i].ptF;
					tmp[x].ptA = top10[i].ptA;
					tmp[x].diff = top10[i].diff;
					x++;
				}
				
			}
			if (x < numTop10 && !inserted){
				std::strcpy(tmp[x].name, top10[idx].name);
				tmp[x].ptF = top10[idx].ptF;
				tmp[x].ptA = top10[idx].ptA;
				tmp[x].diff = top10[idx].diff;
			}
	 		delete[] top10;
			top10 = tmp; 
			}	
		} else {
		std::cout << "else" << std::endl;
			bool found = false;
			int i = 0;
			while (i < 10 && !found){
			
				if (p1->diff >= top10[i].diff){
				
					for (int x = 9; x > i; x--){
						top10[x].diff = top10[x-1].diff;
						top10[x].ptF = top10[x-1].ptF;
						top10[x].ptF = top10[x-1].ptA; 
						std::strcpy(top10[x].name, top10[x-1].name);  
					}
					top10[i].diff = p1->diff;
					top10[i].ptF = p1->ptF;
					top10[i].ptF = p1->ptA;
					std::strcpy(top10[i].name, p1->name);   	
					found = true;
			
				}
				i++;
			}
		}	
	
}



