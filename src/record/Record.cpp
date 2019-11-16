/*
 * Record.cpp
 *
 *  Created on: Nov 13, 2019
 *      Author: gales
 */

#include "Record.h"

Record::Record(): key(NO_KEY), a(0), b(0), c(0){

}

Record::Record(int k){
	this->key = k;
	this->a = 0;
	this->b = 0;
	this->c = 0;
}

Record::Record(int k, int a, int b, int c){
	this->key = k;
	this->a = a;
	this->b = b;
	this->c = c;
}

Record::~Record(){

}

void Record::print(int offset){
	if(offset == NOT_FOUND){
		std::cout << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n"
				  << "┃       RECORD DOESN'T EXIST       ┃\n"
				  << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";
	} else {
		std::cout << "┏━━━━━━━━┯━━━━━━━━┯━━━━━━━━┯━━━━━━━━┯━━━━━━━━┓\n"
				  << "┃ OFFSET │   KEY  │    a   │     b  │     c  ┃\n┃";
		std::cout << std::setw(8) << offset << "│" << std::setw(8) << key
				  << "│" << std::setw(8) << a << "│"
				  << std::setw(8) << b << "│" << std::setw(8) << c << "┃\n";
		std::cout << "┗━━━━━━━━┷━━━━━━━━┷━━━━━━━━┷━━━━━━━━┷━━━━━━━━┛\n";
	}
}
