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

static std::unique_ptr<ExprAST> ParseIdentiferExpr() {
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