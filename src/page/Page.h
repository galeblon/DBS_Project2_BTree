/*
 * Page.h
 *
 *  Created on: Nov 13, 2019
 *      Author: gales
 */

#ifndef PAGE_PAGE_H_
#define PAGE_PAGE_H_

#include "../record/Record.h"

#define NIL -1

class Page {
public:
	Page(int d);
	~Page();

private:
	int d;
	int parent;
	int* x;			// Keys
	int* a;			// Address of the record in the main file
	int* p;			// Pointer to the child page
};

#endif /* PAGE_PAGE_H_ */
