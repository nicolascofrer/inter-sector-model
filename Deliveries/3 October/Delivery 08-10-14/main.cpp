#include "BNetwork.h"
#include "Generator.h"
#include "Loan.h"
#include "RatingAgency.h"
#include "Bank.h"
#include <iostream>
#include <map>
#include <fstream>
#include "Parameters.h"
#include "Listener.h"

using namespace adevs;
using namespace std;

//product>scheme> edit ....
int main (int argc, char** argv)
{
    
    BNetwork* bnetwork = new BNetwork();
    unsigned long int seed;
    //seed = 344445;
   if (argc!=3){ cerr << "argv[1] = random seed and argv[2] = Output file" << endl;
    return 1;}
    
    seed = atoi(argv[1]);
    //seed = 323;
    Generator* generator = new Generator(seed);
    SimpleDigraph<Loan>* model = new SimpleDigraph<Loan>();
    model->add(bnetwork);
    model->add(generator);
    model->couple(generator,bnetwork); //All output from generator is input of BNetwork
    Simulator<Loan>* sim = new Simulator<Loan>(model);
    
    
    //Add the rating agency
    RatingAgency* RA = new RatingAgency(bnetwork);
    model->add(RA);
    
    ofstream myfile;
    //myfile.open("fichier.txt");
    myfile.open (argv[2]);
    ofstream network;
    network.open ("network.numbers");
    
    Listener* listener = new Listener();
    sim->addEventListener(listener);
    
    // Write a header describing the data fields
    myfile << "Time,";
    myfile << "Time between arrivals,";
    myfile << "Number of banks,";
    myfile << "Number of deposit arrivals,";
    myfile << "Number of loan arrivals,";
    myfile << "Value last state-action,";
    
    
    //	for (int i = 0; i != NBANKS-1 ; i++)
    //	{myfile <<  "Equity Bank "<< i  << ",";}
    //	//myfile << "Equity Bank "<< NBANKS << endl;
    //	myfile << "Equity Bank "<< NBANKS-1 << ",";
    
    for (int i = 0; i != NBANKS; i++)
    {myfile <<  "Equity Bank "<< i  << ",";}
    myfile << "Equity Bank "<< NMAX << ",";
    
    
    for (int i = 0; i != NBANKS-1 ; i++)
    {myfile <<  "IB Deposits Requested Bank "<< i<< ",";}
    myfile << "IB Deposits Requested Bank "<< NBANKS-1 << ",";
    
    for (int i = 0; i != NBANKS-1 ; i++)
    {myfile <<  "IB Deposits Accepted Bank "<< i<< ",";}
    myfile << "IB Deposits Accepted Bank "<< NBANKS-1 << ",";
    
    for (int i = 0; i != NBANKS-1 ; i++)
    {myfile <<  "Customer Deposit Bank "<< i<< ",";}
    myfile << "Customer Deposit Bank "<< NBANKS-1 << ",";
    
    
    for (int i = 0; i != NBANKS-1 ; i++)
    {myfile <<  "Customer Loan Bank "<< i<< ",";}
    //myfile << "Customer Loan Bank "<< NBANKS-1 << ",";
    myfile << "Customer Loan Bank "<< NBANKS-1 << endl;
    
    
    for (int i = 0; i != NBANKS-1 ; i++)
    {network <<  "Partners Bank "<< i  << ",";}
    network << "Partners Bank "<< NBANKS-1 << endl;
    
    
    
    while (sim->nextEventTime() <= 3000.0)
    {
        //if( sim->nextEventTime() <= 4000){
        //sim->execNextEvent();
        //generator->set_n_banks( bnetwork->getBankCount() );}
        
        
        //if( sim->nextEventTime() > 4000){
        
        //(bnetwork->getBank(0))->StopUpdatingProcess();
        
        //Pending: Function that restart parameters to initial conditions
        //generator->set_n_banks( bnetwork->getBankCount() );
        
        sim->execNextEvent();
        //write time
        myfile << sim->nextEventTime() << ",";
        //write the time between arrivals
        myfile <<  generator->ta() << ",";
        //write the number of banks
        myfile << bnetwork->getBankCount() << ",";
        //write the number of deposits
        myfile << generator->get_number_of_deposits() << ",";
        myfile << generator->get_number_of_loans() << ",";
        
        
   //      myfile <<(bnetwork->getBank(0))->LastStateActionValue() <<",";
        //Equity
        //		for (int i = 0; i != bnetwork->getBankCount()-1 ; i++)
        for (int i = 0; i != NMAX ; i++)
        {
            if(i<= Bank::getBankCounter()-1) myfile <<  (bnetwork->getBankIncludingBankrupt(i))->GetEquity()   << ",";
            else myfile << 0 << ",";
        }
        //myfile << (bnetwork->getBank( bnetwork->getBankCount()-1 ))->GetEquity() << endl;
        //myfile << (bnetwork->getBank( NMAX ))->GetEquity() << ",";
        
        //IB deposits requested
        for (int i = 0; i != bnetwork->getBankCount()-1 ; i++)
        {myfile <<  (bnetwork->getBank(i))->getIBDepositRequestQueueSize()   << ",";}
        myfile << (bnetwork->getBank( bnetwork->getBankCount()-1 ))->getIBDepositRequestQueueSize() << ",";
        
        //IB deposits accepted
        for (int i = 0; i != bnetwork->getBankCount()-1 ; i++)
        {myfile <<  (bnetwork->getBank(i))->getIBDepositAcceptedQueueSize()   << ",";}
        myfile << (bnetwork->getBank( bnetwork->getBankCount()-1 ))->getIBDepositAcceptedQueueSize() << ",";
        
        //Customer deposit info
        for (int i = 0; i != bnetwork->getBankCount()-1 ; i++)
        {myfile <<  (bnetwork->getBank(i))->getDepositQueueSize() << ",";}
        myfile << (bnetwork->getBank( bnetwork->getBankCount()-1 ))->getDepositQueueSize() << ",";
        //Customer loan info
        for (int i = 0; i != bnetwork->getBankCount()-1 ; i++)
        {myfile <<  (bnetwork->getBank(i))->getLoanQueueSize()   << ",";}
        //myfile << (bnetwork->getBank( bnetwork->getBankCount()-1 ))->getLoanQueueSize() << ",";
        myfile << (bnetwork->getBank( bnetwork->getBankCount()-1 ))->getLoanQueueSize() << endl;
        //Partners info
        for (int i = 0; i != bnetwork->getBankCount()-1 ; i++)
        {network <<  (bnetwork->getBank(i))->printPartners()   << ",";}
        network << (bnetwork->getBank( bnetwork->getBankCount()-1 ))->printPartners() << endl;
        
        
        
        //cerr << Bank::getBankCounter() << endl;
        
        //}
        
        
        
        
        //if(sim->nextEventTime()>100) (bnetwork->getBank(2))->reduce_equity(2000); //just checking the network dynamic
        
    }
    
    //cout <<(bnetwork->getBank(0))->GetEquity() <<endl;
    //	cerr <<(bnetwork->getBank(0))->GetNPartners() <<endl;
    //	cerr <<(bnetwork->getBank(1))->GetNPartners() <<endl;
    //	cerr <<(bnetwork->getBank(2))->GetNPartners() <<endl;
    //	cerr <<(bnetwork->getBank(3))->GetNPartners() <<endl;
    //	cerr <<(bnetwork->getBank(4))->GetNPartners() <<endl;
    //	cerr <<(bnetwork->getBank(5))->GetNPartners() <<endl;
    //	cerr <<(bnetwork->getBank(6))->GetNPartners() <<endl;
    //	cerr <<(bnetwork->getBank(7))->GetNPartners() <<endl;
    //	cerr <<(bnetwork->getBank(8))->GetNPartners() <<endl;
    //	cerr <<(bnetwork->getBank(9))->GetNPartners() <<endl;
    //
    
    
    
    //cout <<(bnetwork->getBank(2))->GetLoanRate() <<endl;
    //cout <<(bnetwork->getBank(2))->GetNumberStatesVisited()<<endl; //Testing the assignment of states as keys
    //cout <<(bnetwork->getBank(1))->GetNumberStatesVisited()<<endl; //Testing the assignment of states as keys
    //	cout <<(bnetwork->getBank(1))->GetIBLoanRate()<<endl;
    
    
    
    myfile.close();
    network.close();
    delete listener;
    delete sim;
    delete model;
    cerr << "FINISHED WITHOUT ERROR" ;
    return 0;
}

