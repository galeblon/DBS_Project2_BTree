/*
 * Record.h
 *
 *  Created on: Nov 13, 2019
 *      Author: gales
 */

#ifndef RECORD_RECORD_H_
#define RECORD_RECORD_H_

#include <iostream>
#include <iomanip>

#define NO_KEY 0
#define NOT_FOUND -2
#define ALREADY_EXISTS -3
#define OK 42

class Record {
public:
	Record();
	Record(int k);
	Record(int k, int a, int b, int c);
	~Record();

	int getKey() {return key;}
	void setKey(int key) {this->key = key;}

	int getA(){ return a;}
	int getB(){ return b;}
	int getC(){ return c;}
	void setABC(int a, int b, int c){
		this->a = a; this->b = b; this->c = c;
	}

	void print(int offset);

private:
	int key;
	int a;
	int b;
	int c;
};

#endif /* RECORD_RECORD_H_ */
