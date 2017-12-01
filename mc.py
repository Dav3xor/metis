#!/usr/bin/python

# an attempt at a compiler using shlex...
import shlex
import re
import sys
  
class Element:
  def __init__(self):
    self.children = []
    self.type     = "element"

  def add_child(self, child):
    self.children.append(child)
  def recurse(self):
    if hasattr(self,'children'):
      for child in self.children:
        if type(child) == str:
          print child
        child.render()

class Head(Element):
  def __init__(self, filename):
    Element.__init__(self)
  def render(self):
    Element.recurse(self)

class File(Element):
  def __init__(self, filename):
    self.filename = filename
    self.add_child(parse_file(filename))
  def render(self):
    print "rendering file"

class Include(Element):
  def __init__(self):
    Element.__init__(self)
  def render(self):
    print "rendering include"
    Element.recurse(self)

class Function(Element):
  def __init__(self, name):
    Element.__init__(self)
    self.args = []
    self.name = name
    self.return_type = None
  def render(self):
    print "# " + "-"*75
    print   "# function: " + self.name
    if len(self.args):
      print "# args:    ",
      args = [' '.join(i) for i in self.args]
      print ', '.join(args),
      print " "
    if self.return_type:
      print "# returns:  " + self.return_type
    print "# " + "-"*75
    print "PUSHR REGA, REGB, REGC REGD"
    Element.recurse(self)
    print "POPR REGA, REGB, REGC REGD"
    print "STORE REGIP, STACK_POP"
    print "# " + "-"*75
    print
    print

class FunctionCall(Element):
  def __init__(self, name):
    Element.__init__(self)
    self.name = name
  def render(self):
    print "rendering function call"
    Element.recurse(self)

class Stmt(Element):
  def render(self):
    print "rendering stmt"

class Expression(Element):
  def __init__(self):
    self.lvalue   = None
    self.operator = None
    self.rvalue   = None
  def render(self):
    print "rendering stmt"

class IfClause(Element):
  def __init__(self):
    Element.__init__(self)
  def render(self):
    print "rendering ifclause"
    Element.recurse(self)

class If(Element):
  def __init__(self):
    Element.__init__(self)
  def render(self):
    print "rendering if"
    Element.recurse(self)

class While(Element):
  def __init__(self):
    Element.__init__(self)
  def render(self):
    print "rendering else"
    Element.recurse(self)
    
class Foreach(Element):
  def __init__(self):
    Element.__init__(self)
  def render(self):
    print "rendering foreach"
    Element.recurse(self)

class Typename(Element): 
  def __init__(self, vartype, name):
    self.name = name
    self.type = vartype
  def render(self):
    print "rendering typedef"

class Typedef(Element): 
  def __init__(self, name):
    self.name = name
    Element.__init__(self)
  def render(self):
    print "rendering typedef"
    Element.recurse(self)
    
class Number(Element):
  def __init__(self, number):
    self.value = number
  def render(self):
    print "rendering number"

class Operator(Element):
  def __init__(self, operator):
    self.operator = operator
  def render(self):
    print "rendering operator"

class Vector(Element):
  def __init__(self):
    Element.__init__(self)
  def render(self):
    print "rendering vector"
    Element.recurse(self)

class Matrix(Element):
  def __init__(self):
    Element.__init__(self)
  def render(self):
    print "rendering matrix"
    Element.recurse(self)

class Boolean(Element):
  def __init__(self, value):
    self.value = value
  def render(self):
    print "rendering boolean"

class String(Element):
  def __init__(self, string):
    self.value = string
  def render(self):
    print "rendering string"

class Label(Element):
  def __init__(self, label):
    self.value = label
  def render(self):
    print "rendering label"
  
class Term(Element):  
  def __init__(self):
    Element.__init__(self)
  def render(self):
    print "rendering term"
    Element.recurse(self)

class Lexp(Element):  
  def __init__(self):
    Element.__init__(self)
  def render(self):
    print "rendering lexp"
    Element.recurse(self)

class Return(Element):  
  def __init__(self):
    Element.__init__(self)
  def render(self):
    print "rendering return"
    Element.recurse(self)

class Throw(Element):  
  def __init__(self):
    Element.__init__(self)
  def render(self):
    print "rendering throw"
    Element.recurse(self)

class Trait(Element):  
  def __init__(self):
    Element.__init__(self)
  def render(self):
    print "rendering trait"
    Element.recurse(self)

class Group(Element):  
  def __init__(self):
    Element.__init__(self)
  def render(self):
    print "rendering group"
    Element.recurse(self)

class Assignment(Element):  
  def __init__(self, varname, vartype):
    self.type = vartype
    self.name = varname
    Element.__init__(self)
  def render(self):
    print "rendering assignment"

def peek(tokens):
  token = tokens.get_token()
  tokens.push_token(token)
  return token

class SyntaxError(Exception):
  def __init__(self, message, tokens=None):
    if tokens:
      print "Syntax Error: " + message + " at line: " + str(tokens.lineno)
    else:
      print "Syntax Error: " + message


class LabelStack(object):
  def __init__(self):
    self.stack = [{}]
  def push_context(self):
    self.stack.append({})
  def pop_context(self):
    self.stack.pop()

  def add_label(self,label,value):
    self.stack[-1][label] = value
  def find_label(self,label, tokens):
    for i in reversed(self.stack):
      if label in i:
        return i[label]
    raise SyntaxError(" label does not exist - " + label, tokens)


labels = LabelStack()

atomic_types = {'string':1, 'bool':1, 
                'unsigned':1, 'integer':1, 
                'float':1, 'label':1, 
                'vector':1, 'matrix':1}

low_precedence = {'+':1,'-':1}

high_precedence = {'*':1, '/':1, '%':1, 'dot':1, 'cross':1}

def valid_label(token):
  if re.match('[a-zA-Z_][a-zA-Z0-9_]*', token):
    return token
  else:
    return None

def valid_integer(token):
  if re.match('[-+]?[0-9]+', token):
    return int(token)
  else:
    return None

def parse_number(tokens):
  cur      = peek(tokens)
  sign     = "positive"
  number   = "" 
  if cur == "-":
    number += cur
    num_type = "signed"
    sign = "negative"
    tokens.get_token()
    cur  = peek(tokens)

  if cur == ".":
    number += cur
    tokens.get_token()
    cur    = peek(tokens)

  if cur.isdigit():
    number   += cur 
    tokens.get_token()
    cur = peek(tokens)

  if "." in number:
    return number

  if cur == ".":
    number += cur
    tokens.get_token()
    cur  = peek(tokens)
  else:
    return number
 
  # read numbers past the . (we got a float...)
  if cur.isdigit():
    number   += cur 
    tokens.get_token() 

  return number



  if not num_type:
    return None

  # ok, we have a number...

  # first burn the current token...
  tokens.get_token()


  cur = peek(tokens)
  if cur == ".":
    
    num_type = "float"

      

def valid_float(token):
  if re.match('[-+]?([0-9*[.])?[0-9]+', token):
    return float(token)
  else:
    return None

def handle_typeident(tokens):
  print "typeident"
  vartype = peek(tokens)
  if vartype not in atomic_types:
    return None
  else:
    tokens.get_token()
    label = valid_label(tokens.get_token())
    labels.add_label(label,1)
    return (vartype, label)

def handle_args(tokens):
  args = []
  arg = handle_typeident(tokens) 
  if arg:
    args.append(arg)
    while True:
      comma = tokens.get_token()
      if comma == ',':
        arg = handle_typeident(tokens)
        if arg:
          args.append(arg)
      else:
        tokens.push_token(comma)
        break
  print args
  return args

def handle_beginfunction(tokens):
  print "functiondef"
  label = valid_label(tokens.get_token())

  if not label:
    raise SyntaxError("invalid label - " + label, tokens)

  f = Function(label)
  f.args = handle_args(tokens)
  
  if handle_return_arrows(tokens) == '<-':
    returntype = tokens.get_token()
    if returntype not in atomic_types:
      raise SyntaxError("unknown return type - " + returntype, tokens)
    f.return_type = returntype
    print returntype
  
  return f

def handle_functiondef(tokens):
  f = handle_beginfunction(tokens)
  colon = tokens.get_token()
  if colon != ':':
    raise SyntaxError("function declaration does not end in ':' - " + returntype, tokens)
  while peek(tokens) != "fin":
    f.add_child(handle_bs(tokens))
  print "end function"
  return f

def handle_assignment_operator(tokens):
  "handling assignment..."
  colon = peek(tokens)
  if colon != ':':
    return False
  tokens.get_token()
  equals = peek(tokens)
  if equals != '=':
    raise SyntaxError("= should follow :, not " + equals, tokens)
  tokens.get_token()
  return True

def handle_return_arrows(tokens):
  token = tokens.get_token()
  # handle return/returnnv/raise
  if token == "<":
    token += tokens.get_token()
    if token == "<-":
      israise = tokens.get_token()
      if israise == '!':
        token += israise
      else:
        tokens.push_token(israise)
    else:
      raise SyntaxError("statement start with < but not <-", tokens)
    return token
  else:
    tokens.push_token(token)
    return None

def handle_group(tokens):
  print "group"
  g = Group()
  g.add_child(handle_lexp(tokens))
  end = tokens.get_token()
  if end != ')':
   raise SyntaxError("grouped lexp doesn't end with ')'", tokens)
  return g

def handle_fcall(tokens):
  print "fcall"
  function_name = tokens.get_token()
  print "function = " + function_name

  f = FunctionCall(function_name)

  lexp = handle_lexp(tokens)
  while lexp:
    f.add_child(lexp)
    comma = peek(tokens)
    if comma != ",":
      break
    else:
      tokens.get_token() # consume the comma
    lexp = handle_lexp(tokens)
  return f

def handle_ffcall(tokens):
  print "ffcall"
  f = handle_fcall(tokens);
  end = tokens.get_token()
  if end != '}':
   raise SyntaxError("ffcall doesn't end with '}'", tokens)
  return f

def handle_vector(tokens):
  v = Vector()
  while peek(tokens) != '|':
    v.add_child(handle_lexp(tokens))
  # consume the end |
  tokens.get_token()
  return v

def handle_matrix(tokens):
  m = Matrix()
  while True:
    m.add_child(handle_vector(tokens))
    if peek(tokens) != '|':
      break
    # consume the next |
    tokens.get_token()
  return m

def handle_true(tokens):
  return Boolean(True)

def handle_false(tokens):
  return Boolean(False)

def handle_factor(tokens):
  factor_handlers = { '(': handle_group,
                      '{': handle_ffcall,
                      'true': handle_true,
                      'false': handle_false,
                      '|': handle_matrix }    
  print "factor"
  print peek(tokens)
  number = parse_number(tokens)
  if number:
    print "number: " + number
    if number[-1] == '.':
      tokens.push_token('.')
    return Number(number)
  else:
    token = peek(tokens)
    print token
    if token[0] == '"':
      print token[1:-1]
      return String(tokens.get_token()[1:-1])
    elif token in factor_handlers:
      tokens.get_token()
      return factor_handlers[token](tokens)
    elif valid_label(token):
      tokens.get_token()
      label = labels.find_label(token,tokens)
      return Label(label)
  return False

def handle_term(tokens):
  print "term"
  print peek(tokens)
  factor = handle_factor(tokens)
  if factor:
    term = Term()
    term.add_child(factor)
    operator = peek(tokens)
    while operator in high_precedence:
      term.add_child(Operator(operator))
      print operator
      tokens.get_token()
      term.add_child(handle_factor(tokens))
      operator = peek(tokens)
    print peek(tokens) 
    return term
  return False

def handle_lexp(tokens):
  print "lexp"
  print peek(tokens)
  term = handle_term(tokens)
  if term:
    lexp = Lexp()
    lexp.add_child(term)
    operator = peek(tokens)
    while operator in low_precedence:
      lexp.add_child(Operator(operator))
      tokens.get_token()
      lexp.add_child(handle_term(tokens))
      operator = peek(tokens)
    return lexp
  return False

def handle_return(tokens):
  print "return"
  print peek(tokens)
  r = Return()
  r.add_child(handle_lexp(tokens))
  return r

def handle_throw_exception(tokens):
  print "throw exception"
  print peek(tokens)
  throw = Throw()
  throw.add_child(handle_lexp(tokens))
  return throw

def handle_trait(tokens):
  print "trait"
  name = valid_label(tokens.get_token())
  trait = Trait()
  while peek(tokens) != "fin":
    trait.add_child(handle_bs(tokens))

  return trait

def handle_assignment(tokens):
  token   = tokens.get_token()
  vartype = None
  varname = None

  if token in atomic_types:
    vartype = token
    varname = tokens.get_token()
  else:
    varname = token
    print varname
  

  if handle_assignment_operator(tokens):
    print varname
    assignment = Assignment(varname,vartype)
    if not valid_label(varname):
      raise SyntaxError("variable name: " + varname + " is not a valid label")
    assignment.add_child(handle_lexp(tokens))
    return assignment
  else:
    if varname:
      tokens.push_token(varname)
    if vartype:
      tokens.push_token(vartype)
    return False

def handle_traitident(tokens):
  print "traitident"
  return handle_beginfunction(tokens)

    
def handle_stmt(tokens):
  # statements start with a return arrow, a colon, a type signature, or a label
  print "stmt"

  retval = False

  stmt_handlers = {'<-':      handle_return,
                   '<-!':     handle_throw_exception,
                   'trait':   handle_traitident}
                   

  # handle return/returnnv/raise
  token = handle_return_arrows(tokens)
  print "---" + peek(tokens) + "---" + str(token )
  if token in stmt_handlers:
    print "handler"
    #tokens.get_token()
    retval = stmt_handlers[token](tokens)

  # else, the next token is our thing...
  elif not token:
    token = peek(tokens)
    if token == "trait":
      print "traitident"
      tokens.get_token()
      retval = handle_traitident(tokens)
    else:
      print "stmt assignment"
      retval = handle_assignment(tokens)
      if not retval:
        print "stmt fcall"
        retval = handle_fcall(tokens)
  end = tokens.get_token()
  if end != ".":
    raise SyntaxError("stmt does not end in '.' (got '"+end+"' instead)", tokens)
  return retval

def handle_include(tokens):
  dash = peek(tokens)
  i = Include()
  while dash == '-':
    # consume the dash...
    tokens.get_token()
    # add filename to include list
    i.add_child(parse_file(tokens.get_token()))
    dash = peek(tokens)
  print peek(tokens)
  return i

def handle_exp(tokens):
  e = Expression()
  e.lvalue   = handle_lexp(tokens)
  operator   = tokens.get_token()
  e.operator = Operator(operator)
  e.rvalue   = handle_lexp(tokens)
  print "operator = " + operator
  print "end exp" + peek(tokens)
  return e

def handle_if(tokens):
  print "if"
  i = If()
  ic = IfClause()
  ic.add_child(handle_exp(tokens))
  while peek(tokens) not in ['else','fin']:
    ic.add_child(handle_bs(tokens))
  i.add_child(ic)
  token = peek(tokens)
  while token == "else":
    ic = IfClause()
    print "else"
    token = tokens.get_token()
    token = peek(tokens)
    if token == "if":
      print "if after else"
      tokens.get_token()
      ic.add_child(handle_exp(tokens) )
    while peek(tokens) not in ['else','fin']:
      ic.add_child(handle_bs(tokens))
  return ic


def handle_while(tokens):
  print "while"
  w = While()
  w.add_child(handle_exp(tokens))
  while peek(tokens) != "fin":
    w.add_child(handle_bs(tokens))
  return w

def handle_foreach(tokens):
  f = Foreach()
  label = valid_label(tokens.get_token())
  if label:
    f.add_child(Label(label))
    labels.add_label(label,1)
  inx   = tokens.get_token()
  if inx != "in":
    raise SyntaxError("foreach requires 'in' keyword")

  f.add_child(handle_factor(tokens))
  
  while peek(tokens) != "fin":
    f.add_child(handle_bs(tokens))
  return f

def handle_typedef(tokens):
  print "typedef"
  name = tokens.get_token()
  t = Typedef(name)
  typedef_handlers = {'def':      handle_functiondef,
                      ':':        handle_trait}
                   
  while peek(tokens) != "fin":
    ti = handle_typeident(tokens)
    if ti:
      t.add_child(Typename(*ti))
      dot = tokens.get_token()
      if dot != ".":
        raise SyntaxError("member declaration in type doesn't end with '.'", tokens)
    elif peek(tokens) in typedef_handlers:
      t.add_child(typedef_handlers[tokens.get_token()](tokens))
      if peek(tokens) != "fin":
        raise SyntaxError("block in type declaration doesn't end with 'fin'")
      else:
        tokens.get_token()
  print "end typedef"
  return t 
block_handlers = {'if':        handle_if,
                  'include':   handle_include,
                  'while':     handle_while,
                  'foreach':   handle_foreach,
                  'def':       handle_functiondef,
                  'type':      handle_typedef}

def handle_block(tokens):
  print "block"
  if peek(tokens) in block_handlers:
    # do stuff
    return block_handlers[tokens.get_token()](tokens)
    return True
  else:
    return False
  
  #end = tokens.get_token()
  #if end != "fin":
  #  raise SyntaxError("block does not end in 'fin'", tokens)

def handle_bs(tokens):
  # shlex removes comments for us.
  token = peek(tokens)
  print "bs - " + token
  b = handle_block(tokens)
  if b:
    tokens.get_token()
    return b
  
  s = handle_stmt(tokens)
  if s:
    print "end stmt"
    return s
  else:
    print "bad bs?"
    return False





def parse_file(filename):
  if not (filename.endswith('.m') or filename.endswith('.M')):
    filename += ".m"
  
  h = Head(filename.split('.')[0])

  with open(filename,"r") as input:
    input = input.read()

    lexer = shlex.shlex(input)

    while peek(lexer):
      h.add_child(handle_bs(lexer))
    print dir(h)
    h.render()
  return h

if __name__ == '__main__':
  parse_file(sys.argv[1]) 
