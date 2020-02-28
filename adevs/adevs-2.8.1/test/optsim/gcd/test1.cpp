#include <iostream>
#include "adevs.h"
#include "gcd.h"
#include "Listener.h"
using namespace std;

int main() 
{
	cout << "Test 1" << endl;
	adevs::Digraph<object*>* model = new adevs::Digraph<object*>();
	gcd* c = new gcd(10.0,2.0,1,false);
	genr* g = new genr(10.0,1,true);
	model->couple(g,g->signal,c,c->in);
	adevs::ParSimulator<PortValue>* sim =
		new adevs::ParSimulator<PortValue>(model);
	sim->addEventListener(new Listener());
	sim->execUntil(100.0);
	cout << "Test done" << endl;
	delete sim;
	delete model;
	return 0;
}
