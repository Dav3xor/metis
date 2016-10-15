#include "metis.hpp"

void MetisASM::assemble(const string &filename) {

};

MetisASM::MetisASM() : handlers( {
{"A", [](MetisVM &m, string &s) -> void {  m.add_end(); } },
{"B", [](MetisVM &m, string &s) -> void {  m.add_end(); } } }) {
};
