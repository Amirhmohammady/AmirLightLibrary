#pragma once
#include "AmirLog.h"
#include <vector>

class AmirParser{
public:
    typedef double(*operatorfunction)(double a, double b);
	AmirParser();
    void operator = (AmirString formula);
	double getResult();
	void addVariable(AmirString text, double value);
	void removeVariable(AmirString text);
	void addOperator(char letter, unsigned int priority, operatorfunction optfunc);
	void removeOperator(char letter);
	~AmirParser();
private:
    enum NodeType{
        NUMBER, OPERATOR, VARIABLE, PARENTHESIS, NONE
    };
    struct Node{
        NodeType type;
        unsigned int ID;
    };
    struct OptrFuncion{
        char letter;
        unsigned int priority;
        operatorfunction func;
    };
    struct Variable{
        AmirString text;
        double value;
    };
    //-----------------------------------------------------
	static double minusFunction(double a, double b);
	static double addFunction(double a, double b);
	static double zarbFunction(double a, double b);
	static double taghsimFunction(double a, double b);
	static double powFunction(double a, double b);
    //-----------------------------------------------------
	double calculate();
    void removeNoneNodes();
	void addNode(NodeType type, char** eip);
	void convertToNode();
	void removeParentheses();
    double double_result;
	char* source = nullptr;
    //-----------------------------------------------------
    std::vector<Node> nodes;
	std::vector<OptrFuncion> operators;
	std::vector<OptrFuncion> operatorlist;
	std::vector<Variable> variables;
	std::vector<bool> parentheses;//true = open false = close
	std::vector<double> numbers;
};
//===============================================================================
//===============================================================================
class Rational
{
public:
    Rational(long long x,long long y);
    Rational(long long x);
    Rational(double x);
    void operator =(const Rational &r1);
    Rational operator *(const Rational &r1);
    Rational operator /(const Rational &r1);
    Rational operator +(const Rational &r1);
    Rational operator -(const Rational &r1);
    void operator =(double d1);
    Rational operator *(double d1);
    Rational operator /(double d1);
    Rational operator +(double d1);
    Rational operator -(double d1);
    void operator *=(const Rational &r1);
    void operator /=(const Rational &r1);
    void operator +=(const Rational &r1);
    void operator -=(const Rational &r1);
    void operator *=(double d1);
    void operator /=(double d1);
    void operator +=(double d1);
    void operator -=(double d1);
    bool operator ==(const Rational &r1);
    std::string toString();
private:
    unsigned long long gcd(unsigned long long m, long long n);
    Rational convertDoubleToRational(double d1);
    unsigned long long numerator;
    long long denominator;
};
