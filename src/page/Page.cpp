/*
 * Page.cpp
 *
 *  Created on: Nov 13, 2019
 *      Author: gales
 */

#include "Page.h"

Page::Page(int d){
	this->d = d;
	this->parent = NIL;
	this->x = new int[2*d];
	this->a = new int[2*d];
	this->p = new int[2*d+1];

	for(int i=0; i<2*d; i++){
		this->x[i] = NO_KEY;
		this->a[i] = NIL;
		this->p[i] = NIL;
	}
	this->p[2*d] = NIL;
}

Page::~Page(){
	delete[] x;
	delete[] a;
	delete[] p;
}

int Page::getM(){
	int m = 0;
	for(int i=0; i<2*d; i++){
		if(x[i] != NO_KEY)
			m++;
		else
			break;
	}
	return m;
}
