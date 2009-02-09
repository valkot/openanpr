//---------------------------------------------------------------------------
//  PROJECT     : Common Utility
//  DESCRIPTION : Pseudo Random Number Generator with 64bit Seed
//  FILE NAME   : randoml64.cpp
//  VERSION     : 1.3
//  AUTHOR      : Andy Thomas [www.alpha-india5.com]
//              : Trivial modifications for GNU compilation by Bob Mottram
//  COPYRIGHT   : Free of Copyright
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// The algorithm used here was based upon code appearing on Dan Hirschberg's
// website (Professor of Information and Computer Science at UCI), which itself
// was based on code appearing in "Random number generators: good ones are hard
// to find", by Stephen Park and Keith Miller. However, I have extended the
// internal seed storage from 32 to 64 bits.

// This material is free of copyright and may be copied or used for any purpose
// without royalty. However, if distributed in a near-verbatim form, I would
// appreciate you including this notice.

// REFERENCES:
// http://www.ics.uci.edu/~dan/
// http://www.ics.uci.edu/~dan/class/165/random.c
// Also web search on "random number"+"Miller" for everything you ever wanted to
// to know about random number generators.

// Cheers

// Andy Thomas, April 2001. [www.alpha-india5.com]
//---------------------------------------------------------------------------
#include "randomal64.h"
//---------------------------------------------------------------------------
// CLASS Randomal64 : PRIVATE METHODS
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// CLASS Randomal64 : PUBLIC METHODS
//---------------------------------------------------------------------------
Randomal64::Randomal64(unsigned long sd)
{
    // Seed constructor - also a default constructor
    seed(sd);
}
//---------------------------------------------------------------------------
void Randomal64::seed(unsigned long sd)
{
    pSeed = sd;
}
//---------------------------------------------------------------------------
int Randomal64::randInt(int range)
{
    // Return random integer32 between 0 & range value
    unsigned long hi = pSeed/RDX_Q;
    unsigned long lo = pSeed - (hi * RDX_Q);
    pSeed = ((RDX_CONST * lo) - (RDX_R * hi));
    if (pSeed == 0) pSeed = RDX_RANGE;

    int rslt = (int)(pSeed % (unsigned long)range);
    if (rslt < 0) rslt = -rslt;
    int rand_value = rslt;
    //cout << "pseed: " << pSeed << endl;
    //cout << "rand: " << rand_value << endl;
    return (rand_value);
}
//---------------------------------------------------------------------------
double Randomal64::rand()
{
    // Return double between 0 & 1
    int rslt = randInt(RDX_RANGE);
    return (double(rslt) / RDX_RANGE);
}
//---------------------------------------------------------------------------
float Randomal64::randFloat()
{
    // Return double between 0 & 1
    int rslt = randInt(RDX_RANGE);
    return (float(rslt) / RDX_RANGE);
}
