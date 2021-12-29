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

 
int main(){
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40;
} 