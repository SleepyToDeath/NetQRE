#include "../parser.h"
#include <iostream>

int main()
{
	Netqre::NetqreParser parser;
	Rubify::string program = 
	"{program,{#filter,{filter([_]),},},{#qre_top,{#qre_vs,{#agg_op,{sum,},},{#qre_ps,{#qre_ps,{#qre_ps,},{#agg_op,},},{#qre_ps,{#re,{#predicate_set,{#predicate_entry,},},},},{#agg_op,{max,},},},{#feature_set,{\r0,{1,},},},},},{#threshold,{threshold(0),},},}";
	parser.parse(program);
	std::cerr<<std::endl;
}

