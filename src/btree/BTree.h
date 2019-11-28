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

	bool IsReady(){ return isLoaded;}
	void CreateBTree(std::string name, int d);
	void LoadBTree(std::string name);

	// Basic operations
	Record SearchForRecord(int x);
	int InsertRecord(Record rec);
	int UpdateRecord(Record rec);
	void SequentialRead();

	// Helpers (visualization)
	void PrintMainMem();
	void PrintIndex();

	// Helper (statistics)
	void PrintIOStatistics();
	void ResetIOCounters();

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
	void updatePage(int offset, Page* page);

	Record loadRecord(int offset);
	int saveRecord(Record record);
	void updateRecord(int offset, Record record);

	int readRecord(int x);

	int tryCompensation(Record rec, int recordOffset, int nPOffset);
	void distribute(Page* ovP, Page* sbP, Page* pP, Record rec, int recordOffset, int nPOffset, int parentIndex, bool left);

	int split(Record& rec, int& recordOffset, int nPOffset);
	void distributeSplit(Page* ovP, Page* sbP, Record& rec, int& recordOffset, int nPOffset);

	void sequentialRead(int pageOffset);

	//TODO turn it into a cache h-sized
	Page* currPage;
	int currPageOffset;


	// For disk usage statistics
	int diskReadMainMemory;
	int diskReadIndexMemory;
	int diskWriteMainMemory;
	int diskWriteIndexMemory;
};

#endif /* BTREE_BTREE_H_ */
