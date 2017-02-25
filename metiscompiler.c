#include "compiler.h"
#include "grammar.h"

handler *handlers = NULL;
handler *lexphandlers = NULL;
handler *bshandlers = NULL;
handler *blockhandlers = NULL;
handler *stmthandlers = NULL;

handler lexp_handlers[] = { {"lexp|term|factor|>",                &handle_comment, {0}},
                        {"lexp|term|>",                       &handle_comment, {0}},
                        {"lexp|term|factor|label|regex",      &handle_comment, {0}},
                        {"lexp|term|factor|float|regex",      &handle_comment, {0}}
                      };

handler handler_defs[] = { {"bs|comment|longcomment|regex",   &handle_comment, {0}},
                           {"bs|comment|shortcomment|regex",  &handle_comment, {0}},
                           {"bs|block|>",                     &handle_block, {0}},
                           {"function|>",                     &handle_function, {0}},
                           {"label|regex",                    &handle_label, {0}},
                           {"lexp|term|factor|integer|regex", &handle_integer, {0}},
                           {"lexp|term|factor|float|regex",   &handle_float, {0}},
                           {"fcall|>",                        &handle_fcall, {0}},
                           {"string",                         &handle_string, {0}}
                         };

handler bs_handlers[] =  { {"bs|comment|longcomment|regex",   &handle_comment, {0}},
                           {"bs|comment|shortcomment|regex",  &handle_comment, {0}},
                           {"bs|block|>",                     &handle_block, {0}},
                           {"bs|stmt|>",                      &handle_stmt, {0}}
                         };

handler block_handlers[] = { {"typedef|>",                   &handle_type, {0}},
                             {"if|>",                        &handle_if, {0}},
                             {"include|>",                   &handle_include, {0}},
                             {"while|>",                     &handle_while, {0}},
                             {"for|>",                       &handle_for, {0}},
                             {"functiondef|>",               &handle_def, {0}} 
                           };

handler stmt_handlers[] =  { {"return|>",                    &handle_return, {0}}
                           };
unsigned int num_handlers = sizeof(handler_defs)/sizeof(handler);
unsigned int num_lexp_handlers = sizeof(lexp_handlers)/sizeof(handler);
unsigned int num_bs_handlers = sizeof(bs_handlers)/sizeof(handler);
unsigned int num_block_handlers = sizeof(block_handlers)/sizeof(handler);
unsigned int num_stmt_handlers = sizeof(stmt_handlers)/sizeof(handler);


int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  mpc_result_t r;
  mpc_ast_t *ast;

  parser_state state;

  PARSER(Label,        "label");
  PARSER(String,       "string");
  PARSER(File,         "file");
  PARSER(Unsigned,     "unsigned");
  PARSER(Integer,      "integer");
  PARSER(Float,        "float");
  PARSER(Vector,       "vector");
  PARSER(Matrix,       "matrix");
  PARSER(Fcall,        "fcall");
  PARSER(Term,         "term");
  PARSER(Lexp,         "lexp");
  PARSER(Type,         "type");
  PARSER(Typeident,    "typeident");
  PARSER(Comparator,   "comparator");
  PARSER(Exp,          "exp");
  PARSER(Return,       "return");
  PARSER(Returnnv,     "returnnv");
  PARSER(Raise,        "raise");
  PARSER(Assignment,   "assignment");
  PARSER(Shortcomment, "shortcomment");
  PARSER(Longcomment,  "longcomment");
  PARSER(Comment,      "comment");
  PARSER(Args,         "args");
  PARSER(Bs,           "bs");
  PARSER(Function,     "function");
  PARSER(Trait,        "trait");
  PARSER(Traitident,   "traitident");
  PARSER(If,           "if");
  PARSER(Include,      "include");
  PARSER(While,        "while");
  PARSER(For,          "for");
  PARSER(Typedef,      "typedef");
  PARSER(Functiondef,  "functiondef");
  PARSER(Block,        "block");
  PARSER(Stmt,         "stmt");
  PARSER(Factor,       "factor");
  PARSER(Metis,        "metis");


  for(uint64_t i=0; i<num_handlers; i++) {
    handler *cur = &(handler_defs[i]);
    HASH_ADD_STR(handlers, handle, cur);
  }
  
  for(uint64_t i=0; i<num_lexp_handlers; i++) {
    handler *cur = &(lexp_handlers[i]);
    HASH_ADD_STR(lexphandlers, handle, cur);
  }
  for(uint64_t i=0; i<num_bs_handlers; i++) {
    handler *cur = &(bs_handlers[i]);
    HASH_ADD_STR(bshandlers, handle, cur);
  }
  for(uint64_t i=0; i<num_block_handlers; i++) {
    handler *cur = &(block_handlers[i]);
    HASH_ADD_STR(blockhandlers, handle, cur);
  }
  for(uint64_t i=0; i<num_stmt_handlers; i++) {
    handler *cur = &(stmt_handlers[i]);
    HASH_ADD_STR(stmthandlers, handle, cur);
  }

  mpca_lang(MPCA_LANG_DEFAULT, (char *)grammar_txt,
            String,  Label, File, Unsigned, Integer, 
            Float, Vector, Matrix, Fcall,
            Term, Lexp, Type, 
            Typeident, Comparator, 
            Exp, Return, Returnnv, Raise, Assignment, Shortcomment, Longcomment, Comment, Args, 
            Bs, Trait, Traitident, Function, 
            If, Include, While, For, Typedef, Functiondef, Block, Stmt, Factor, Metis, NULL);

  mpc_print(Label);
  mpc_print(String);
  mpc_print(File);
  mpc_print(Unsigned);
  mpc_print(Integer);
  mpc_print(Float);
  mpc_print(Vector);
  mpc_print(Matrix);
  mpc_print(Fcall);
  mpc_print(Term);
  mpc_print(Lexp);
  mpc_print(Type);
  mpc_print(Typeident);
  mpc_print(Comparator);
  mpc_print(Exp);
  mpc_print(Return);
  mpc_print(Returnnv);
  mpc_print(Raise);
  mpc_print(Assignment);
  mpc_print(Shortcomment);
  mpc_print(Longcomment);
  mpc_print(Comment);
  mpc_print(Args);
  mpc_print(Bs);
  mpc_print(If);
  mpc_print(Include);
  mpc_print(While);
  mpc_print(For);
  mpc_print(Typedef);
  mpc_print(Functiondef);
  mpc_print(Block);
  mpc_print(Stmt);
  mpc_print(Function);
  mpc_print(Trait);
  mpc_print(Traitident);
  mpc_print(Factor);
  mpc_print(Metis);
  
  if(mpc_parse_contents("test.m", Metis, &r)) {
    ast = r.output;
    mpc_ast_print(r.output);
    //mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
    mpc_cleanup(18, Label, String, File, Unsigned, Integer, Float, Vector,
                    Matrix, Fcall, Term, Lexp, Type, Typeident, Comparator,
                    Exp, Return, Returnnv, Raise, Assignment, Comment, Shortcomment, 
                    Args, Bs, If, Include, While, For, Typedef, Functiondef, Block, Stmt, Function, Trait, 
                    Traitident, Factor, Metis);
    return EXIT_FAILURE;
  }



  handle_start(&state, mpc_ast_traverse_start(ast, mpc_ast_trav_order_pre));

}

