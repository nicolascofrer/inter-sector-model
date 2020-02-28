/*
 * Listener.h
 *
 *  Created on: Sep 7, 2014
 *      Author: ncofre
 */
#include "adevs.h"
#include "Loan.h"
#include <fstream>
#ifndef LISTENER_H_
#define LISTENER_H_


class Listener:
	public adevs::EventListener<Loan>
{
	public:
	Listener();
	void outputEvent(adevs::Event<Loan> x, double t);



~Listener();

private:

std::ofstream outputfile;



};

#endif /* LISTENER_H_ */
