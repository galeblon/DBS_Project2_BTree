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
	this->currPageOffset = NIL;
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
		else{
			if(p[2*d] == NIL)
			std::cout << "  NIL ┃\n";
		else{
			pages.push(p[2*d]);
			std::cout << std::setw(6) << p[2*d];
		}

		}
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
	for(int i=2*d+1; i<4*d+1; i++)
		lPage->a[i-2*d-1] = buffer[i];
	for(int i=4*d+1; i<6*d+2; i++)
		lPage->p[i-4*d-1] = buffer[i];

	this->currPageOffset = offset;
	this->currPage = lPage;

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

	this->indexFile.seekg(0, indexFile.end);
	int offset = this->indexFile.tellg();
	this->indexFile.write(reinterpret_cast<const char *>(buffer), (6*d+2)*sizeof(int));
	this->indexFile.flush();

	delete[] buffer;
	return offset;
}

void BTree::updatePage(int offset, Page* page){
	int* buffer = new int[6*this->d+2];
	buffer[0] = page->parent;
	for(int i=1; i<2*d+1; i++)
		buffer[i] = page->x[i-1];
	for(int i=2*d+1; i<4*d+1; i++)
		buffer[i] = page->a[i-2*d-1];
	for(int i=4*d+1; i<6*d+2; i++)
		buffer[i] = page->p[i-4*d-1];

	this->indexFile.seekg(offset);
	this->indexFile.write(reinterpret_cast<const char *>(buffer), (6*d+2)*sizeof(int));
	this->indexFile.flush();

	delete[] buffer;
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

	this->mainMemoryFile.seekg(0, mainMemoryFile.end);
	int offset = this->mainMemoryFile.tellg();
	this->mainMemoryFile.write(reinterpret_cast<const char *>(buffer), 4*sizeof(int));
	this->mainMemoryFile.flush();
	this->mainMemoryFile.clear();

	return offset;
	delete[] buffer;
}



int BTree::ReadRecord(int x){
	int s = this->rootPageOffset;
	//Page* page;

	while(true){
		if(s == NIL)
			return NOT_FOUND;
		//page = loadPage(s)
		delete this->currPage;
		this->currPage = loadPage(s);

		int m = currPage->getM();
		if(!m)
			return NOT_FOUND;
		if(x < currPage->x[0]){
			s = currPage->p[0];
			continue;
		}
		for(int i=0; i<m; i++){
			//if(currPage->x[i] == NO_KEY && i == 0 ){
			//	return NOT_FOUND;
			//}
			if(currPage->x[i] == x){
				int res = currPage->a[i];
				return res;
			}
			if(i > 0 && x > currPage->x[i-1] && x < currPage->x[i]){
				s = currPage->p[i];
				continue;
			}
		}
		if(x > currPage->x[m-1]){
			s = currPage->p[m];
			continue;
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

// TODO return early if not connected to db
int BTree::InsertRecord(Record rec){
	int res = ReadRecord(rec.getKey());
	if(res != NOT_FOUND)
		return ALREADY_EXISTS;
	int offset = saveRecord(rec);
	int cpointer = NIL;
	while(true){
		int m = this->currPage->getM();
		if(m < 2*d){
			int key_temp;
			int offset_temp;
			int cpointer_temp;
			for(int i=0; i<2*d; i++){
				if(currPage->x[i] == NO_KEY){
					currPage->x[i] = rec.getKey();
					currPage->a[i] = offset;
					currPage->p[i+1] = cpointer;
					break;
				}
				if(currPage->x[i] == rec.getKey()){
					return OK; // When created new root
				}
				if(rec.getKey() < currPage->x[i]){
					key_temp = currPage->x[i];
					offset_temp = currPage->a[i];
					cpointer_temp = currPage->p[i+1];
					currPage->x[i] = rec.getKey();
					currPage->a[i] = offset;
					currPage->p[i+1] = cpointer;
					offset = offset_temp;
					rec.setKey(key_temp);
					cpointer = cpointer_temp;
				}
			}
			// TODO when caching works, this wont be necessary
			updatePage(currPageOffset, currPage);
			return OK;
		}
		int res = tryCompensation(rec, offset, cpointer);
		if(res == COMPENSATION_NOT_POSSIBLE){
			cpointer = split(rec, offset, cpointer);
			// TODO if parent is nill we will have to create new page
			int parent = this->currPage->parent;
			delete this->currPage;
			loadPage(parent);
		} else{
			return res;
		}
	}
	return 1;
}

// TODO fix it, some memory leak
int BTree::tryCompensation(Record rec, int recordOffset, int nPOffset){
	int result = COMPENSATION_NOT_POSSIBLE;
	bool done = false;
	Page *overflowPage = this->currPage;
	Page *parent, *leftSibling, *rightSibling;
	parent = leftSibling = rightSibling = NULL;
	int overflowPageOffset = this->currPageOffset;
	if(overflowPage->parent == NIL)
		return COMPENSATION_NOT_POSSIBLE;
	loadPage(overflowPage->parent);
	parent = this->currPage;
	int parentOffset = this->currPageOffset;
	int pIndex=0;
	// It's his parent he has to be here somewhere
	for(pIndex=0; pIndex<2*d+1;pIndex++){
		if(this->currPage->p[pIndex] == overflowPageOffset)
			break;
	}
	int leftSiblingOffset = pIndex > 0 ? this->currPage->p[pIndex-1] : NIL;
	int rightSiblingOffset = pIndex < 2*d ? this->currPage->p[pIndex+1] : NIL;

	// Check with left sibling first
	if(leftSiblingOffset != NIL){
		Page* leftSibling = loadPage(leftSiblingOffset);
		int m = leftSibling->getM();
		if(m<2*d){
			// Possible compensation
			distribute(overflowPage, leftSibling, parent, rec, recordOffset, nPOffset, pIndex, true);
			done = true;
			result = OK;
			updatePage(leftSiblingOffset, leftSibling);
		}

	}
	// Then right sibling
	if(rightSiblingOffset != NIL && !done){
		Page* rightSibling = loadPage(rightSiblingOffset);
		int m = rightSibling->getM();
		if(m<2*d){
			// Possible compensation
			distribute(overflowPage, rightSibling, parent, rec, recordOffset, nPOffset, pIndex, false);
			result = OK;
			done = true;
			updatePage(rightSiblingOffset, rightSibling);
		}
	}
	if(done){
		updatePage(overflowPageOffset, overflowPage);
		updatePage(parentOffset, parent);
	}
	this->currPage = overflowPage;
	this->currPageOffset = overflowPageOffset;
	if(leftSibling != NULL)
		delete leftSibling;
	if(rightSibling != NULL)
		delete rightSibling;
	if(parent != NULL)
		delete parent;
	return result;
}

int BTree::split(Record& rec, int& recordOffset, int nPOffset){
	Page* overflowPage = this->currPage;
	int overFlowPageOffset = this->currPageOffset;
	Page* newPage = new Page(this->d);
	newPage->parent = overflowPage->parent;
	int newPageOffset = savePage(newPage);

	distributeSplit(overflowPage, newPage, rec, recordOffset, nPOffset);

	if(newPage->parent == NIL){
		this->h++;
		Page* newRoot = new Page(d);
		newRoot->x[0] = rec.getKey();
		newRoot->a[0] = recordOffset;
		newRoot->p[0] = overFlowPageOffset;
		newRoot->p[1] = newPageOffset;
		int newRootOffset = savePage(newRoot);
		newPage->parent = newRootOffset;
		overflowPage->parent = newRootOffset;
		this->rootPageOffset = newRootOffset;

	}
	updatePage(overFlowPageOffset, overflowPage);
	updatePage(newPageOffset, newPage);

	delete newPage;
	return newPageOffset;
}

void BTree::distribute(Page* ovP, Page* sbP, Page* pP, Record rec, int recordOffset, int nPOffset, int parentIndex, bool left){
	// Oveflow has 2*d, sibling has m, one from parent and we also insert 1 record
	int toDistribute = 2*d + sbP->getM() + 1 + 1;
	int* x = new int[toDistribute];
	int* a = new int[toDistribute];
	int* p = new int[toDistribute+1];

	int rIndex = 0;
	if(left){
		// Fill the start with sibling
		int sbM = sbP->getM();
		for(int i=0; i<sbM; i++, rIndex++){
			x[rIndex] = sbP->x[rIndex];
			a[rIndex] = sbP->a[rIndex];
			p[rIndex] = sbP->p[rIndex];
			sbP->x[rIndex] = NO_KEY;
			sbP->a[rIndex] = NIL;
			sbP->p[rIndex] = NIL;
		}
		p[rIndex] = sbP->p[sbM];
		sbP->p[sbM] = NIL;

		// Insert the parent
		x[rIndex] = pP->x[parentIndex-1];
		a[rIndex] = pP->a[parentIndex-1];
		rIndex++;
	}

	int newRecIndex = 0;
	// find index where new recod should be inserted
	for(; newRecIndex<2*d; newRecIndex++)
		if(ovP->x[newRecIndex] > rec.getKey())
			break;

	// We can rewrite all up to conflicting record
	for(int i=0; i<newRecIndex; i++){
		x[rIndex + i] = ovP->x[i];
		a[rIndex + i] = ovP->a[i];
		p[rIndex + i] = ovP->p[i];
		ovP->x[i] = NO_KEY;
		ovP->a[i] = NIL;
		ovP->p[i] = NIL;
	}
	// Insert the new record
	if(left){
		x[rIndex + newRecIndex] = rec.getKey();
		a[rIndex + newRecIndex] = recordOffset;
		p[rIndex + newRecIndex] = ovP->p[newRecIndex];
		p[rIndex + newRecIndex + 1] = nPOffset;
		rIndex++;
		ovP->p[rIndex] = NIL;
	} else {
		x[rIndex + newRecIndex] = rec.getKey();
		a[rIndex + newRecIndex] = recordOffset;
		p[rIndex + newRecIndex] = ovP->p[rIndex + newRecIndex];
		p[rIndex + newRecIndex + 1] = nPOffset;
		rIndex++;
		ovP->p[rIndex + newRecIndex] = NIL;
	}

	// Insert the rest from the overflow page
	for(;newRecIndex<2*d; newRecIndex++){
		x[rIndex+newRecIndex] = ovP->x[newRecIndex];
		a[rIndex+newRecIndex] = ovP->a[newRecIndex];
		p[rIndex+newRecIndex+1] = ovP->p[newRecIndex+1];
		ovP->x[newRecIndex] = NO_KEY;
		ovP->a[newRecIndex] = NIL;
		ovP->p[newRecIndex+1] = NIL;
	}

	if(!left){
		// insert the parent
		x[newRecIndex+rIndex] = pP->x[parentIndex];
		a[newRecIndex+rIndex] = pP->a[parentIndex];
		rIndex++;
		// Fill the rest with sibling
		int sbM = sbP->getM();
		for(int i=0; i<sbM; i++, rIndex++){
			x[newRecIndex + rIndex] = sbP->x[i];
			a[newRecIndex + rIndex] = sbP->a[i];
			p[newRecIndex + rIndex] = sbP->p[i];
			sbP->x[i] = NO_KEY;
			sbP->a[i] = NIL;
			sbP->p[i] = NIL;
		}
		p[newRecIndex + rIndex] = sbP->p[sbM];
		sbP->p[sbM] = NIL;
	}

	// Get the middle element
	int middle = toDistribute/2;
	//Write him to parent
	if(left){
		pP->x[parentIndex-1] = x[middle];
		pP->a[parentIndex-1] = a[middle];
	} else{
		pP->x[parentIndex] = x[middle];
		pP->a[parentIndex] = a[middle];
	}
	// Fill back overflow
	if(!left){
		int i;
		for(i=0; i<middle; i++){
			ovP->x[i] = x[i];
			ovP->a[i] = a[i];
			ovP->p[i] = p[i];
		}
		ovP->p[i+1] = p[i+1];
		for(int j=middle+1, i=0; j<toDistribute; j++, i++){
			sbP->x[i] = x[j];
			sbP->a[i] = a[j];
			sbP->p[i] = p[j];
		}
		sbP->p[i+1] = p[toDistribute];
	} else {
		int i;
		for(i=0; i<middle; i++){
			sbP->x[i] = x[i];
			sbP->a[i] = a[i];
			sbP->p[i] = p[i];
		}
		sbP->p[i+1] = p[i+1];
		for(int j=middle+1, i=0; j<toDistribute; j++, i++){
			ovP->x[i] = x[j];
			ovP->a[i] = a[j];
			ovP->p[i] = p[j];
		}
		ovP->p[i+1] = p[toDistribute];
	}

	delete[] x;
	delete[] a;
	delete[] p;
}

void BTree::distributeSplit(Page* ovP, Page* sbP, Record& rec, int& recordOffset, int nPOffset){
	// Oveflow has 2*d, sibling has 0, and we also insert 1 record
	int toDistribute = 2*d + 1;
	int* x = new int[toDistribute];
	int* a = new int[toDistribute];
	int* p = new int[toDistribute+1];

	// find index where new recod should be inserted
	int rIndex = 0;
	for(; rIndex<2*d; rIndex++)
		if(ovP->x[rIndex] > rec.getKey())
			break;

	// We can rewrite all up to conflicting record
	for(int i=0; i<rIndex; i++){
		x[i] = ovP->x[i];
		a[i] = ovP->a[i];
		p[i] = ovP->p[i];
		ovP->x[i] = NO_KEY;
		ovP->a[i] = NIL;
		ovP->p[i] = NIL;
	}
	// Insert the new record
	x[rIndex] = rec.getKey();
	a[rIndex] = recordOffset;
	p[rIndex] = ovP->p[rIndex];
	p[rIndex+1] = nPOffset;
	rIndex++;
	ovP->p[rIndex] = NIL;

	// Insert the rest from the overflow page
	for(;rIndex-1<2*d; rIndex++){
		x[rIndex] = ovP->x[rIndex-1];
		a[rIndex] = ovP->a[rIndex-1];
		p[rIndex+1] = ovP->p[rIndex-1+1];
		ovP->x[rIndex-1] = NO_KEY;
		ovP->a[rIndex-1] = NIL;
		ovP->p[rIndex-1+1] = NIL;
	}

	// Get the middle element
	int middle = toDistribute/2;
	// He will be returned by the function
	rec.setKey(x[middle]);
	recordOffset = a[middle];
	// TODO check if nPOffset also needs to be returned
	// Fill back overflow
	int i;
	for(i=0; i<middle; i++){
		ovP->x[i] = x[i];
		ovP->a[i] = a[i];
		ovP->p[i] = p[i];
	}
	ovP->p[i+1] = p[i+1];
	// Fill new page
	for(int j=middle+1, i=0; j<toDistribute; j++, i++){
		sbP->x[i] = x[j];
		sbP->a[i] = a[j];
		sbP->p[i] = p[j];
	}
	sbP->p[i+1] = p[toDistribute];

	delete[] x;
	delete[] a;
	delete[] p;
}






