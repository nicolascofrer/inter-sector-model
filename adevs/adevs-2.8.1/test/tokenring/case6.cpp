#include <iostream>
#include "node.h"
using namespace std;

int main() 
{
	adevs::Digraph<token_t*> model;
	node* n1 = new node(0,1,new token_t(0));
	node* n2 = new node(1,1,new token_t(1));
	node* n3 = new node(2,3,NULL);
	model.add(n1);
	model.add(n2);
	model.add(n3);
	model.couple(n1,n1->out,n2,n2->in);
	model.couple(n2,n2->out,n3,n3->in);  
	model.couple(n3,n3->out,n1,n1->in);  
	adevs::Simulator<PortValue> sim(&model);
	for (int i = 0; i < 10 && sim.nextEventTime() < DBL_MAX; i++)
	{
		cout << endl;
		sim.execNextEvent();
	}
	cout << endl;
	cout << "End of run!" << endl;
	return 0;
}
