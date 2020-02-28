#include "fireCell.h"
#include <cassert>
#include <iostream>
using namespace std;

// Movement rate of the fire
const double fireCell::move_rate = 1.0;

fireCell::fireCell(double fuel, bool on_fire, 
long int x, long int y):
adevs::Atomic<CellEvent>(),
fuel(fuel),
move_left(move_rate),
heat(0),
x(x),
y(y)
{
	assert(fuel >= 0.0);
	// If it is on fire and has enough fuel to spread
	if (on_fire && fuel >= move_rate) phase = IGNITE;
	// If it is on fire but does not have enough fuel to spread
	else if (on_fire) phase = BURN_FAST;
	// If it is not burning at all
	else phase = UNBURNED;
}

double fireCell::lookahead()
{
	if (fuel < move_rate) return fuel;
	else return move_rate;
}

double fireCell::ta()
{
	// A Burning cell burns for as long as it has fuel
	if (phase == BURN || phase == BURN_FAST) return fuel;
	// An igniting cell becomes a burning cell move_rate time units later,
	// and informs its neighbors that it is burning
	else if (phase == IGNITE) return min(move_left,fuel);
	// Otherwise, the cell just waits for something to happen
	else return DBL_MAX;
}

void fireCell::delta_int()
{
	// If cell is starting to burn
	if (phase == IGNITE) 
	{
		fuel -= ta(); // Lose fuel while igniting
		phase = BURN; // Start burning
	}
	// If the phase is burning
	else if (phase == BURN || phase == BURN_FAST) 
	{
		fuel = 0.0; // Fuel is gone
		phase = BURNED; // Change to BURNED
	}
	// Internal transitions should not occur otherwise
	else
	{
		assert(false);
	}
}

void fireCell::delta_ext(double e, const adevs::Bag<CellEvent>& xb)
{
	// If we are on fire, then update the fuel supply
	if (phase == IGNITE || phase == BURN || phase == BURN_FAST) 
	{
		move_left -= e;
		fuel -= e;
	}
	// Change heat level of this cell
	adevs::Bag<CellEvent>::const_iterator iter;
	for (iter = xb.begin(); iter != xb.end(); iter++)
	{
		heat += (*iter).value;
	}
	// If the cell can catch fire
	if (heat >= 2 && phase == UNBURNED && fuel > 0.0)
	{
		// If it will burn long enough to propagate
		if (fuel >= move_rate) phase = IGNITE;
		// Otherwise, burns to fast to affect anybody else
		else phase = BURN_FAST;
	}
}

void fireCell::delta_conf(const adevs::Bag<CellEvent>& xb) 
{
	delta_int();
	delta_ext(0.0,xb);
}

void fireCell::output_func(adevs::Bag<CellEvent>& yb)
{
	// Output only if cell is burning out or igniting
	if (phase == IGNITE || phase == BURN)
	{
		// Set the cell output value
		CellEvent e;
		// Starting to burn
		if (phase == IGNITE)
		{
			e.value = 1;
		}
		// Burning out
		else if (phase == BURN)
		{
			e.value = -1;
		}
		// Produce an event for each of the 8 neighbors
		for (int dx = -1; dx <= 1; dx++)
		{
			for (int dy = -1; dy <= 1; dy++)
			{
				e.x = x+dx;
				e.y = y+dy;
				// Don't send an event to self
				if (e.x != x || e.y != y)
				{
					yb.insert(e);
				}
			}
		}
	}
}

fireCell::state_t fireCell::getState()
{
	state_t s(phase,fuel,heat,move_left);
	return s;
}

void fireCell::beginLookahead()
{
	chkpt.setState(phase,fuel,heat,move_left);
}

void fireCell::endLookahead()
{
	phase = chkpt.phase;
	heat = chkpt.heat;
	move_left = chkpt.move_left;
	fuel = chkpt.fuel;
}

