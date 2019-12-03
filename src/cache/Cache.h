/*
 * Cache.h
 *
 *  Created on: Nov 30, 2019
 *      Author: gales
 */

#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include "../page/Page.h"
#include "../btree/BTree.h"

#define DEFAULT_CACHE_SIZE 2

class BTree;

class PageListNode{
public:
	PageListNode();
	~PageListNode();

	int pageOffset;
	bool isDirty;
	Page* page;

	PageListNode* nextElement;

};

class Cache {
public:
	Cache(int len, BTree* tree);
	~Cache();

	void ExpandCache(int howMuch);
	void ShrinkCache(int howMuch);

	Page* FindPage(int offset, bool willBeDirty=true);
	void CachePage(Page* page, int offset, bool willBeDirty=true);
	bool IsCached(Page* page);
	bool IsCached(int offset);

private:
	BTree* tree;
	int length;
	int cycle;
	PageListNode* cacheList;
};

#endif /* CACHE_CACHE_H_ */
