#ifndef _Qn_h_
#define _Qn_h_
#include "adevs.h"
#include "Genr.h"
#include "Sq.h"
#include <iostream>

class Qn: public adevs::Network<int>
{
	public:
		// Create a model with s lines of q servers. The arrangement
		// of the logical processes is returned via the supplied LpGraph.
		Qn(int q, int s, adevs::LpGraph& lpg):
			adevs::Network<int>(),qcount(s),ql(new Ql*[s]),
			genr((double)s/2.0),collect()
		{
			int thrds = omp_get_max_threads(); // Get the number of threads
			genr.setParent(this); // The generator goes to process zero
			genr.setProc(0);
			collect.setParent(this); // Collector goes on the last process
			collect.setProc(thrds-1);
			// The queues are split among all the processes
			for (int i = 0; i < qcount; i++)
			{
				ql[i] = new Ql(q);
				ql[i]->setParent(this);
				ql[i]->setProc(i%thrds);
			}
			// Add 0->[1,thrds) to the LpGraph. This connects the
			// generator to everyone.
			for (int i = 1; i < thrds; i++) lpg.addEdge(0,i);
			// Add [0,thrds-1)->thrds-1 to the LpGraph. This connects
			// everyone to the collector.
			for (int i = 0; i < thrds-1; i++) lpg.addEdge(i,thrds-1);
		}
		void getComponents(adevs::Set<adevs::Devs<int>*>& c)
		{
			for (int i = 0; i < qcount; i++) c.insert(ql[i]);
			c.insert(&genr); c.insert(&collect);
		}
		void route(const int &value, adevs::Devs<int> *model,
			adevs::Bag<adevs::Event<int> > &r)
		{
			if (model == &genr)
				r.insert(adevs::Event<int>(ql[value%qcount],value));
			else r.insert(adevs::Event<int>(&collect,value));
		}
		~Qn()
		{
			// Delete the components of the model
			for (int i = 0; i < qcount; i++) delete ql[i];
			delete [] ql;
		}
	private:
		const int qcount;
		Ql** ql;
		Genr genr;
		Collector collect;
};

#endif
