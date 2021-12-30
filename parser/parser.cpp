#include<cctype>
#include<cstdio>
#include<cstdlib>
#include<map> 
#include<memory>
#include<string>
#include<utility>
#include<vector>
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "ast/ast.cpp"
/// CurTok/getNextToken-  Provide a simple buffer. CurTok is the current
/// token the parser is looking at. getNextToken reads another token from the
/// lexer and updates CurTok with its results.

static int CurTok;
static int getNextToken() {
    return CurTok = gettok();
} 

/// LogError* - These are little helper functions for error handling.

std::unique_ptr<ExprAST> LogError(const char *Str) {
    fprintf(stderr, "LogError: %s\n", Str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str){
    LogError(Str);
    return nullptr;
}

/// numberexpr ::= number 
static std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(NumVal);
    getNextToken(); // consume the number 
    return std::move(Result); 
}

/// parenexpr ::= '(' expression ')' 

static std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken(); // eat (.
    auto V = ParseExpression();
    if(!V)
       return nullptr;

     if(CurTok != ')')
       return LogError("expected ')'");
       getNextToken(); //eat ).

    return V;  
} 

/// identiferexpr 
/// ::= identifier 
/// ::= identifier '(' expression* ')' 

static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = IdentifierStr;

    getNextToken(); // eat identifier 

    if(CurTok != '(') // Simple variable ref.
      return std::make_unique<VariableExprAST>(IdName);

      // Call.
      getNextToken(); // eat ( 
      std::vector<std::unique_ptr<ExprAST>> Args;
      if(CurTok != ')'){
          while(1){
              if(auto Arg = ParseExpression())
                 Args.push_back(std::move(Arg));
               else
                return nullptr;

             if(CurTok == ')')
              break;

             if(CurTok != ',')
               return LogError("Expected ')' or ',' in argument list");
            getNextToken();         
          }
      }  
       //Eat the ')' 
       getNextToken();
     return std::make_unique<CallExprAST>(IdName,std::move(Args));   
}

/// primary 
/// ::= identifierexpr
/// ::= numberexpr 
/// ::=parenexpr 

static std::unique_ptr<ExprAST> ParsePrimary() {
  switch(CurTok) {
    default: 
       return LogError("unknown token when expecting an expression");
    case tok_identifier:
        return ParseIdentifierExpr();
    case tok_number: 
         return ParseNumberExpr();
    case '(':
          return ParseParenExpr();         
  }
}

/// BinopPrecedence - This holds the precedence for each binary operator
/// defined. 

static std::map<char,int> BinopPrecedence;

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence() {
  if(!isascii(CurTok))
     return -1; 

  // Make sure it's a declared binop.
  int TokPrec = BinopPrecedence[CurTok];
  if(TokPrec <= 0)
    return -1;
  return TokPrec;  
}

/// expression 
/// ::= primary binoprhs 

static std::unique_ptr<ExprAST> ParseExpression() {
  auto LHS = ParsePrimary();
  if(!LHS)
    return nullptr;

  return ParseBinOpRHS(0, std::move(LHS));  
}

/// binorphs 
/// ::= ('+' primary)* 
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                              std::unique_ptr<ExprAST> LHS){
  // If this is a binop, find its precedence.
  while(1){
    int TokPrec = GetTokPrecedence();

    //If this is binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done. 

    if(TokPrec < ExprPrec)
      return LHS;

    // OKay, we know this is a  binop.
    int BinOp = CurTok;
    getNextToken(); // eat binop 

    //Parse the primary expression after the binary operator.
    auto RHS = ParsePrimary();
    if(!RHS)
       return nullptr;
    
    // If BinOp binds less tightly with RHS than the operator after RHS, let 
    // the pending operator take RHS as its LHS.
    int NextPrec = GetTokPrecedence();
    if(TokPrec < NextPrec){
      RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
      if(!RHS)
         return nullptr;
    }

    // Merge LHS/RHS. 
    LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
  }

 }

 /// Prototype 
 /// ::= id '(' id* ')' 
 static std::unique_ptr<PrototypeAST> ParsePrototype() {
   if(CurTok!=tok_identifier)
      return LogErrorP("Expected function name in prototype");

    std::string FnName = IdentifierStr;
    getNextToken();

    if(CurTok != '(')
      return LogErrorP("Expected '(' in prototype");

    // Read the list of argument names. 
     std::vector<std::string> ArgNames;
     while(getNextToken() == tok_identifier)  
       ArgNames.push_back(IdentifierStr);

     if(CurTok != ')')
       return LogErrorP("Expected ')' in protoype");

     //success 
     getNextToken(); // eat ')' 

     return std::make_unique<PrototypeAST>(FnName,std::move(ArgNames));      
 }
  
  /// definition ::= 'def' prototype expression 
   static std::unique_ptr<FunctionAST> ParseDefinition() {
     getNextToken(); // eat def 
     auto Proto = ParsePrototype();
     if(!Proto) return nullptr;

     if(auto E = ParseExpression())
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));

      return nullptr;  
   }

  
  /// external ::= 'extern' protoype
  static std::unique_ptr<PrototypeAST> ParseExtern() {
    getNextToken() ; //eat extern 
    return ParsePrototype();
  }
  
  /// toplevelexpr::= expression 

  static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    if(auto E = ParseExpression()){
     // Make an anonymous proto. 
     auto Proto = std:: make_unique<PrototypeAST>("",std::vector<std::string>());
     return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
  }
   
   // Top Level Parsing---------------------------- 


   static void HandleDefinition(){
     if(ParseDefinition())
     fprintf(stderr, "Parsed a fucntion definition. \n");
     else 
     getNextToken(); // skip token for error recovery
   }

   static void HandleExtern(){
     if(ParseExtern())
     fprintf(stderr, "Parsed an extern\n");
     else
     getNextToken();
   }

   static void HandleTopLevelExpression(){
   // Evalute a top level expression ito an anonymous function.
   if(ParseTopLevelExpr())
   fprintf(stderr, "Parsed a top level expr\n");
   else
   getNextToken();
   }

  /// top ::= definition / external / expression / ';' 

  static void MainLoop(){
    while(true){
      fprintf(stderr,"ready> ");
      switch(CurTok){
        case tok_eof:
        return;
        case ';': // ignore top level semicolons. 
        getNextToken();
        break;
        case tok_def:
        HandleDefinition();
        break;
        case tok_extern:
        HandleExtern();
        break;
        default: 
        HandleTopLevelExpression();
        break;
      }
    }
  }

int main(){
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40;
  // Prime the first token.
  fprintf(stderr, "ready> ");
  getNextToken();

  // Run the main "interpreter loop" now.
  MainLoop();

  return 0;
} 