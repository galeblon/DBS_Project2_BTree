/*
 * BTree.h
 *
 *  Created on: Nov 12, 2019
 *      Author: gales
 */

#ifndef BTREE_BTREE_H_
#define BTREE_BTREE_H_

#include<fstream>
#include"../page/Page.h"

typedef int PAGE_OFFSET;

class BTree {
public:
	BTree();
	~BTree();

	bool isReady(){ return isLoaded;}
	void createBTree(std::string name, int d);

private:
	bool isLoaded;
	std::fstream metaDataFile;
	std::fstream indexFile;
	std::fstream mainMemoryFile;

	int d;
	int h;
	PAGE_OFFSET rootPage;

};

#endif /* BTREE_BTREE_H_ */
