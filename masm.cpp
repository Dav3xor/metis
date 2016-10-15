#include "metis.hpp"

void MetisASM::assemble(const string &filename) {

};

MetisASM::MetisASM() : handlers({
{"ERROR",               [](MetisVM &m, char *s) -> void {  m.add_error(); } },
{"END",                 [](MetisVM &m, char *s) -> void {  m.add_end(); } }, 
{"NOOP",                [](MetisVM &m, char *s) -> void {  m.add_noop(); } }, 
{"JUMP",                [](MetisVM &m, char *s) -> void {  m.add_noop(); } } }) {
};
