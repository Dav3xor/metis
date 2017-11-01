#include "metis.hpp"



// TODO: work on cleaning this up... 
// 1. move into separate file
// 2. abstract sections better
void MetisVM::save(const string &filename) {
  ofstream outfile(filename, ios::out|ios::binary);
  if(!(outfile.good())) {
    throw MetisException("could not open file for save: " +filename,__LINE__,__FILE__);
  }
  outfile.write((const char *)header,10);
  uint16_t header_len = 0;
  uint64_t buffer_len;
  outfile.write("H",1);
  outfile.write((char *) &header_len, 2);
  for (auto kv : labels) {
    uint16_t label_len = kv.first.length();
    outfile.write("L",1);
    outfile.write((char *) &label_len,2);
    outfile.write(kv.first.c_str(), kv.first.length());
    outfile.write((char *) &kv.second.cell.ulong, sizeof(uint64_t));
  }
  buffer_len = buffer_end-buffer;
  if(buffer_len > 0) {
    outfile.write("B",1);
    outfile.write((char *)&buffer_len,8);
    outfile.write((char *)buffer, (uint64_t)(buffer_end-buffer));
  }
  uint64_t code_len = registers[REGIP].ulong-(uint64_t)code_start;
  outfile.write("C",1);
  outfile.write((char *)&code_len, 8);
  outfile.write((char *)code_start, code_len);
  outfile.close();
}



void MetisVM::load(const string &filename) {
  char label[MAX_LABEL_LEN+1];
  uint16_t label_len;
  uint16_t header_len;
  uint64_t code_len;
  uint64_t buffer_len;
  uint64_t value;
  TypedCell cell;
  reset();
  labels.clear();

  ifstream infile(filename, ios::in|ios::binary);
  if(!(infile.good())) {
    throw MetisException("could not open file for read: " +filename,__LINE__,__FILE__);
  }

  infile.read((char *)header,10);
  header[10] = '\0';
  while(!(infile.eof())) {
    char type;
    infile.read(&type,1);
    switch(type) {
      case 'H':     // header, ignore for now...
        infile.read((char *)&header_len, 2);
        break;
      case 'L':     // label
        infile.read((char *)&label_len,2);
        if (label_len > MAX_LABEL_LEN) {
          throw MetisException("label too big?!? (load)",__LINE__,__FILE__);
        }
        infile.read(label, label_len);
        label[label_len]='\0';

        infile.read((char *)&value,8);
       
        cell.cell.ulong = value;
        cell.type = TypedCell::TYPE_ULONG;

        labels[label] = cell;
        break;
      case 'B':     // buffer
        infile.read((char *)&buffer_len,8);
        if ((uint64_t)(buffer_end + buffer_len - buffer) > buffer_size) {
          throw MetisException("buffer too big?!? (load)",__LINE__,__FILE__);
        }
        infile.read((char *)buffer, buffer_len);
        buffer_end += buffer_len;
        break;

      case 'C':     // code
        // TODO: allow multiple code segments, load 
        //       them one after another...
        infile.read((char *)&code_len,8);
        if (code_len > (uint64_t)(code_end-registers[REGIP].ulong)) {
          throw MetisException("code too big?!? (load)",__LINE__,__FILE__);
        }
        infile.read((char *) registers[REGIP].ulong, code_len);
        registers[REGIP].ulong += code_len; 
        break;
    }
  }
  infile.close();
}
