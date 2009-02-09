//---------------------------------------------------------------------------
//  PROJECT     : Common Utility
//  DESCRIPTION : Pseudo Random Number Generator with 64bit Seed
//  FILE NAME   : randoml64.h
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
#ifndef randomal64H
#define randomal64H

#include <iostream>
using namespace std;

//---------------------------------------------------------------------------
// Randomal64 Constants
//---------------------------------------------------------------------------
static const unsigned int RDX_RANGE = 0x7FFFFF;
static const unsigned int RDX_CONST = 0x00000000000041A7;
//static const unsigned int RDX_RANGE = 0x7FFFFFFF;
//static const unsigned int RDX_CONST = 0x00000000000041A7;
static const unsigned int RDX_Q = RDX_RANGE / RDX_CONST;
static const unsigned int RDX_R = RDX_RANGE % RDX_CONST;
//---------------------------------------------------------------------------
// CLASS Randomal64
//---------------------------------------------------------------------------
class Randomal64
{
private:
    unsigned long pSeed;

public:
    Randomal64(unsigned long sd = 0);
    void seed(unsigned long sd);
    int randInt(int range = 0x7FFFFFFF);
    double rand();
    float randFloat();
};
//---------------------------------------------------------------------------
#endif
