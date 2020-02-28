/*
 * Listener.cpp
 *
 *  Created on: Sep 7, 2014
 *      Author: ncofre
 */
#include "adevs.h"
#include "Listener.h"
#include "Bank.h"
#include <fstream>
using namespace std;
using namespace adevs;

	Listener::Listener(): adevs::EventListener<Loan>()
			{
		outputfile.open("ListenerOutput.txt");
		outputfile
		<< "IDborrower"
		<< "," << "IDlender"
		<< "," << "getID"
		<< "," << "IsExpired"
		<< "," << "GetLinkedAgentID"
		<< "," << "getType"
		<< "," << "Initial Borrower"
		<< "," << "Initial Lender"
		<< "," << "Linked Borrower (IB Market)"
		<< "," << "Linked Lender (IB Market)"
		<< "," << "IB Deposit Rate Linked Agent"
		<< "," << "IB Loan Rate Linked Agent"
		<<endl;
			}
	void Listener::outputEvent(Event<Loan> x, double t)
		{
		//Here we write to the Listener's output file <
		outputfile << x.value.get_IDborrower()
				<< "," << x.value.get_IDlender()
				<< "," << x.value.getID()
				<< "," << x.value.IsExpired()
				<< "," << x.value.GetLinkedAgentID()
				<< "," << x.value.getType();
		outputfile << "," ; if(x.value.getType()==1 | x.value.getType()==11 ) outputfile  << (dynamic_cast<Bank*> (x.value.get_borrower()))->getID() ; else outputfile << "";

		outputfile << "," ; if(x.value.getType()==2 | x.value.getType()==22) outputfile  << (dynamic_cast<Bank*> (x.value.get_lender()))->getID() ; else outputfile << "";

		outputfile << "," ; if(x.value.getType()==11) outputfile  << (dynamic_cast<Bank*> (x.value.GetLinkedAgent()))->getID() ; else outputfile << "";

		outputfile << "," ; if(x.value.getType()==22) outputfile  << (dynamic_cast<Bank*> (x.value.GetLinkedAgent()))->getID() ; else outputfile << "";

		outputfile << "," ; if(x.value.getType()==11) outputfile  << (dynamic_cast<Bank*> (x.value.GetLinkedAgent()))->GetIBDepositRate() ; else outputfile << "";

		outputfile << "," ; if(x.value.getType()==22) outputfile  << (dynamic_cast<Bank*> (x.value.GetLinkedAgent()))->GetIBLoanRate() ; else outputfile << "";

		outputfile << endl;
		}

	//Listener::~Listener(){outputfile.close();}



