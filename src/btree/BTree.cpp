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
	this->metaDataFile.open(metaName.c_str(), std::ios::in | std::ios::out);// | std::ios::binary);
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
	// TODO

	// Insert all meta data
	this->metaDataFile.write(reinterpret_cast<const char *>(&this->d), sizeof(this->d));
	this->metaDataFile.write(reinterpret_cast<const char *>(&this->h), sizeof(this->h));
	this->metaDataFile.write(reinterpret_cast<const char *>(&this->rootPage), sizeof(this->rootPage));
	this->metaDataFile.flush();

	this->isLoaded = true;
}
