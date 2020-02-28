#ifndef __proba_hpp_
#define __proba_hpp_
//#pragma once
/*
#include <iostream>
#include <fstream>
#include <list>
#include <utility>
#include <iterator>
#include <algorithm> */



#include <cmath>
#include <ctime>
#include "mt19937.cpp"



class Time(){// time class is the internal representation of time
public:
    Time(): t(time(0)){};

private:

double t ;



};

void init_alea(bool Alea = 1) // Initialisation du générateur avec le temps informatique si Alea=True, avec valeur constante sinon
{
    if (Alea)
        init_genrand( static_cast<unsigned int>(time(0)) );
    else
        init_genrand( 31 );
};



struct var_alea
{
    var_alea() : value(0) {};
    var_alea(double value) : value(value) {};
    virtual ~var_alea() {};
    virtual double operator()() = 0;
    double current() const { return value; };

    protected:
    	double value;
};


struct uniform : public var_alea // va uniforme sur [left,right]
{
    uniform(double left = 0, double right = 1) : left(left), size(right-left), genrand(genrand_real1) {};
    double operator()()
    {
        return value = left + size * genrand();
    };

    private:
        double left, size;
        double (*genrand)(void);
};


struct exponent : public var_alea // exponentielle de parametre lambda
{


    exponent(doube x=0 ): lambda(x) {};
   double operator() ()
   {
    uniform U;
      return value= -log(U()/lambda);
     };
    private:

   double lambda;

};

struct bernoulli : public var_alea

{
    public :
    bernoulli(double p ): parameter(p) {};
    double operator () (double p)
    {
        uniform U ;
        if (U()<p ){return -1;}
        else {return 1;}
    }
    private:
        double parameter ;
};

/* *********************************************************************************
Abstract class processus

**********************************************************************************
template <typename T>
class Process
{
public:
	// typedef pour avoir acces a au type de retour (liste) et pouvoir la parcourir
	typedef std::pair<Time, T> state; // (temps,valeur)
	typedef std::list<state> result_type;
	typedef typename result_type::iterator iter; //
	typedef typename result_type::const_iterator cst_iter; //


	Process(int size = 0) : value(size) {}; // Constructeur
	virtual result_type operator()() = 0; // methode virtuelle pure
//	virtual result_type GeneratePath() { return this->operator()(); }; // destine a remplacer la surcharge de ()
	result_type current() const { return value; };
	T & TerminalValue() { return value.back().second; };
// Type de retour a modifier en functor variable aleatoire
// doit-on renvoyer un (smart) pointer ou une reference sur la derniere valeur du processus?
//	T & GenerateTerminalValue() {
//		this->operator()();
//		return value.back().second;
//	};
	std::function<T()> GenerateTerminalValue() {
		return [this]() {
			this->operator()();
			return value.back().second;
		};
	};
//	auto FinalValue(){return lastvalue;};
	// Surcharge de << en amie
	template <typename S>
	friend std::ostream& operator<<(std::ostream &o, const Process<S>& p);

protected:
	result_type value;
/*	class terminalvalue : public var_alea<T> {
		T operator()(){
			this->operator()();
			return this->value; //((this->value).back()).second;
		}
	} lastvalue;
*/
/*};


// Surcharge de l'operateur <<
template <typename T>
std::ostream& operator <<(std::ostream &o, const Process<T> &p){
  typename Process<T>::cst_iter i;
  for(i = p.value.begin(); i != p.value.end(); ++i)
	o << (*i).first << "\t" << (*i).second << std::endl;
  return o;
}


/*  *********************************************************************************
Poisson process class
This class simulates the jump times and values of a Poisson process with intensity \lambda
**********************************************************************************
class PoissonProcess : public Process<unsigned>
{
public:
	PoissonProcess(double lambda, double T = 1.) : E(lambda), T(T) {};
	result_type operator()() {
		value.clear();
		state val_k(0, 0);
		value.push_back(val_k);
		val_k.first += E();
		while (val_k.first < T) {
			val_k.second += 1;
			value.push_back(val_k);
			val_k.first += E();
		};
		val_k.first = T;
		value.push_back(val_k);
		return value;
	};
	friend class CompoundPoissonProcess;
	friend std::ostream& operator<<(std::ostream &o, const PoissonProcess  &p);

protected:
	double T;
	expo E;
};

*/
#endif
