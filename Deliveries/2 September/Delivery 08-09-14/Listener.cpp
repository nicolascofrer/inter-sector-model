/*
 * Listener.cpp
 *
 *  Created on: Sep 7, 2014
 *      Author: ncofre
 */
#include "adevs.h"
#include "Listener.h"
#include <fstream>
using namespace std;
using namespace adevs;

	Listener::Listener(): adevs::EventListener<Loan>()
			{
		outputfile.open("ListenerOutput.txt");
		outputfile << "IDborrower" << "," << "IDlender" << "," << "getID" << "," << "IsExpired" << "," << "GetLinkedAgentID" << endl;
			}
	void Listener::outputEvent(Event<Loan> x, double t)
		{
		//Here we write to the Listener's output file <
		outputfile << x.value.get_IDborrower() << "," << x.value.get_IDlender() << "," << x.value.getID() << "," << x.value.IsExpired() << "," << x.value.GetLinkedAgentID() << endl;
		}

	Listener::~Listener(){outputfile.close();}



