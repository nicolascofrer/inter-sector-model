

#ifndef __mt19937_hpp_
#define __mt19937_hpp_

void init_genrand(unsigned long s);

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void);

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void);

/* generates a random number on [0,1]-real-interval */
double genrand_real1(void);

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void);

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void);

/* generates a random number on (0,1]-real-interval */
double genrand_real4(void);

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void);

#endif