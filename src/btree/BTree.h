/*
 * BTree.h
 *
 *  Created on: Nov 12, 2019
 *      Author: gales
 */

#ifndef BTREE_BTREE_H_
#define BTREE_BTREE_H_

#include<iostream>
#include<iomanip>
#include<fstream>
#include<stack>
#include"../page/Page.h"

typedef int PAGE_OFFSET;


class BTree {
public:
	BTree();
	~BTree();

	bool isReady(){ return isLoaded;}
	void createBTree(std::string name, int d);
	void loadBTree(std::string name);

	// Basic operations
	Record SearchForRecord(int x);

	// Helpers (visualization)
	void printMainMem();
	void printIndex();

private:
	bool isLoaded;
	std::fstream metaDataFile;
	std::fstream indexFile;
	std::fstream mainMemoryFile;

	int d;
	int h;
	PAGE_OFFSET rootPageOffset;

	void saveMetaData();
	void loadMetaData();

	Page* loadPage(int offset);
	int savePage(Page* page);

	Record loadRecord(int offset);
	int saveRecord(Record record);

	int ReadRecord(int x);


	//TODO turn it into a cache h-sized
	Page* currPage;

};

#endif /* BTREE_BTREE_H_ */
