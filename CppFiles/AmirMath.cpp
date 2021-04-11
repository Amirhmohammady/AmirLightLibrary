#include "../Headers/AmirMath.h"
#include "../Headers/AmirException.h"
#include "../Headers/AmirLib1Globals.h"
#include <cstring>
#include <math.h>
#include <float.h>

using namespace std;

AmirParser::AmirParser()
{
    addOperator('+', 1, addFunction);
    addOperator('-', 1, minusFunction);
    addOperator('*', 2, zarbFunction);
    addOperator('/', 2, taghsimFunction);
    addOperator('^', 3, powFunction);
}
//===============================================================================
double AmirParser::calculate()
{
    //clear all vector expect operator list and variables
    double_result = 0;
    numbers.clear();
    parentheses.clear();
    operators.clear();
    nodes.clear();
    _AMIRLOG(AmirLib1LOG, _INFO, "numbers, parentheses, operators, nodes cleared and result become zero.");
    convertToNode();
    removeParentheses();
    _AMIRLOG(AmirLib1LOG, _INFO, "parentheses removed.");
    while(nodes.size()>1)
    {
        unsigned int maxperiority = 0, optID;
        for(unsigned int i=1; i<nodes.size(); i++)
            if (nodes[i].type == OPERATOR)
            {
                if (operators[nodes[i].ID].priority>maxperiority)
                {
                    maxperiority = operators[nodes[i].ID].priority;
                    optID = i;
                }
            }
        if (nodes[optID-1].type != NUMBER || nodes[optID+1].type != NUMBER)
            throw AmirException(AmirString("Nodes between operator '")<<operators[nodes[optID].ID].letter <<"' are not number!");
        numbers[nodes[optID+1].ID] = operators[nodes[optID].ID].func(numbers[nodes[optID-1].ID],numbers[nodes[optID+1].ID]);
        nodes[optID].type = NONE;
        nodes[optID-1].type = NONE;
        removeNoneNodes();
    }
    if (!nodes.empty()) double_result = numbers[nodes.back().ID];
    return double_result;
}
//===============================================================================
void AmirParser::operator = (AmirString formula)
{
    unsigned int s = formula.size();
    if (source) delete [] source;
    source = new char[s];
    memcpy(source, formula.c_str(), s);
    source[s] = 0;
    _AMIRLOG(AmirLib1LOG, _INFO, AmirString(formula)<<" was copied to source successfully.");
    calculate();
}
//===============================================================================
void AmirParser::convertToNode()
{
    char *eip = source;
    while(*eip)
    {
        if (*eip == ' ' || *eip == '\t')
        {
            eip++;
            continue;
        }
        if (*eip == '-')
        {
            //if last node be number or variable push_back operator - else push_back number -1 and operator *
            if (nodes.empty() || (nodes.back().type==PARENTHESIS && parentheses[nodes.back().ID]==true))
            {
                Node temp;
                temp.ID = numbers.size();
                temp.type = NUMBER;
                numbers.push_back(-1);
                nodes.push_back(temp);
                unsigned int i;
                for (i=0; i<operatorlist.size(); i++)
                    if (operatorlist[i].letter == '*')
                    {
                        temp.ID = operators.size();
                        temp.type = OPERATOR;
                        nodes.push_back(temp);
                        operators.push_back(operatorlist[i]);
                        eip++;
                        break;
                    }
                continue;
            }
            /*if (nodes.empty() || (nodes.back().type == PARENTHESIS && parentheses[nodes.back().ID] == true))
            {
                if ((*(eip+1)>='a' && *(eip+1)<='z')||(*(eip+1)>='A' && *(eip+1)<='Z')) addNode(VARIABLE, &eip);
                else if ((*(eip+1)>='0' && *(eip+1)<='9')|| *(eip+1) == '.') addNode(NUMBER, &eip);
            }
            else
            {
                if (nodes.back().type==NUMBER || nodes.back().type==VARIABLE) addNode(OPERATOR, &eip);
                else throw AmirException(AmirString("can not parse sorce in: ")<<eip);
            }
            continue;*/
        }
        if ((*eip>='a' && *eip<='z')||(*eip>='A' && *eip<='Z'))
        {
            addNode(VARIABLE, &eip);
            continue;
        }
        if ((*eip >= '0' && *eip <= '9') || *eip == '.')
        {
            addNode(NUMBER, &eip);
            continue;
        }
        if (*eip == ')' || *eip == '(')
        {
            addNode(PARENTHESIS, &eip);
            continue;
        }
        addNode(OPERATOR, &eip);
    }
}
//===============================================================================
double AmirParser::getResult()
{
    return double_result;
}
//===============================================================================
void AmirParser::addNode(NodeType type, char** eip)
{
    switch (type)
    {
    case NUMBER:
    {
        if (!nodes.empty() && (nodes.back().type == VARIABLE || nodes.back().type == NUMBER))
            throw AmirException(AmirString("Error reached NUMBER after NUMBER or VARIABLE at: ")<<*eip);
        bool isreached = false;
        double value = 0;
        double decimalpart = 1;
        while ((**eip >= '0'&&**eip <= '9') || **eip == '.')
        {
            if (isreached) decimalpart /= 10;
            if (**eip == '.')
            {
                if (isreached) throw AmirException(AmirString("find two . point at: ")<<*eip);
                else isreached = false;
            }
            else
            {
                if (isreached)
                {
                    value += (**eip - '0') * decimalpart;
                }
                else
                {
                    value = value * 10 + **eip - '0';
                }
            }
            (*eip)++;
        }
        Node temp;
        temp.ID = numbers.size();
        temp.type = NUMBER;
        numbers.push_back(value);
        nodes.push_back(temp);
        _AMIRLOG(AmirLib1LOG, _INFO, AmirString("number ")<<value<<" added to nodes.");
    }
    break;
    case OPERATOR:
    {
        if (nodes.empty() || nodes.back().type == OPERATOR || (nodes.back().type == PARENTHESIS && parentheses[nodes.back().ID]))
            throw AmirException(AmirString("Error reached OPERATOR after OPERATOR or '(' at: ")<<*eip);
        unsigned int i;
        for (i=0; i<operatorlist.size(); i++)
            if (**eip == operatorlist[i].letter) break;
        if (i >= operatorlist.size()) throw AmirException(AmirString("Error add this letter to parser before using: ")<<*eip);
        else
        {
            Node temp;
            temp.ID = operators.size();
            temp.type = OPERATOR;
            nodes.push_back(temp);
            operators.push_back(operatorlist[i]);
            _AMIRLOG(AmirLib1LOG, _INFO, AmirString("operator ")<<operatorlist[i].letter<<" added to nodes.");
        }
        (*eip)++;
    }
    break;
    case PARENTHESIS:
    {
        if (**eip=='(' && !nodes.empty() && (nodes.back().type==NUMBER || nodes.back().type==VARIABLE))
            throw AmirException(AmirString("Error reached '(' after number or variable at: ")<<*eip);
        if (**eip==')' && (nodes.empty() || nodes.back().type==OPERATOR)) throw AmirException(AmirString("Error reached ')' at: ")<<*eip);
        Node temp;
        temp.ID = parentheses.size();
        temp.type = PARENTHESIS;
        nodes.push_back(temp);
        if (**eip=='(') parentheses.push_back(true);
        else parentheses.push_back(false);
        _AMIRLOG(AmirLib1LOG, _INFO, AmirString("parentheses '")<<**eip<<"' added to nodes.");
        (*eip)++;
    }
    break;
    case VARIABLE:
    {
        if (!nodes.empty() && (nodes.back().type == VARIABLE || nodes.back().type == NUMBER))
            throw AmirException(AmirString("Error reached VARIABLE after NUMBER or VARIABLE at: ")<<*eip);
        char *ch = *eip;
        while ((*ch>='0' && *ch<='9')||(*ch>='a' && *ch<='z')||(*ch>='A' && *ch<='Z'))
        {
            ch++;
        }
        char tch = *ch;
        *ch = 0;
        string foundvariable(*eip);
        *ch = tch;
        unsigned int i;
        for(i=0; i<variables.size(); i++)
            if (variables[i].text == foundvariable) break;
        if (i >= variables.size()) throw AmirException(AmirString("Error variable not found at: ")<<*eip);
        else
        {
            Node temp;
            temp.ID = numbers.size();
            temp.type = NUMBER;
            nodes.push_back(temp);
            numbers.push_back(variables[i].value);
            _AMIRLOG(AmirLib1LOG, _INFO, AmirString("variable ")<<variables[i].text<<" added to nodes.");
        }
        *eip = ch;
    }
    break;
    default:
        break;
    }
}
//===============================================================================
void AmirParser::removeParentheses()
{
    int openCnt = 0;
    for (unsigned int i = 0; i<nodes.size(); i++)
    {
        if (nodes[i].type == PARENTHESIS)
        {
            nodes[i].type = NONE;
            if (parentheses[nodes[i].ID]) openCnt++;
            else openCnt--;
            if (openCnt<0) throw AmirException(AmirString("Error close parentheses should not be more than open parentheses"));

        }else if (nodes[i].type == OPERATOR) operators[nodes[i].ID].priority += 64*openCnt;
    }
    if (openCnt) throw AmirException("Error parentheses are not closed!");
    removeNoneNodes();
}
//===============================================================================
void AmirParser::addOperator(char letter, unsigned int priority, operatorfunction optfunc)
{
    if (priority>63) throw AmirException("Error operator priority can not be more than 63!");
    //checking for tekrari variables
    unsigned int z01;
    for (z01 = 0; z01 < operatorlist.size(); z01++)
        if (operatorlist[z01].letter == letter)
        {
            throw AmirException(AmirString("Error: redeclaration operator ")<<letter);
        }
    if (z01 >= operatorlist.size())
    {
        OptrFuncion temp;
        temp.letter = letter;
        temp.priority = priority;
        temp.func = optfunc;
        operatorlist.push_back(temp);
    }
}
//===============================================================================
void AmirParser::addVariable(AmirString text, double value)
{
    //checking for tekrari variables
    unsigned int z01;
    for (z01 = 0; z01 < variables.size(); z01++)
        if (variables[z01].text == text)
        {
            throw AmirException(AmirString("Error: redeclaration of ")<<text);
        }
    if (z01 >= variables.size())
    {
        Variable temp;
        temp.text = text;
        temp.value = value;
        variables.push_back(temp);
    }
}
//===============================================================================
AmirParser::~AmirParser()
{
    if (source)
    {
        delete [] source;
        source = nullptr;
    }
}
//===============================================================================
void AmirParser::removeNoneNodes()
{
    unsigned int i = 0;
    while (i<nodes.size())
        if (nodes[i].type == NONE) nodes.erase(nodes.begin()+i);
        else i++;
}
//===============================================================================
void AmirParser::removeVariable(AmirString text)
{
    unsigned int z01;
    for (z01 = 0; z01 < variables.size(); z01++)
        if (variables[z01].text == text)
        {
            variables.erase(variables.begin()+z01);
            _AMIRLOG(AmirLib1LOG, _INFO, AmirString("variables ")<<text<<" removed successfully.");
            break;
        }
}
//===============================================================================
void AmirParser::removeOperator(char letter)
{
    unsigned int z01;
    for (z01 = 0; z01 < operatorlist.size(); z01++)
        if (operatorlist[z01].letter == letter)
        {
            operatorlist.erase(operatorlist.begin()+z01);
            _AMIRLOG(AmirLib1LOG, _INFO, AmirString("operator ")<<letter<<" removed successfully.");
            break;
        }
}
//===============================================================================
double AmirParser::minusFunction(double a, double b)
{
    return a-b;
}
double AmirParser::addFunction(double a, double b)
{
    return a+b;
}
double AmirParser::zarbFunction(double a, double b)
{
    return a*b;
}
double AmirParser::taghsimFunction(double a, double b)
{
    return a/b;
}
double AmirParser::powFunction(double a, double b)
{
    return pow(a,b);
}
//===============================================================================
//===============================================================================
//===============================================================================
Rational::Rational(long long x,long long y)
{
    if (y == 0) throw overflow_error("Divide by zero exception");
    if (x == 0)
    {
        numerator = 0;
        denominator = 1;
    }
    else
    {
        if (x<0)
        {
            x = -x;
            y = -y;
        }
        long long gcdd = gcd(x,y);
        numerator = x /gcdd;
        denominator = y /gcdd;
    }
}
//===============================================================================
Rational::Rational(double x)
{
    *this = convertDoubleToRational(x);
}
//===============================================================================
Rational::Rational(long long x)
{
    numerator = x;
    denominator = 1;
}
//===============================================================================
void Rational::operator =(const Rational &r1)
{
    numerator = r1.numerator;
    denominator = r1.denominator;
}
//===============================================================================
Rational Rational::operator *(const Rational &r1)
{
    return Rational(r1.numerator*numerator, r1.denominator*denominator);
}
//===============================================================================
Rational Rational::operator /(const Rational &r1)
{
    return Rational(r1.denominator*numerator, r1.numerator*denominator);
}
//===============================================================================
Rational Rational::operator +(const Rational &r1)
{
    return Rational(r1.numerator*denominator+r1.denominator*numerator, r1.denominator*denominator);
}
//===============================================================================
Rational Rational::operator -(const Rational &r1)
{
    return Rational(r1.denominator*numerator-r1.numerator*denominator, r1.denominator*denominator);
}
//===============================================================================
void Rational::operator =(double d1)
{
    *this = convertDoubleToRational(d1);
}
//===============================================================================
Rational Rational::operator *(double d1)
{
    return *this * convertDoubleToRational(d1);
}
//===============================================================================
Rational Rational::operator /(double d1)
{
    return *this / convertDoubleToRational(d1);
}
//===============================================================================
Rational Rational::operator +(double d1)
{
    return *this + convertDoubleToRational(d1);
}
//===============================================================================
Rational Rational::operator -(double d1)
{
    return *this - convertDoubleToRational(d1);
}
//===============================================================================
void Rational::operator *=(const Rational &r1)
{
    this->numerator = r1.numerator*numerator;
    this->denominator = r1.denominator*denominator;
}
//===============================================================================
void Rational::operator /=(const Rational &r1)
{
    this->numerator = r1.numerator*denominator;
    this->denominator = r1.denominator*numerator;
}
//===============================================================================
void Rational::operator +=(const Rational &r1)
{
    this->numerator = r1.numerator*denominator+r1.denominator*numerator;
    this->denominator = r1.denominator*denominator;
}
//===============================================================================
void Rational::operator -=(const Rational &r1)
{
    this->numerator = r1.denominator*numerator-r1.numerator*denominator;
    this->denominator = r1.denominator*denominator;
}
//===============================================================================
void Rational::operator *=(double d1)
{
    *this *= convertDoubleToRational(d1);
}
//===============================================================================
void Rational::operator /=(double d1)
{
    *this /= convertDoubleToRational(d1);
}
//===============================================================================
void Rational::operator +=(double d1)
{
    *this += convertDoubleToRational(d1);
}
//===============================================================================
void Rational::operator -=(double d1)
{
    *this -= convertDoubleToRational(d1);
}
//===============================================================================
bool Rational::operator ==(const Rational &r1)
{
    if(r1.numerator*denominator == numerator*r1.denominator) return true;
    else return false;
}
//===============================================================================
string Rational::toString()
{
    stringstream ss;//ostringstream oss;
    ss<<"numerator="<<numerator<<"  denominator="<<denominator;//string result(oss.str());
    return ss.str();
}
//===============================================================================
unsigned long long Rational::gcd(unsigned long long m, long long n)
{
    if (n<0) n = -n;
    unsigned long long r;
    if (n>m)
    {
        r = m;
        m = n;
        n = r;
    }
    r = n;
    while (m % n != 0)
    {
        r = m % n;
        m = n;
        n = r;
    }
    return r;
}
//===============================================================================
Rational Rational::convertDoubleToRational(double d1)
{
    d1 += DBL_EPSILON*d1;
    long long numerator=d1, denominator=1;
    while((d1-numerator/(double)denominator>2*d1*DBL_EPSILON || d1-numerator/(double)denominator<-2*d1*DBL_EPSILON)
            &&(denominator<numeric_limits<long long>::max()/10)&&(numerator<numeric_limits<long long>::max()/10)&&
            (numerator>-numeric_limits<long long>::max()/10))
    {
        denominator *= 10;
        numerator = d1*denominator;
    }
    return Rational(numerator, denominator);
}




