#include<cctype>
#include<cstdio>
#include<cstdlib>
#include<map> 
#include<memory>
#include<string>
#include<utility>
#include<vector> 

/// ExprAST - Base class for all expression nodes. 

class ExprAST {
public:
  virtual ~ExprAST() {}
};

/// NumberEXprAST - Expression class for numeric literal like "1.0".

class NumberExprAST : public ExprAST {
  double Val;

  public:
    NumberExprAST(double Val) : Val(Val){}
};

/// VariableExprAST - Expression class for referencing a variable, like "a". 

class VariableExprAST : public ExprAST {
    std::string Name;

   public: 
   VariableExprAST(const std::string &Name) : Name(Name) {}
};

/// BinaryExprAST - Expression class for a binary operator. 

class BinaryExprAST : public ExprAST {
  char Op;
  std::unique_ptr<ExprAST> LHS, RHS;

  public: 
  BinaryExprAST(char op, std::unique_ptr<ExprAST> LHS, std:: unique_ptr<ExprAST> RHS): 
              Op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

/// CallExprAST - Expression class for function calls.

 class CallExprAST: public ExprAST {
  std:: string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

  public: 
   CallExprAST(const std::string &Callee,
   std::vector<std::unique_ptr<ExprAST>> Args) :
   Callee(Callee), Args(std::move(Args)){}
 };