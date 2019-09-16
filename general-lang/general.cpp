#include "general.hpp"

std::unique_ptr<SyntaxTreeFactory> SyntaxTree::factory = unique_ptr<GeneralSyntaxTreeFactory>(new GeneralSyntaxTreeFactory());
