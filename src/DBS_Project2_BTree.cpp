//============================================================================
// Name        : DBS_Project2_BTree.cpp
// Author      : galeblon
// Version     :
// Copyright   : Free as in green pepper
// Description : Managing simple records in persistent B-Tree
//============================================================================

#include <iostream>
#include "btree/BTree.h"

void printHelp();
bool parseAction(char action, BTree& tree);

int main() {
	char action = ' ';
	bool programFinished = false;
	BTree tree;

	std::cout << "Enter H to display help page.\n";
	while(!programFinished){
		std::cin >> action;
		programFinished = parseAction(action, tree);
		std::cin.clear();
	}

	std::cout << "Goodbye.\n";
	return 0;
}



void printHelp(){
	std::cout << "Valid Instructions:\n"
			  << "\tL {Name}      - Loads database from file\n"
			  << "\tC {Name} {d}  - Creates new BTree with d param, saved in file named {Name}\n"
			  << "\tI {K} {V}     - Inserts record with key {K} and value {V} into database\n"
			  << "\tU {K} {nV}    - Updates record with {K} key with {nV} value\n"
			  << "\tD {K}         - Removes record with {K} key\n"
			  << "\tR {K}         - Read record with {K} key\n"
			  << "\tA             - Read all records in order\n"
			  << "\tX             - Display Index file (formatted)\n"
			  << "\tM             - Display Main memory file (formatted)\n"
			  << "\tQ             - Quit program\n"
			  << "\tH             - Display this message\n";
}


bool parseAction(char action, BTree& tree){
	std::string valStr;
	int valI1, valI2, valI3, valI4;
	try{
		switch(toupper(action)){
			case 'L':
				//std::cout << "Loading db operation here\n";
				std::cin >> valStr;
				tree.loadBTree(valStr);
				break;
			case 'C':
				//std::cout << "Creating db operation here\n";
				std::cin >> valStr >> valI1;
				tree.createBTree(valStr, valI1);
				break;
			case 'I':{
				//std::cout << "Record insert operation here\n";
				std::cin >> valI1 >> valI2 >> valI3 >> valI4;
				Record rec(valI1, valI2, valI3, valI4);
				int res = tree.InsertRecord(rec);
				if(res == ALREADY_EXISTS){
					std::cout << "\tRecord already exists\n";
				} else if(res == OK){
					std::cout << "\tRecord insert OK\n";
				}
			}
				break;
			case 'U':
				std::cout << "Record update operation here\n";
				break;
			case 'D':
				std::cout << "Record remove operation here\n";
				break;
			case 'R':
				//std::cout << "Record read operation here\n";
				std::cin >> valI1;
				tree.SearchForRecord(valI1);
				break;
			case 'A':
				std::cout << "All records read operation here\n";
				break;
			case 'X':
				//std::cout << "Display index file operation here\n";
				tree.printIndex();
				break;
			case 'M':
				//std::cout << "Display main memory file operation here\n";
				tree.printMainMem();
				break;
			case 'Q':
				return true;
			case 'H':
				printHelp();
				break;
			default:
				std::cout << '\"' << action << "\" is undefined action.\n";
				printHelp();
		}
	} catch (std::exception* ex){
		std::cout << "Error parsing command: " << ex->what() << '\n';
	}
	return false;
}
