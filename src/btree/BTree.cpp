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
	this->rootPageOffset = 0;
	this->currPage = NULL;
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
	this->currPage = new Page(this->d);
	// Save page to indexFile
	this->rootPageOffset = this->savePage(this->currPage);

	// Insert all meta data
	saveMetaData();

	// TODO create record insert method
	// FOR TESTING
	//int rec[4] = {100, 121, 140, 160};
	//for(int i=0; i<10; i++)
	//	this->mainMemoryFile.write(reinterpret_cast<const char *>(rec), 4*sizeof(int));
	//this->mainMemoryFile.flush();
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
	this->metaDataFile.write(reinterpret_cast<const char *>(&this->rootPageOffset), sizeof(this->rootPageOffset));
	this->metaDataFile.flush();
}

void BTree::loadMetaData(){
	this->metaDataFile.seekg(0);
	this->metaDataFile.read(reinterpret_cast<char *>(&this->d), sizeof(this->d));
	this->metaDataFile.read(reinterpret_cast<char *>(&this->h), sizeof(this->h));
	this->metaDataFile.read(reinterpret_cast<char *>(&this->rootPageOffset), sizeof(this->rootPageOffset));
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
// Displays all BTree pages DFS style.
void BTree::printIndex(){
	//TODO optimize code to avoid repetable sections of code
	if(!this->isLoaded){
		throw new std::runtime_error("No BTree is loaded");
	}

	this->indexFile.seekg(0);

	std::stack<int> pages;
	pages.push(this->rootPageOffset);

	int parent;
	int* p = new int[2*this->d+1];
	int* x = new int[2*d];
	int* a = new int[2*d];

	std::cout << "Tree Pages Memory:\n";
	while(!pages.empty()){
		int page = pages.top();
		pages.pop();
		this->indexFile.seekg(page);
		this->indexFile.read(reinterpret_cast<char *>(&parent), sizeof(int));
		this->indexFile.read(reinterpret_cast<char *>(x), (2*this->d)*sizeof(int));
		this->indexFile.read(reinterpret_cast<char *>(a), (2*this->d)*sizeof(int));
		this->indexFile.read(reinterpret_cast<char *>(p), (2*this->d+1)*sizeof(int));

		std::cout << "┏━━━━━━┯━━━━━━┯━━━━━━┯━━━━━━┓\n"
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
		bool first_el = true;
		bool reached_trash = false;
		for(int i=0; i<2*d; i++){
			if(x[i] == NO_KEY){
				if(!reached_trash && !first_el){
					if(p[i] == NIL)
						std::cout << "  NIL ";
					else{
						pages.push(p[i]);
						std::cout << std::setw(6) << p[i];
					}
					std::cout <<"│██████│██████│";
				} else
					std::cout << "██████│██████│██████│";
				reached_trash = true;
			} else{
				if(p[i] == NIL)
					std::cout << "  NIL ";
				else{
					pages.push(p[i]);
					std::cout << std::setw(6) << p[i];
				}
				std::cout << "│" << std::setw(6) << x[i]
						  << "│" << std::setw(6) << a[i] << "│";
			}
			first_el = false;
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
	delete[] p;
	delete[] x;
	delete[] a;
}

Page* BTree::loadPage(int offset){
	// TODO ask cache first

	// Load from index memory
	Page* lPage = new Page(this->d);
	int* buffer = new int[6*this->d+2];
	this->indexFile.seekg(offset);

	this->indexFile.read(reinterpret_cast<char *>(buffer), (6*this->d+2)*sizeof(int));
	lPage->d = this->d;
	lPage->parent = buffer[0];
	for(int i=1; i<2*d+1; i++)
		lPage->x[i-1] = buffer[i];
	for(int i=2*d+2; i<4*d+1; i++)
		lPage->a[i-2*d-2] = buffer[i];
	for(int i=4*d+2; i<6*d+2; i++)
		lPage->p[i-4*d-2] = buffer[i];

	delete[] buffer;
	return lPage;
}

int BTree::savePage(Page* page){
	int* buffer = new int[6*this->d+2];
	buffer[0] = page->parent;
	for(int i=1; i<2*d+1; i++)
		buffer[i] = page->x[i-1];
	for(int i=2*d+1; i<4*d+1; i++)
		buffer[i] = page->a[i-2*d-1];
	for(int i=4*d+1; i<6*d+2; i++)
		buffer[i] = page->p[i-4*d-1];

	this->indexFile.seekg(indexFile.end);
	int offset = this->indexFile.tellg();
	this->indexFile.write(reinterpret_cast<const char *>(buffer), (6*d+2)*sizeof(int));
	this->indexFile.flush();

	delete[] buffer;
	return offset;
}

Record BTree::loadRecord(int offset){
	//TODO ask cache first
	Record record;
	int* buffer = new int[4];
	this->mainMemoryFile.seekg(offset);

	this->mainMemoryFile.read(reinterpret_cast<char *>(buffer), 4*sizeof(int));
	record.setKey(buffer[0]);
	record.setABC(buffer[1], buffer[2], buffer[3]);

	delete[] buffer;
	return record;
}

int BTree::saveRecord(Record record){
	int* buffer = new int[4];
	buffer[0] = record.getKey();
	buffer[1] = record.getA();
	buffer[2] = record.getB();
	buffer[3] = record.getC();

	this->mainMemoryFile.seekg(mainMemoryFile.end);
	int offset = this->mainMemoryFile.tellg();
	this->mainMemoryFile.write(reinterpret_cast<const char *>(buffer), 4*sizeof(int));
	this->mainMemoryFile.flush();

	return offset;
	delete[] buffer;
}



int BTree::ReadRecord(int x){
	int s = this->rootPageOffset;
	Page* page;

	while(true){
		if(s == NIL)
			return NOT_FOUND;
		page = loadPage(s);
		if(x < page->x[0]){
			s = page->p[0];
			continue;
		}
		if(x > page->x[2*d]){
			s = page->p[2*d+1];
			continue;
		}
		for(int i=0; i<2*d-1; i++){
			if(page->x[i] == NO_KEY){
				delete page;
				return NOT_FOUND;
			}
			if(page->x[i] == x){
				int res = page->a[i];
				delete page;
				return res;
			}
			if(x > page->x[i] && x < page->x[i+1]){
				s = page->p[i+1];
				continue;
			}
		}
	}
}

Record BTree::SearchForRecord(int x){
	int offset = ReadRecord(x);
	if(offset == NOT_FOUND){
		Record rec;
		rec.print(offset);
		return Record();
	} else {
		Record rec = this->loadRecord(offset);
		rec.print(offset);
		return rec;
	}
}


