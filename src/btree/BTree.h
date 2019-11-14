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
	PAGE_OFFSET rootPage;

	void saveMetaData();
	void loadMetaData();

};

#endif /* BTREE_BTREE_H_ */
