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
#include"../cache/Cache.h"

typedef int PAGE_OFFSET;

class Cache;

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
	int RemoveRecord(int x);

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
	Cache* pageCache;

	void saveMetaData();
	void loadMetaData();


	Page* loadPage(int offset);
	int savePage(Page* page);
	void updatePage(int offset, Page* page, bool skipCache=false);

	Record loadRecord(int offset);
	int saveRecord(Record record);
	void updateRecord(int offset, Record record);

	int readRecord(int x, int startPage=NIL);

	int tryCompensation(Record rec, int recordOffset, int nPOffset);
	void distributeCompensation(int ovP, int sbP, int pP, Record rec, int recordOffset, int nPOffset, int parentIndex, bool left);

	int split(Record& rec, int& recordOffset, int nPOffset);
	void distributeSplit(int ovP, int sbP, Record& rec, int& recordOffset, int nPOffset);

	void removeKeyFromLeafPage(Page* page, int index);
	//TODO tryCompensationRemoval
	//TODO distributeCompensationRemoval

	//TODO merge
	//TODO distributeMerge

	void sequentialRead(int pageOffset);

	//TODO turn it into a cache h-sized
	Page* currPage;
	int currPageOffset;


	// For disk usage statistics
	int diskReadMainMemory;
	int diskReadIndexMemory;
	int diskWriteMainMemory;
	int diskWriteIndexMemory;
friend class Cache;
};

#endif /* BTREE_BTREE_H_ */
