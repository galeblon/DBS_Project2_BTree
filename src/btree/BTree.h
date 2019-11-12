/*
 * BTree.h
 *
 *  Created on: Nov 12, 2019
 *      Author: gales
 */

#ifndef BTREE_BTREE_H_
#define BTREE_BTREE_H_

#include<fstream>

class BTree {
public:
	BTree();
	~BTree();

	bool isReady(){ return isLoaded;}

private:
	bool isLoaded;
	std::fstream indexFile;
	std::fstream mainMemoryFile;
};

#endif /* BTREE_BTREE_H_ */
