/*
 * BTree.cpp
 *
 *  Created on: Nov 12, 2019
 *      Author: gales
 */

#include "BTree.h"

BTree::BTree(){
	this->isLoaded = false;
	this->h = 0;
	this->d = 0;
	this->rootPage = 0;
}

BTree::~BTree(){
	this->indexFile.close();
	this->mainMemoryFile.close();
	this->metaDataFile.close();
}


void BTree::createBTree(std::string name, int d){
	if(this->isLoaded){
		throw new std::runtime_error("The BTree is already created");
	}

	std::string metaName = name;
	metaName.append("_meta");
	std::string indexName = name;
	indexName.append("_index");
	std::string mainmemName = name;
	mainmemName.append("_mainmem");

	this->d = d;
	this->h = 0;
	this->metaDataFile.open(metaName.c_str(), std::ios::out);
	this->metaDataFile.close();
	this->metaDataFile.open(metaName.c_str(), std::ios::in | std::ios::out | std::ios::binary);
	this->indexFile.open(indexName.c_str(), std::ios::out);
	this->indexFile.close();
	this->indexFile.open(indexName.c_str(), std::ios::in | std::ios::out | std::ios::binary);
	this->mainMemoryFile.open(mainmemName.c_str(), std::ios::out);
	this->mainMemoryFile.close();
	this->mainMemoryFile.open(mainmemName.c_str(), std::ios::in | std::ios::out | std::ios::binary);

	// Insert empty page
	this->rootPage = indexFile.tellg();
	Page rootPage(this->d);
	// Save page to indexFile
	// TODO create page insert method
	// FOR TESTING
	// 10 key/addr and 11 pointers to children
	// 32 * 4 =  128B
	int parent = NIL;
	int p[11] = {128, NIL, NIL, NIL, NIL};
	int x[10] =    {100, 120, 130, 140, NO_KEY};
	int a[10] =    { 16,  32,  48,  64};
	this->indexFile.write(reinterpret_cast<const char *>(&parent), sizeof(int));
	this->indexFile.write(reinterpret_cast<const char *>(x), 10*sizeof(int));
	this->indexFile.write(reinterpret_cast<const char *>(a), 10*sizeof(int));
	this->indexFile.write(reinterpret_cast<const char *>(p), 11*sizeof(int));

	int p2[11] = {NIL, NIL, NIL, NIL};
	int x2[10] =    {100, 120, 130, NO_KEY};
	int a2[10] =    { 16,  32,  48};
	int parent2 = 0;
	this->indexFile.write(reinterpret_cast<const char *>(&parent2), sizeof(int));
	this->indexFile.write(reinterpret_cast<const char *>(x2), 10*sizeof(int));
	this->indexFile.write(reinterpret_cast<const char *>(a2), 10*sizeof(int));
	this->indexFile.write(reinterpret_cast<const char *>(p2), 11*sizeof(int));
	this->indexFile.flush();
	// FOR TESTING

	// Insert all meta data
	saveMetaData();

	// TODO create record insert method
	// FOR TESTING
	int rec[4] = {100, 121, 140, 160};
	for(int i=0; i<10; i++)
		this->mainMemoryFile.write(reinterpret_cast<const char *>(rec), 4*sizeof(int));
	this->mainMemoryFile.flush();
	// FOR TESTING


	this->isLoaded = true;
}

void BTree::loadBTree(std::string name){
	if(this->isLoaded){
		throw new std::runtime_error("The BTree is already loaded");
	}

	std::string metaName = name;
	metaName.append("_meta");
	std::string indexName = name;
	indexName.append("_index");
	std::string mainmemName = name;
	mainmemName.append("_mainmem");

	this->metaDataFile.open(metaName.c_str(), std::ios::in | std::ios::out);// | std::ios::binary);
	this->indexFile.open(indexName.c_str(), std::ios::in | std::ios::out | std::ios::binary);
	this->mainMemoryFile.open(mainmemName.c_str(), std::ios::in | std::ios::out | std::ios::binary);

	if(!this->metaDataFile.good() || !this->indexFile.good() || !this->mainMemoryFile.good()){
		throw new std::runtime_error("Couldn't load all files properly");
	}

	// Load all meta data
	loadMetaData();

	this->isLoaded = true;
}

void BTree::saveMetaData(){
	this->metaDataFile.write(reinterpret_cast<const char *>(&this->d), sizeof(this->d));
	this->metaDataFile.write(reinterpret_cast<const char *>(&this->h), sizeof(this->h));
	this->metaDataFile.write(reinterpret_cast<const char *>(&this->rootPage), sizeof(this->rootPage));
	this->metaDataFile.flush();
}

void BTree::loadMetaData(){
	this->metaDataFile.seekg(0);
	this->metaDataFile.read(reinterpret_cast<char *>(&this->d), sizeof(this->d));
	this->metaDataFile.read(reinterpret_cast<char *>(&this->h), sizeof(this->h));
	this->metaDataFile.read(reinterpret_cast<char *>(&this->rootPage), sizeof(this->rootPage));
}

// This is a helper function, it doesn't contribute to I/O operations count,
// and doesn't change the state of the tree
void BTree::printMainMem(){
	if(!this->isLoaded){
		throw new std::runtime_error("No BTree is loaded");
	}

	std::cout << "Main Memory:\n"
			  << "┏━━━━━━━━━━┯━━━━━━━━━━┯━━━━━━━━━━┯━━━━━━━━━━┯━━━━━━━━━━┓\n"
			  << "┃   OFFSET │      KEY │        a │        b │        c ┃\n"
			  << "┣━━━━━━━━━━┿━━━━━━━━━━┿━━━━━━━━━━┿━━━━━━━━━━┿━━━━━━━━━━┫\n";
	this->mainMemoryFile.seekg(0);
	while(!this->mainMemoryFile.eof()){
		int record[4];
		int offset = this->mainMemoryFile.tellg();
		this->mainMemoryFile.read(reinterpret_cast<char *>(record), 4*sizeof(int));
		if(this->mainMemoryFile.eof())
			break;
		if(record[0] != NO_KEY){
			std::cout << "┃" << std::setw(10) << offset << "│" << std::setw(10)
			          << record[0] << "│" << std::setw(10) << record[1]
					  << "│" << std::setw(10) << record[2] << "│" << std::setw(10)
			 	      << record[3] << "┃\n";
		}
	}
	std::cout << "┗━━━━━━━━━━┷━━━━━━━━━━┷━━━━━━━━━━┷━━━━━━━━━━┷━━━━━━━━━━┛\n";

	this->mainMemoryFile.clear();
}

// This is a helper function, it doesn't contribute to I/O operations count,
// and doesn't change the state of the tree
void BTree::printIndex(){
	//TODO update stack with new pages found mid printing
	if(!this->isLoaded){
		throw new std::runtime_error("No BTree is loaded");
	}

	this->indexFile.seekg(0);

	std::stack<int> pages;
	pages.push(this->rootPage);

	int parent;
	int* p = new int[2*this->d+1];
	int* x = new int[2*d];
	int* a = new int[2*d];

	while(!pages.empty()){
		int page = pages.top();
		pages.pop();
		this->indexFile.read(reinterpret_cast<char *>(&parent), sizeof(int));
		this->indexFile.read(reinterpret_cast<char *>(x), (2*this->d)*sizeof(int));
		this->indexFile.read(reinterpret_cast<char *>(a), (2*this->d)*sizeof(int));
		this->indexFile.read(reinterpret_cast<char *>(p), (2*this->d+1)*sizeof(int));


		std::cout << "Main Memory:\n"
				  << "┏━━━━━━┯━━━━━━┯━━━━━━┯━━━━━━┓\n"
				  << "┃ PAGE │" << std::setw(6) << page <<"│PARENT│";
		if(parent == NIL)
			std::cout << "  NIL ";
		else
			std::cout << std::setw(6) << parent;
		std::cout << "┃\n";
		std::cout << "┣━━━━━━┿━━━━━━┿━━━━━━┿━━━━━━╇";
		for(int i=4; i<6*d; i++){
			std::cout << "━━━━━━┯";
		}
		std::cout << "━━━━━━┓";
		std::cout << "\n";
		std::cout << "┃";
		for(int i=0; i<2*d; i++){
			std::cout <<  "   p" << std::setw(2) << i << "│" << "   x" << std::setw(2) << i+1
					  << "│" << "   a" << std::setw(2) << i+1 << "│";
		}
		std::cout << "   p" << std::setw(2) << 2*d << "┃\n";
		std::cout << "┃";
		bool reached_trash = false;
		for(int i=0; i<2*d; i++){
			if(x[i] == NO_KEY){
				if(!reached_trash){
					reached_trash = true;
					if(p[i] == NIL)
						std::cout << "  NIL ";
					else
						std::cout << std::setw(6) << p[i];
					std::cout <<"│██████│██████│";
				} else
					std::cout << "██████│██████│██████│";
			} else{
				if(p[i] == NIL)
					std::cout << "  NIL ";
				else
					std::cout << std::setw(6) << p[i];
				std::cout << "│" << std::setw(6) << x[i]
						  << "│" << std::setw(6) << a[i] << "│";
			}
		}
		if(reached_trash)
			std::cout << "██████┃\n";
		else
			std::cout << std::setw(6) << p[2*d] << "┃\n";

		std::cout << "┗";
		for(int i=0; i<6*d; i++)
			std::cout << "━━━━━━┷";
		std::cout << "━━━━━━┛\n";
	}

	this->indexFile.seekg(0);
}
