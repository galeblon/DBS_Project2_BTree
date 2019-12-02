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
		tree.ResetIOCounters();
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
			// Loading db operation
			case 'L':
				std::cin >> valStr;
				tree.LoadBTree(valStr);
				break;
			// Creating db operation
			case 'C':
				std::cin >> valStr >> valI1;
				tree.CreateBTree(valStr, valI1);
				break;
			// Record insert operation
			case 'I':{
				std::cin >> valI1 >> valI2 >> valI3 >> valI4;
				Record rec(valI1, valI2, valI3, valI4);
				int res = tree.InsertRecord(rec);
				if(res == ALREADY_EXISTS){
					std::cout << "\tRecord already exists\n";
				} else if(res == OK){
					std::cout << "\tRecord insert OK\n";
				}
				tree.PrintIOStatistics();
			}
				break;
			// Record update operation
			case 'U':{
				std::cin >> valI1 >> valI2 >> valI3 >> valI4;
				Record rec(valI1, valI2, valI3, valI4);
				int res = tree.UpdateRecord(rec);
				if(res == NOT_FOUND){
					std::cout << "\tNo Record with such key\n";
				} else if (res == OK){
					std::cout << "\tRecord update OK\n";
				}
				tree.PrintIOStatistics();
			}
				break;
			// Record remove operation
			case 'D':{
				std::cin >> valI1;
				int res = tree.RemoveRecord(valI1);
				if(res == NOT_FOUND){
					std::cout << "\tNo Record with such key\n";
				} else if (res == OK){
					std::cout << "\tRecord removal OK\n";
				}
				tree.PrintIOStatistics();
			}
				break;
			// Record read operation
			case 'R':
				std::cin >> valI1;
				tree.SearchForRecord(valI1);
				tree.PrintIOStatistics();
				break;
			// All records read operation
			case 'A':
				tree.SequentialRead();
				tree.PrintIOStatistics();
				break;
			// Display index file content operation
			case 'X':
				tree.PrintIndex();
				break;
			// Display main memory file content operation
			case 'M':
				tree.PrintMainMem();
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
