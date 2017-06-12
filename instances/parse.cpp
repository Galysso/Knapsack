#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

/** Parser for input files from the web-site of Zitzler
*/
int main(int argc, char * argv[]){
	if ( argc != 3) {
		cout << "Number of parameters incorrect : sourceFolder destinFolder\n";
		return -1;
	}
	char * sourceFolder = argv[1];
	char * destinFolder = argv[2];

	try{
		cout << "Reading data from " << sourceFolder << endl;
		ifstream infile;
		infile.open(sourceFolder, ios::in);
			
		stringstream strLine;
		string temp;
		getline(infile,temp);
		strLine.str(temp);
		temp = "";
	
		int nbItems = 0;
		int nbKnapsacks = 0;
		bool fg = true;
		while (!strLine.eof()) {
			strLine >> temp;
			if (atoi(temp.c_str()) != 0){
				if (fg) {
					nbKnapsacks = atoi(temp.c_str());
					fg = false;
				}
				nbItems = atoi(temp.c_str());
			}
		}			
	
		//KP& kpTmp = KP::create(kpName, nbItems, nbKnapsacks, nbKnapsacks);
		int capacity[nbKnapsacks];		
		int weight[nbKnapsacks][nbItems];
		int obj[nbKnapsacks][nbItems];	


		int curCapacity = 0;
		int curItem = 0;
		int curKnapsack = -1;
		while (!infile.eof()){
			getline(infile,temp);
			strLine.clear();
			strLine.str(temp);
			temp = "";
			while (!strLine.eof()) {
				strLine >> temp;
				if (temp.find("capacity") != string::npos){
					strLine >> temp;
					//kpTmp.Om[curCapacity] = atoi(temp.c_str());
					capacity[curCapacity] = atoi(temp.c_str());
					curCapacity++;
					curKnapsack++;
					curItem = 0;
				}
				if (temp.find("weight") != string::npos){
					strLine >> temp;
					//kpTmp.W.item(curKnapsack, curItem) = atoi(temp.c_str());
					weight[curKnapsack][curItem] = atoi(temp.c_str());
				}
				if (temp.find("profit") != string::npos){
					strLine >> temp;
					//kpTmp.C.item(curKnapsack, curItem) = atoi(temp.c_str());
					obj[curKnapsack][curItem] = atoi(temp.c_str());
					curItem++;
				}
			}
		}

		FILE *destin = fopen(destinFolder, "w");
		fprintf(destin, "%d\n", nbItems);
		fprintf(destin, "%d\n", capacity[0]);
		fprintf(destin, "%d\n", capacity[1]);
		for (int i = 0; i < nbItems; i++){
			fprintf(destin, "%d %d\t%d %d\n", obj[0][i], obj[1][i], weight[0][i], weight[1][i]);
		}

		// Close the input stream
		infile.close();
		//kp = &kpTmp;
   }catch(...){
		throw;
	}
	return 0;
};
