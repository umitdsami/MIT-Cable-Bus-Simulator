#pragma once


#include "resource.h"

static Lps::Uint32 namecounter = 0; //assigns a unique incremental id to each created leaps objects (components, connections, nodes, diagrams, etc)
static Lps::Uint32 mid_bus = 2; //to build a minimal ring bus two mid bus is required (ALWAYS MUST BE AN EVEN NUMBER SO YOU HAVE ONE FOR LEFT MID AND ONE FOR RIGHT MID)
static Lps::Uint32 fwr_bus = 1;
static Lps::Uint32 aft_bus = 1;
static Lps::Uint32 total_bus = fwr_bus + mid_bus + aft_bus;
static Lps::Uint32 linknum = 0;
static Lps::Uint32 bussecnum = 0;
static Lps::Uint32 iloc = 1; 