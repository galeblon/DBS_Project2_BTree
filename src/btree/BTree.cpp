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
	this->pageCache = new Cache(DEFAULT_CACHE_SIZE, this);
	diskReadIndexMemory = 0;
	diskReadMainMemory = 0;
	diskWriteIndexMemory = 0;
	diskWriteMainMemory = 0;
}

BTree::~BTree(){
	// It also saves all pending changes into file.
	delete pageCache;
	std::cout << "Saved all cached data.\n";
	saveMetaData();
	std::cout << "Saved all meta data.\n";

	this->indexFile.close();
	this->mainMemoryFile.close();
	this->metaDataFile.close();
}


void BTree::CreateBTree(std::string name, int d){
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

	this->isLoaded = true;
}

void BTree::LoadBTree(std::string name){
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

	// Expand the cache
	pageCache->ExpandCache(h);

	loadPage(rootPageOffset);

	this->isLoaded = true;
}

void BTree::saveMetaData(){
	this->metaDataFile.seekg(0);
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
void BTree::PrintMainMem(){
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
void BTree::PrintIndex(){
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
		Page* cachedPage = pageCache->FindPage(page, false);
		if(cachedPage != NULL){
			std::copy(cachedPage->x, cachedPage->x+2*d, x);
			std::copy(cachedPage->a, cachedPage->a+2*d, a);
			std::copy(cachedPage->p, cachedPage->p+2*d+1, p);
			parent = cachedPage->parent;
		} else {
			this->indexFile.seekg(page);
			this->indexFile.read(reinterpret_cast<char *>(&parent), sizeof(int));
			this->indexFile.read(reinterpret_cast<char *>(x), (2*this->d)*sizeof(int));
			this->indexFile.read(reinterpret_cast<char *>(a), (2*this->d)*sizeof(int));
			this->indexFile.read(reinterpret_cast<char *>(p), (2*this->d+1)*sizeof(int));
		}

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
			std::cout << std::setw(6) << p[2*d] << "┃\n";
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

Page* BTree::loadPage(int offset, bool willBeDirty){
	// Ask cache first
	Page* lPage = this->pageCache->FindPage(offset, willBeDirty);
	if(lPage != NULL){
		this->currPage = lPage;
		this->currPageOffset = offset;
		return lPage;
	}

	// Load from index memory
	diskReadIndexMemory++;

	lPage = new Page(this->d);
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

	this->pageCache->CachePage(lPage, offset, willBeDirty);
	delete[] buffer;
	return lPage;
}

int BTree::savePage(Page* page){
	diskWriteIndexMemory++;

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

	this->pageCache->CachePage(page, offset);

	delete[] buffer;
	return offset;
}

void BTree::updatePage(int offset, Page* page, bool skipCache){
	if(!skipCache && pageCache->IsCached(page)){
		return;
	}

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
	diskReadMainMemory++;

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
	diskWriteMainMemory++;

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

	delete[] buffer;
	return offset;
}

void BTree::updateRecord(int offset, Record record){
	diskWriteMainMemory++;

	int* buffer = new int[4];
	buffer[0] = record.getKey();
	buffer[1] = record.getA();
	buffer[2] = record.getB();
	buffer[3] = record.getC();

	this->mainMemoryFile.seekg(offset);
	this->mainMemoryFile.write(reinterpret_cast<const char *>(buffer), 4*sizeof(int));
	this->mainMemoryFile.flush();
	this->mainMemoryFile.clear();

	delete[] buffer;
}


int BTree::readRecord(int x, int startPage){
	int s;
	if(startPage == NIL)
		s = this->rootPageOffset;
	else
		s = startPage;

	while(true){
		if(s == NIL)
			return NOT_FOUND;
		loadPage(s);

		int m = currPage->getM();
		if(!m)
			return NOT_FOUND;
		if(x < currPage->x[0]){
			s = currPage->p[0];
			continue;
		}
		for(int i=0; i<m; i++){
			if(currPage->x[i] == x){
				int res = currPage->a[i];
				return res;
			}
			if(i > 0 && x > currPage->x[i-1] && x < currPage->x[i]){
				s = currPage->p[i];
				break;
			}
		}
		if(x > currPage->x[m-1]){
			s = currPage->p[m];
			continue;
		}
	}
}

Record BTree::SearchForRecord(int x){
	if(!this->isLoaded){
		throw new std::runtime_error("Not connected to any BTree");
	}
	int offset = readRecord(x);
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

int BTree::UpdateRecord(Record rec){
	if(!this->isLoaded){
		throw new std::runtime_error("Not connected to any BTree");
	}
	int offset = readRecord(rec.getKey());
	if(offset == NOT_FOUND)
		return NOT_FOUND;
	updateRecord(offset, rec);
	return OK;
}


int BTree::RemoveRecord(int x){
	if(!this->isLoaded){
		throw new std::runtime_error("Not connected to any BTree");
	}
	int offset = readRecord(x);
	if(offset == NOT_FOUND)
		return NOT_FOUND;
	int m = this->currPage->getM();
	Record rec;
	updateRecord(offset, rec);
	int toRemoveIndex;
	for(toRemoveIndex=0; toRemoveIndex<m; toRemoveIndex++)
		if(this->currPage->x[toRemoveIndex] == x)
			break;
	if(this->currPage->p[0] == NIL){
		// This is leaf page, safe to remove
		removeKeyFromLeafPage(this->currPage, toRemoveIndex);
	} else {
		// Not leaf, get smallest key from right subtree
		int originPageOffset = this->currPageOffset;
		int rSubTreeIndex = toRemoveIndex+1;
		readRecord(MINUS_INF, this->currPage->p[rSubTreeIndex]);
		int x = this->currPage->x[0];
		int a = this->currPage->a[0];
		removeKeyFromLeafPage(this->currPage, 0);
		int lowestSubTreePageOffset = this->currPageOffset;
		loadPage(originPageOffset);
		this->currPage->x[toRemoveIndex] = x;
		this->currPage->a[toRemoveIndex] = a;
		loadPage(lowestSubTreePageOffset);
	}
	while(true){
		int m = this->currPage->getM();
		if(m >= d || this->currPage->parent == NIL)
			return OK;
		else{
			int res = tryCompensationRemoval(this->currPageOffset);
			if(res == COMPENSATION_NOT_POSSIBLE){
				int page = merge(this->currPageOffset);
				loadPage(page);
				if(currPage->parent != NIL){
					loadPage(currPage->parent);
					if(currPage->getM() == 0){
						rootPageOffset = page;
						this->h--;
						pageCache->ShrinkCache(1);
						loadPage(page);
						currPage->parent = NIL;
					}
				}
			} else {
				return res;
			}
		}
	}

	return OK;
}

int BTree::merge(int ufP){
	int underFlowPageOffset = ufP;
	loadPage(currPage->parent);
	int parentOffset = this->currPageOffset;
	int pIndex=0;
	// It's his parent he has to be here somewhere
	for(pIndex=0; pIndex<2*d+1;pIndex++)
		if(this->currPage->p[pIndex] == underFlowPageOffset)
			break;
	int leftSiblingOffset = pIndex > 0 ? this->currPage->p[pIndex-1] : NIL;
	int rightSiblingOffset = pIndex < 2*d ? this->currPage->p[pIndex+1] : NIL;

	if(leftSiblingOffset != NIL){
		distributeMerge(leftSiblingOffset, underFlowPageOffset, parentOffset, pIndex-1);
		return leftSiblingOffset;
	}
	else if(rightSiblingOffset != NIL){
		distributeMerge(underFlowPageOffset, rightSiblingOffset, parentOffset, pIndex);
		return underFlowPageOffset;
	}
	else{
		throw new std::runtime_error("B-Tree structure got corrupted, will not work correctly.");
	}
}

void BTree::distributeMerge(int lP, int rP, int pP, int pIndex){
	loadPage(rP);
	int mR = currPage->getM();
	loadPage(lP);
	int mL = currPage->getM();

	int* x = new int[2*d];
	int* a = new int[2*d];
	int* p = new int[2*d+1];

	// Fill with left Page
	for(int i=0; i<mL; i++){
		x[i] = currPage->x[i];
		a[i] = currPage->a[i];
		p[i] = currPage->p[i];
		currPage->x[i] = NO_KEY;
		currPage->a[i] = NIL;
		currPage->p[i] = NIL;
	}
	p[mL] = currPage->p[mL];
	currPage->p[mL] = NIL;

	// Insert parent element
	loadPage(pP);
	x[mL] = currPage->x[pIndex];
	a[mL] = currPage->a[pIndex];

	// Fill with right page
	loadPage(rP);
	for(int i=0; i<mR; i++){
		x[i + mL + 1] = currPage->x[i];
		a[i + mL + 1] = currPage->a[i];
		p[i + mL + 1] = currPage->p[i];
		if(currPage->p[i] != NIL){
			loadPage(currPage->p[i]);
			currPage->parent = lP;
			loadPage(rP);
		}
		currPage->x[i] = NO_KEY;
		currPage->a[i] = NIL;
		currPage->p[i] = NIL;
	}
	p[mL + mR + 1] = currPage->p[mR];
	if(currPage->p[mR] != NIL){
		loadPage(currPage->p[mR]);
		currPage->parent = lP;
		loadPage(rP);
	}
	currPage->p[mR] = NIL;

	// Copy everything to left page
	loadPage(lP);
	std::copy(x, x+2*d, currPage->x);
	std::copy(a, a+2*d, currPage->a);
	std::copy(p, p+2*d+1, currPage->p);

	// Delete the element from parent page
	loadPage(pP);
	int mP = currPage->getM();
	for(int i = pIndex; i<mP-1; i++){
		currPage->x[i] = currPage->x[i+1];
		currPage->a[i] = currPage->a[i+1];
		currPage->p[i+1] = currPage->p[i+2];
	}
	currPage->x[mP-1] = NO_KEY;
	currPage->a[mP-1] = NIL;
	currPage->p[mP] = NIL;

	loadPage(lP);
	delete[] x;
	delete[] a;
	delete[] p;
}

int BTree::tryCompensationRemoval(int ufP){
	int result = COMPENSATION_NOT_POSSIBLE;
	bool done = false;
	int underFlowPageOffset = ufP;
	if(currPage->parent == NIL)
		return COMPENSATION_NOT_POSSIBLE;
	loadPage(currPage->parent);
	int parentOffset = this->currPageOffset;
	int pIndex=0;
	// It's his parent, he has to be here somewhere
	for(pIndex=0; pIndex<2*d+1; pIndex++)
		if(this->currPage->p[pIndex] == underFlowPageOffset)
			break;
	int leftSiblingOffset = pIndex > 0 ? this->currPage->p[pIndex-1] : NIL;
	int rightSiblingOffset = pIndex < 2*d ? this->currPage->p[pIndex+1] : NIL;

	// Check with left sibling first
	if(leftSiblingOffset != NIL){
		loadPage(leftSiblingOffset);
		int m = this->currPage->getM();
		if(m > d){
			// Possible compensation
			distributeCompensationRemoval(leftSiblingOffset, underFlowPageOffset, parentOffset, pIndex-1);
			done = true;
			result = OK;
		}
	}
	// If left is unavailable, then check right sibling
	if(rightSiblingOffset != NIL && !done){
		loadPage(rightSiblingOffset);
		int m = this->currPage->getM();
		if(m > d){
			// Possible compensation
			distributeCompensationRemoval(underFlowPageOffset, rightSiblingOffset, parentOffset, pIndex);
			done = true;
			result = OK;
		}
	}
	loadPage(underFlowPageOffset);
	return result;
}

void BTree::distributeCompensationRemoval(int lP, int rP, int pP, int pIndex){
	loadPage(rP);
	int mR = currPage->getM();
	loadPage(lP);
	int mL = currPage->getM();
	int toDistribute = mR + mL + 1;
	int* x = new int[toDistribute];
	int* a = new int[toDistribute];
	int* p = new int[toDistribute+1];
	// Fill with left Page
	for(int i=0; i<mL; i++){
		x[i] = currPage->x[i];
		a[i] = currPage->a[i];
		p[i] = currPage->p[i];
		currPage->x[i] = NO_KEY;
		currPage->a[i] = NIL;
		currPage->p[i] = NIL;
	}
	p[mL] = currPage->p[mL];
	currPage->p[mL] = NIL;

	// Insert parent element
	loadPage(pP);
	x[mL] = currPage->x[pIndex];
	a[mL] = currPage->a[pIndex];

	// Fill with right page
	loadPage(rP);
	for(int i=0; i<mR; i++){
		x[i + mL + 1] = currPage->x[i];
		a[i + mL + 1] = currPage->a[i];
		p[i + mL + 1] = currPage->p[i];
		currPage->x[i] = NO_KEY;
		currPage->a[i] = NIL;
		currPage->p[i] = NIL;
	}
	p[mL + mR + 1] = currPage->p[mR];
	currPage->p[mR] = NIL;
	// Get middle element
	int middle = toDistribute/2;
	// Write back to pages
	// To parent:
	loadPage(pP);
	currPage->x[pIndex] = x[middle];
	currPage->a[pIndex] = a[middle];
	// Fill back left page:
	loadPage(lP);
	for(int i=0; i<middle; i++){
		currPage->x[i] = x[i];
		currPage->a[i] = a[i];
		currPage->p[i] = p[i];
		if(currPage->p[i] != NIL){
			loadPage(currPage->p[i]);
			currPage->parent = lP;
			loadPage(lP);
		}
	}
	currPage->p[middle] = p[middle];
	if(currPage->p[middle] != NIL){
		loadPage(currPage->p[middle]);
		currPage->parent = lP;
		loadPage(lP);
	}
	// Fill back right page:
	loadPage(rP);
	int j, i;
	for(i=middle+1, j=0; i<toDistribute; i++, j++){
		currPage->x[j] = x[i];
		currPage->a[j] = a[i];
		currPage->p[j] = p[i];
		if(currPage->p[j] != NIL){
			loadPage(currPage->p[j]);
			currPage->parent = rP;
			loadPage(rP);
		}
	}
	currPage->p[j] = p[toDistribute];
	if(currPage->p[j] != NIL){
		loadPage(currPage->p[j]);
		currPage->parent = rP;
		loadPage(rP);
	}
	delete[] x;
	delete[] a;
	delete[] p;
}

void BTree::removeKeyFromLeafPage(Page* page, int index){
	int m = page->getM();
	for(int i=index+1; i<m; i++){
		// Pointers are nill anyway
		page->x[i-1] = page->x[i];
		page->a[i-1] = page->a[i];
	}
	page->x[m-1] = NO_KEY;
}

int BTree::InsertRecord(Record rec){
	if(!this->isLoaded){
		throw new std::runtime_error("Not connected to any BTree");
	}
	int res = readRecord(rec.getKey());
	if(res != NOT_FOUND)
		return ALREADY_EXISTS;
	int offset = saveRecord(rec);
	int cpointer = NIL;
	while(true){
		int m = this->currPage->getM();
		if(m < 2*d){
			// Update the parent
			if(cpointer != NIL){
				int tmp_parentOffset = this->currPageOffset;
				loadPage(cpointer);
				this->currPage->parent = tmp_parentOffset;
				loadPage(tmp_parentOffset);
			}
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
					return OK; //This occurs when new root was created
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
			return OK;
		}
		int res = tryCompensation(rec, offset, cpointer);
		if(res == COMPENSATION_NOT_POSSIBLE){
			cpointer = split(rec, offset, cpointer);
			int parent = this->currPage->parent;
			loadPage(parent);
		} else{
			return res;
		}
	}
	return OK;
}

int BTree::tryCompensation(Record rec, int recordOffset, int nPOffset){
	int result = COMPENSATION_NOT_POSSIBLE;
	bool done = false;
	int overflowPageOffset = this->currPageOffset;
	if(currPage->parent == NIL)
		return COMPENSATION_NOT_POSSIBLE;
	loadPage(currPage->parent);
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
		loadPage(leftSiblingOffset);
		int m = this->currPage->getM();
		if(m<2*d){
			// Possible compensation
			distributeCompensation(overflowPageOffset, leftSiblingOffset, parentOffset, rec, recordOffset, nPOffset, pIndex, true);
			done = true;
			result = OK;
		}

	}
	// Then right sibling
	if(rightSiblingOffset != NIL && !done){
		loadPage(rightSiblingOffset);
		int m = this->currPage->getM();
		if(m<2*d){
			// Possible compensation
			distributeCompensation(overflowPageOffset, rightSiblingOffset, parentOffset, rec, recordOffset, nPOffset, pIndex, false);
			result = OK;
			done = true;
		}
	}
	loadPage(overflowPageOffset);
	return result;
}

int BTree::split(Record& rec, int& recordOffset, int nPOffset){
	int overFlowPageOffset = this->currPageOffset;
	Page* newPage = new Page(this->d);
	newPage->parent = currPage->parent;
	int newPageOffset = savePage(newPage);

	distributeSplit(overFlowPageOffset, newPageOffset, rec, recordOffset, nPOffset);
	loadPage(newPageOffset);
	if(currPage->parent == NIL){
		this->h++;
		this->pageCache->ExpandCache(1);
		Page* newRoot = new Page(d);
		newRoot->x[0] = rec.getKey();
		newRoot->a[0] = recordOffset;
		newRoot->p[0] = overFlowPageOffset;
		newRoot->p[1] = newPageOffset;
		int newRootOffset = savePage(newRoot);
		loadPage(newPageOffset);
		currPage->parent = newRootOffset;
		loadPage(overFlowPageOffset);
		currPage->parent = newRootOffset;
		rootPageOffset = newRootOffset;
	}

	return newPageOffset;
}

void BTree::distributeCompensation(int ovP, int sbP, int pP, Record rec, int recordOffset, int nPOffset, int parentIndex, bool left){
	// Oveflow has 2*d, sibling has m, one from parent and we also insert 1 record
	loadPage(sbP);
	int toDistribute = 2*d + this->currPage->getM() + 1 + 1;
	int* x = new int[toDistribute];
	int* a = new int[toDistribute];
	int* p = new int[toDistribute+1];

	int rIndex = 0;
	if(left){
		// Fill the start with sibling
		int sbM = currPage->getM();
		for(int i=0; i<sbM; i++, rIndex++){
			x[rIndex] = currPage->x[rIndex];
			a[rIndex] = currPage->a[rIndex];
			p[rIndex] = currPage->p[rIndex];
			currPage->x[rIndex] = NO_KEY;
			currPage->a[rIndex] = NIL;
			currPage->p[rIndex] = NIL;
		}
		p[rIndex] = currPage->p[sbM];
		currPage->p[sbM] = NIL;

		loadPage(pP);
		// Insert the parent
		x[rIndex] = currPage->x[parentIndex-1];
		a[rIndex] = currPage->a[parentIndex-1];
		rIndex++;
	}

	int newRecIndex = 0;
	loadPage(ovP);
	// find index where new recod should be inserted
	for(; newRecIndex<2*d; newRecIndex++)
		if(currPage->x[newRecIndex] > rec.getKey())
			break;

	// We can rewrite all up to conflicting record
	for(int i=0; i<newRecIndex; i++){
		x[rIndex + i] = currPage->x[i];
		a[rIndex + i] = currPage->a[i];
		p[rIndex + i] = currPage->p[i];
		currPage->x[i] = NO_KEY;
		currPage->a[i] = NIL;
		currPage->p[i] = NIL;
	}
	// Insert the new record
	if(left){
		x[rIndex + newRecIndex] = rec.getKey();
		a[rIndex + newRecIndex] = recordOffset;
		p[rIndex + newRecIndex] = currPage->p[newRecIndex];
		p[rIndex + newRecIndex + 1] = nPOffset;
		rIndex++;
		currPage->p[newRecIndex] = NIL;
	} else {
		x[rIndex + newRecIndex] = rec.getKey();
		a[rIndex + newRecIndex] = recordOffset;
		p[rIndex + newRecIndex] = currPage->p[rIndex + newRecIndex];
		p[rIndex + newRecIndex + 1] = nPOffset;
		currPage->p[rIndex + newRecIndex] = NIL;
		rIndex++;
	}

	// Insert the rest from the overflow page
	for(;newRecIndex<2*d; newRecIndex++){
		x[rIndex+newRecIndex] = currPage->x[newRecIndex];
		a[rIndex+newRecIndex] = currPage->a[newRecIndex];
		p[rIndex+newRecIndex+1] = currPage->p[newRecIndex+1];
		currPage->x[newRecIndex] = NO_KEY;
		currPage->a[newRecIndex] = NIL;
		currPage->p[newRecIndex+1] = NIL;
	}

	if(!left){
		loadPage(pP);
		// insert the parent
		x[newRecIndex+rIndex] = currPage->x[parentIndex];
		a[newRecIndex+rIndex] = currPage->a[parentIndex];
		rIndex++;
		// Fill the rest with sibling
		loadPage(sbP);
		int sbM = currPage->getM();
		for(int i=0; i<sbM; i++, rIndex++){
			x[newRecIndex + rIndex] = currPage->x[i];
			a[newRecIndex + rIndex] = currPage->a[i];
			p[newRecIndex + rIndex] = currPage->p[i];
			currPage->x[i] = NO_KEY;
			currPage->a[i] = NIL;
			currPage->p[i] = NIL;
		}
		p[newRecIndex + rIndex] = currPage->p[sbM];
		currPage->p[sbM] = NIL;
	}

	// Get the middle element
	int middle = toDistribute/2;
	//Write him to parent
	loadPage(pP);
	if(left){
		currPage->x[parentIndex-1] = x[middle];
		currPage->a[parentIndex-1] = a[middle];
	} else{
		currPage->x[parentIndex] = x[middle];
		currPage->a[parentIndex] = a[middle];
	}
	// Fill back overflow
	if(!left){
		int i;
		loadPage(ovP);
		for(i=0; i<middle; i++){
			currPage->x[i] = x[i];
			currPage->a[i] = a[i];
			currPage->p[i] = p[i];
			if(currPage->p[i] != NIL){
				loadPage(currPage->p[i]);
				currPage->parent = ovP;
				loadPage(ovP);
			}
		}
		currPage->p[i] = p[i];
		if(currPage->p[i] != NIL){
			loadPage(currPage->p[i]);
			currPage->parent = ovP;
		}
		loadPage(sbP);
		int j;
		for(j=middle+1, i=0; j<toDistribute; j++, i++){
			currPage->x[i] = x[j];
			currPage->a[i] = a[j];
			currPage->p[i] = p[j];
			if(currPage->p[i] != NIL){
				loadPage(currPage->p[i]);
				currPage->parent = sbP;
				loadPage(sbP);
			}
		}
		currPage->p[i] = p[toDistribute];
		if(currPage->p[i] != NIL){
			loadPage(currPage->p[i]);
			currPage->parent = sbP;
		}
	} else {
		int i;
		loadPage(sbP);
		for(i=0; i<middle; i++){
			currPage->x[i] = x[i];
			currPage->a[i] = a[i];
			currPage->p[i] = p[i];
			if(currPage->p[i] != NIL){
				loadPage(currPage->p[i]);
				currPage->parent = sbP;
				loadPage(sbP);
			}
		}

		currPage->p[i] = p[i];
		if(currPage->p[i] != NIL){
			loadPage(currPage->p[i]);
			currPage->parent = sbP;
			loadPage(sbP);
		}
		loadPage(ovP);
		int j;
		for(j=middle+1, i=0; j<toDistribute; j++, i++){
			currPage->x[i] = x[j];
			currPage->a[i] = a[j];
			currPage->p[i] = p[j];
			if(currPage->p[i] != NIL){
				loadPage(currPage->p[i]);
				currPage->parent = ovP;
				loadPage(ovP);
			}
		}
		currPage->p[i] = p[j];
		if(currPage->p[i] != NIL){
			loadPage(currPage->p[i]);
			currPage->parent = ovP;
		}
	}

	delete[] x;
	delete[] a;
	delete[] p;
}

void BTree::distributeSplit(int ovP, int sbP, Record& rec, int& recordOffset, int nPOffset){
	// Oveflow has 2*d, sibling has 0, and we also insert 1 record
	int toDistribute = 2*d + 1;
	int* x = new int[toDistribute];
	int* a = new int[toDistribute];
	int* p = new int[toDistribute+1];

	// find index where new recod should be inserted
	loadPage(ovP);
	int rIndex = 0;
	for(; rIndex<2*d; rIndex++)
		if(currPage->x[rIndex] > rec.getKey())
			break;

	// We can rewrite all up to conflicting record
	for(int i=0; i<rIndex; i++){
		x[i] = currPage->x[i];
		a[i] = currPage->a[i];
		p[i] = currPage->p[i];
		currPage->x[i] = NO_KEY;
		currPage->a[i] = NIL;
		currPage->p[i] = NIL;
	}
	// Insert the new record
	x[rIndex] = rec.getKey();
	a[rIndex] = recordOffset;
	p[rIndex] = currPage->p[rIndex];
	p[rIndex+1] = nPOffset;
	currPage->p[rIndex] = NIL;
	rIndex++;

	// Insert the rest from the overflow page
	for(;rIndex-1<2*d; rIndex++){
		x[rIndex] = currPage->x[rIndex-1];
		a[rIndex] = currPage->a[rIndex-1];
		p[rIndex+1] = currPage->p[rIndex-1+1];
		currPage->x[rIndex-1] = NO_KEY;
		currPage->a[rIndex-1] = NIL;
		currPage->p[rIndex-1+1] = NIL;
	}

	// Get the middle element
	int middle = toDistribute/2;
	// He will be returned by the function
	rec.setKey(x[middle]);
	recordOffset = a[middle];
	// Fill back overflow
	int i;
	for(i=0; i<middle; i++){
		currPage->x[i] = x[i];
		currPage->a[i] = a[i];
		currPage->p[i] = p[i];
	}
	currPage->p[i] = p[i];
	// Fill new page
	loadPage(sbP);
	for(int j=middle+1, i=0; j<toDistribute; j++, i++){
		currPage->x[i] = x[j];
		currPage->a[i] = a[j];
		currPage->p[i] = p[j];
		if(currPage->p[i] != NIL){
			loadPage(currPage->p[i]);
			currPage->parent = sbP;
			loadPage(sbP);
		}
	}
	currPage->p[i] = p[toDistribute];
	if(currPage->p[i] != NIL){
		loadPage(currPage->p[i]);
		currPage->parent = sbP;
		loadPage(sbP);
	}

	delete[] x;
	delete[] a;
	delete[] p;
}

void BTree::PrintIOStatistics(){
	std::cout << "Main  Memory: " << diskReadMainMemory << " reads, " << diskWriteMainMemory << " writes.\n"
			  << "Index Memory: " << diskReadIndexMemory << " reads, " << diskWriteIndexMemory << " writes.\n";
}

void BTree::ResetIOCounters(){
	this->diskReadIndexMemory = 0;
	this->diskReadMainMemory = 0;
	this->diskWriteIndexMemory = 0;
	this->diskWriteMainMemory = 0;
}

void BTree::SequentialRead(){
	sequentialRead(this->rootPageOffset);
}

void BTree::sequentialRead(int offset){
	if(offset == NIL)
		return;
	loadPage(offset, false);
	int m = currPage->getM();
	for(int i=0; i<m; i++){
		sequentialRead(currPage->p[i]);
		loadPage(offset, false);
		Record rec = loadRecord(currPage->a[i]);
		rec.print(currPage->a[i]);
	}
	if(m)
		sequentialRead(currPage->p[m]);
}
