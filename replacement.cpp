#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>


using namespace std;

typedef struct{
	int block;
	int lruInt;
}cacheContent;

typedef struct thisStruct hotColdStruct;

struct thisStruct{
	int hc;
	hotColdStruct* left;
	hotColdStruct* right;
	int val;
};

void directMapped(string in, ofstream& out, int size);

void associative(string in, ofstream& out, int associativity);

void fullyAssocLRU(string in, ofstream& out);

void fullyAssocHC(string in, ofstream& out);

void associativeNoWrite(string in, ofstream& out, int associativity);

void associativeNextLine(string in, ofstream& out, int associativity);

void associativeNextLineMiss(string in, ofstream& out, int associativity);

int main(int argc, char** argv){
	string in(argv[1]);
	ofstream out(argv[2]);
	for(int i=32; i<=512; i *=4){
		directMapped(in, out, i);
	}
	directMapped(in, out, 1024);
	out << endl;
	for(int i=2; i<=16; i*=2){
		associative(in, out, i);
	}
	out << endl;
	fullyAssocLRU(in, out);
	out << endl;
	fullyAssocHC(in, out);
	out << endl;
	for(int i=2; i<=16; i*=2){
		associativeNoWrite(in, out, i);
	}
	out << endl;
	for(int i=2; i<=16; i*=2){
		associativeNextLine(in, out, i);
	}
	out << endl;
	for(int i=2; i<=16; i*=2){
		associativeNextLineMiss(in, out, i);
	}
	out << endl;
	out.close();
	return 0;
}

void directMapped(string in, ofstream& out, int size){
	char* junk;
	int numTotal=0, numHits=0;
	string currentLine;
	ifstream inStream(in.c_str());
	long cache[size];
	for(int i=0; i<size; i++){
		cache[i] = 0;
	}
	while(getline(inStream, currentLine)){
		string saddress = currentLine.substr(4,8);
		long address = strtol(saddress.c_str(), &junk, 16);
		//cout << hex << address << endl;
		long block = address/32;
		long index = block % size;
		if(cache[index] != block){
			cache[index] = block;
		}
		else{
			numHits++;
		}
		numTotal++;
	}
	out << numHits << "," << numTotal << "; ";
	inStream.close();
}

void associative(string in, ofstream& out, int associativity){
	char* junk;
	int numTotal=0, numHits=0;
	string currentLine;
	ifstream inStream(in.c_str());
	cacheContent cache[16384/32/associativity][associativity];
	bool cacheValid[16384/32/associativity][associativity];
	for(int i=0; i<(16384/32/associativity); i++){
		for(int j=0; j<associativity; j++){
			cacheValid[i][j] = false;
		}
	}
	while(getline(inStream, currentLine)){
		bool found = false;
		string saddress = currentLine.substr(4,8);
		long address = strtol(saddress.c_str(), &junk, 16);
		long block = address / 32;
		long index = block % (16384/32/associativity);
		cacheContent insert;
		insert.block = block;
		insert.lruInt = 0;
		cacheContent* cacheSet = cache[index];
		bool* validSet = cacheValid[index];
		for(int i=0; i<associativity; i++){
			if(validSet[i] == true){
				if(cacheSet[i].block == block){
					found = true;
					numHits++;
					cacheSet[i].lruInt = 0;
					continue;
				}
				cacheSet[i].lruInt++;
			}
		}
		if(!found){
			int biggest=0, indexToSwap=0;
			for(int i=0; i<associativity; i++){
				if(validSet[i] == true){
					if(cacheSet[i].lruInt > biggest){
						biggest = cacheSet[i].lruInt;
						indexToSwap = i;
					}
				}
				else{
					indexToSwap = i;
					break;
				}
			}
			cacheSet[indexToSwap] = insert;
			validSet[indexToSwap] = true;
		}
		numTotal++;
	}
	out << numHits << "," << numTotal << "; ";
}

void fullyAssocLRU(string in, ofstream& out){
	char* junk;
	int numTotal=0, numHits=0;
	string currentLine;
	ifstream inStream(in.c_str());
	long cache[512];
	bool valid[512];
	int lruCount[512];
	for(int i=0; i<512; i++){
		cache[i] = 0;
		valid[i] = false;
		lruCount[i] = 0;
	}
	while(getline(inStream, currentLine)){
		string saddress = currentLine.substr(4,8);
		long address = strtol(saddress.c_str(), &junk, 16);
		long block = address / 32;
		bool set = false;
		for(int i=0; i<512; i++){
			if(valid[i] == true && cache[i] == block){
				numHits++;
				lruCount[i] = 0;
				set = true;
				for(int j=0; j<512; j++){
					if(j != i && valid[j]){
						lruCount[j]++;
					}
				}
				break;
			}
			else if(valid[i] == false){
				cache[i] = block;
				valid[i] = true;
				set = true;
				for(int j=0; j<512; j++){
					if(j != i && valid[j]){
						lruCount[j]++;
					}
				}
				break;
			}
		}
		if(!set){
			int biggest=0, indexToSwap=0;
			for(int i=0; i<512; i++){
				if(lruCount[i] > biggest){
					indexToSwap = i;
					biggest = lruCount[i];
				}
				lruCount[i]++;
			}
			lruCount[indexToSwap] = 0;
			cache[indexToSwap] = block;
		}
		numTotal++;
	}
	out << numHits << "," << numTotal << "; "; 
}

void fullyAssocHC(string in, ofstream& out){
	char* junk;
	int hotColdIndex = 0;
	int numTotal=0, numHits=0;
	string currentLine;
	ifstream inStream(in.c_str());
	long cache[512];
	bool valid[512];
	hotColdStruct lru[1024];
	for(int i=0; i<512; i++){
		cache[i] = 0;
		valid[i] = false;
	}
	for(int i=0; i<1024; i++){
		if(i < 511){
			lru[i].left = &lru[(2*i)+1];
			lru[i].right = &lru[(2*i)+2];
			lru[i].hc = 0;
		}
		else{
			lru[i].left = NULL;
			lru[i].right = NULL;
			lru[i].val = i-511;
		}
	}
	while(getline(inStream, currentLine)){
		hotColdStruct* cold = &lru[0];
		while(cold->left != NULL){
			if(cold->hc == 0){
				cold = cold->right;
			}
			else{
				cold = cold->left;
			}
		}
		hotColdIndex = cold->val;
		string saddress = currentLine.substr(4,8);
		long address = strtol(saddress.c_str(), &junk, 16);
		long block = address / 32;
		bool set = false;
		for(int i=0; i<512; i++){
			if(valid[i] == true && cache[i] == block){
				numHits++;
				set = true;
				//update hotColdIndex
				int index = i;
				hotColdStruct* current = &lru[0];
				for(int j=8; j>=0; j--){
					int numToSubtract = (int)pow(2.0, (double)j);
					int zeroOrOne = 0;
					if(index >= numToSubtract){
						index -= numToSubtract;
						zeroOrOne = 1;
					}
					if(zeroOrOne){
						current->hc = 1;
						current = current->right;
					}
					else{
						current->hc = 0;
						current = current->left;
					}
				}
				break;
			}
			else if(valid[i] == false){
				cache[i] = block;
				valid[i] = true;
				set = true; 
				//update hotColdIndex
				int index = i;
				hotColdStruct* current = &lru[0];
				for(int j=8; j>=0; j--){
					int numToSubtract = (int)pow(2.0, (double)j);
					int zeroOrOne = 0;
					if(index >= numToSubtract){
						index -= numToSubtract;
						zeroOrOne = 1;
					}
					if(zeroOrOne){
						current->hc = 1;
						current = current->right;
					}
					else{
						current->hc = 0;
						current = current->left;
					}
				}
				break;
			}
		}
		if(!set){
			int i = hotColdIndex;
			cache[hotColdIndex] = block;
			//update hotColdIndex
			int index = i;
			hotColdStruct* current = &lru[0];
			for(int j=8; j>=0; j--){
				int numToSubtract = (int)pow(2.0, (double)j);
				int zeroOrOne = 0;
				if(index >= numToSubtract){
					index -= numToSubtract;
					zeroOrOne = 1;
				}
				if(zeroOrOne){
					current->hc = 1;
					current = current->right;
				}
				else{
					current->hc = 0;
					current = current->left;
				}
			}
		}
		numTotal++;
	}
	out << numHits << "," << numTotal << "; "; 
}

void associativeNoWrite(string in, ofstream& out, int associativity){
	char* junk;
	int numTotal=0, numHits=0;
	string currentLine;
	ifstream inStream(in.c_str());
	cacheContent cache[16384/32/associativity][associativity];
	bool cacheValid[16384/32/associativity][associativity];
	for(int i=0; i<(16384/32/associativity); i++){
		for(int j=0; j<associativity; j++){
			cacheValid[i][j] = false;
		}
	}
	while(getline(inStream, currentLine)){
		bool found = false;
		string saddress = currentLine.substr(4,8);
		char loadOrStore = currentLine[0];
		long address = strtol(saddress.c_str(), &junk, 16);
		long block = address / 32;
		long index = block % (16384/32/associativity);
		cacheContent insert;
		insert.block = block;
		insert.lruInt = 0;
		cacheContent* cacheSet = cache[index];
		bool* validSet = cacheValid[index];
		for(int i=0; i<associativity; i++){
			if(validSet[i] == true){
				if(cacheSet[i].block == block){
					found = true;
					numHits++;
					cacheSet[i].lruInt = 0;
					continue;
				}
				cacheSet[i].lruInt++;
			}
		}
		if(!found && loadOrStore != 'S'){
			int biggest=0, indexToSwap=0;
			for(int i=0; i<associativity; i++){
				if(validSet[i] == true){
					if(cacheSet[i].lruInt > biggest){
						biggest = cacheSet[i].lruInt;
						indexToSwap = i;
					}
				}
				else{
					indexToSwap = i;
					break;
				}
			}
			cacheSet[indexToSwap] = insert;
			validSet[indexToSwap] = true;
		}
		numTotal++;
	}
	out << numHits << "," << numTotal << "; ";
}

void associativeNextLine(string in, ofstream& out, int associativity){
	char* junk;
	int numTotal=0, numHits=0;
	string currentLine;
	ifstream inStream(in.c_str());
	cacheContent cache[16384/32/associativity][associativity];
	bool cacheValid[16384/32/associativity][associativity];
	for(int i=0; i<(16384/32/associativity); i++){
		for(int j=0; j<associativity; j++){
			cacheValid[i][j] = false;
		}
	}
	while(getline(inStream, currentLine)){
		bool found = false;
		bool foundNext = false;
		string saddress = currentLine.substr(4,8);
		long address = strtol(saddress.c_str(), &junk, 16);
		long block = address / 32;
		long nextBlock = block + 1;
		long index = block % (16384/32/associativity);
		long nextIndex = nextBlock % (16384/32/associativity);
		cacheContent insert;
		insert.block = block;
		insert.lruInt = 0;
		cacheContent nextInsert;
		nextInsert.block = nextBlock;
		nextInsert.lruInt = 0;
		cacheContent* cacheSet = cache[index];
		cacheContent* nextCacheSet = cache[nextIndex];
		bool* validSet = cacheValid[index];
		bool* nextValidSet = cacheValid[nextIndex];
		for(int i=0; i<associativity; i++){
			if(validSet[i] == true){
				if(cacheSet[i].block == block){
					found = true;
					numHits++;
					cacheSet[i].lruInt = 0;
					continue;
				}
				cacheSet[i].lruInt++;
			}
		}
		for(int i=0; i<associativity; i++){
			if(nextValidSet[i] == true){
				if(nextCacheSet[i].block == nextBlock){
					foundNext = true;
					//numHits++;
					nextCacheSet[i].lruInt = 0;
					continue;
				}
				nextCacheSet[i].lruInt++;
			}
		}
		if(!found){
			int biggest=0, indexToSwap=0;
			for(int i=0; i<associativity; i++){
				if(validSet[i] == true){
					if(cacheSet[i].lruInt > biggest){
						biggest = cacheSet[i].lruInt;
						indexToSwap = i;
					}
				}
				else{
					indexToSwap = i;
					break;
				}
			}
			cacheSet[indexToSwap] = insert;
			validSet[indexToSwap] = true;
		}
		if(!foundNext){
			int biggest=0, indexToSwap=0;
			for(int i=0; i<associativity; i++){
				if(nextValidSet[i] == true){
					if(nextCacheSet[i].lruInt > biggest){
						biggest = nextCacheSet[i].lruInt;
						indexToSwap = i;
					}
				}
				else{
					indexToSwap = i;
					break;
				}
			}
			nextCacheSet[indexToSwap] = nextInsert;
			nextValidSet[indexToSwap] = true;
		}
		numTotal++;
	}
	out << numHits << "," << numTotal << "; ";
}

void associativeNextLineMiss(string in, ofstream& out, int associativity){
	char* junk;
	int numTotal=0, numHits=0;
	string currentLine;
	ifstream inStream(in.c_str());
	cacheContent cache[16384/32/associativity][associativity];
	bool cacheValid[16384/32/associativity][associativity];
	for(int i=0; i<(16384/32/associativity); i++){
		for(int j=0; j<associativity; j++){
			cacheValid[i][j] = false;
		}
	}
	while(getline(inStream, currentLine)){
		bool found = false;
		bool foundNext = false;
		string saddress = currentLine.substr(4,8);
		long address = strtol(saddress.c_str(), &junk, 16);
		long block = address / 32;
		long nextBlock = block + 1;
		long index = block % (16384/32/associativity);
		long nextIndex = nextBlock % (16384/32/associativity);
		cacheContent insert;
		insert.block = block;
		insert.lruInt = 0;
		cacheContent nextInsert;
		nextInsert.block = nextBlock;
		nextInsert.lruInt = 0;
		cacheContent* cacheSet = cache[index];
		cacheContent* nextCacheSet = cache[nextIndex];
		bool* validSet = cacheValid[index];
		bool* nextValidSet = cacheValid[nextIndex];
		for(int i=0; i<associativity; i++){
			if(validSet[i] == true){
				if(cacheSet[i].block == block){
					found = true;
					numHits++;
					cacheSet[i].lruInt = 0;
					continue;
				}
				cacheSet[i].lruInt++;
			}
		}
		if(!found){
			for(int i=0; i<associativity; i++){
				if(nextValidSet[i] == true){
					if(nextCacheSet[i].block == nextBlock){
						foundNext = true;
						//numHits++;
						nextCacheSet[i].lruInt = 0;
						continue;
					}
					nextCacheSet[i].lruInt++;
				}
			}
		}
		if(!found){
			int biggest=0, indexToSwap=0;
			for(int i=0; i<associativity; i++){
				if(validSet[i] == true){
					if(cacheSet[i].lruInt > biggest){
						biggest = cacheSet[i].lruInt;
						indexToSwap = i;
					}
				}
				else{
					indexToSwap = i;
					break;
				}
			}
			cacheSet[indexToSwap] = insert;
			validSet[indexToSwap] = true;
		}
		if(!foundNext && !found){
			int biggest=0, indexToSwap=0;
			for(int i=0; i<associativity; i++){
				if(nextValidSet[i] == true){
					if(nextCacheSet[i].lruInt > biggest){
						biggest = nextCacheSet[i].lruInt;
						indexToSwap = i;
					}
				}
				else{
					indexToSwap = i;
					break;
				}
			}
			nextCacheSet[indexToSwap] = nextInsert;
			nextValidSet[indexToSwap] = true;
		}
		numTotal++;
	}
	out << numHits << "," << numTotal << "; ";
}
