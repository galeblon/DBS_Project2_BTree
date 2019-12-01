/*
 * Cache.cpp
 *
 *  Created on: Nov 30, 2019
 *      Author: gales
 */

#include "Cache.h"

PageListNode::PageListNode(){
	pageOffset = NIL;
	page = NULL;
	nextElement = NULL;
}

PageListNode::~PageListNode(){
	delete page;
}

Cache::Cache(int len, BTree* tree){
	length = len;
	cacheList = new PageListNode();
	PageListNode* current = cacheList;
	for(int i=1; i<len; i++){
		current->nextElement = new PageListNode();
		current = current->nextElement;
	}
	cycle = 0;
	this->tree = tree;
}

Cache::~Cache(){
	PageListNode* current = cacheList;
	PageListNode* next;
	while(current != NULL){
		next = current->nextElement;
		if(current->page != NULL)
			tree->updatePage(current->pageOffset, current->page, true);
		delete current;
		current = next;
	}
}

void Cache::ExpandCache(int howMuch){
	for(int i=0; i<howMuch; i++){
		PageListNode* current = cacheList;
		while(current->nextElement != NULL)
			current = current->nextElement;
		current->nextElement = new PageListNode();
		length++;
	}
}

void Cache::ShrinkCache(int howMuch){
	if(length < DEFAULT_CACHE_SIZE)
		return;
	for(int i=0; i<howMuch; i++){
		PageListNode* current = cacheList;
		while(current->nextElement->nextElement != NULL)
			current = current->nextElement;
		delete current->nextElement;
		current->nextElement = NULL;
		length--;
	}
}

Page* Cache::FindPage(int offset){
	PageListNode* current = cacheList;
	while(current != NULL){
		if(current->pageOffset == offset)
			return current->page;
		current = current->nextElement;
	}
	return NULL;
}

void Cache::CachePage(Page* page, int offset){
	PageListNode* current = cacheList;
	for(int i=0; i<cycle; i++)
		current = current->nextElement;
	cycle = (cycle+1)%length;
	if(current->page != NULL){
		tree->updatePage(current->pageOffset, current->page, true);
		delete current->page;
	}
	current->page = page;
	current->pageOffset = offset;
}

bool Cache::IsCached(Page* page){
	PageListNode* current = cacheList;
	while(current != NULL){
		if(current->page == page)
			return true;
		current = current->nextElement;
	}
	return false;
}

bool Cache::IsCached(int offset){
	PageListNode* current = cacheList;
	while(current != NULL){
		if(current->pageOffset == offset)
			return true;
		current = current->nextElement;
	}
	return false;
}


