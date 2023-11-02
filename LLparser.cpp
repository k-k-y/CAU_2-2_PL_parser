// ERROR1 : 연산자(사칙연산)가 연속해서 나오는 경우
// ERROR2 : 정의되지 않은 변수(IDENT)를 사용하는 경우
// ERROR3 : 괄호 쌍이 맞지 않는 경우
// ERROR4 : 피연산자(CONST, IDENT)가 연속해서 나오는 경우

#include <iostream>
#include <cctype>
#include <fstream>
#include <string>
#include <unordered_map>
#include <stack>
#include <queue>

#define NC "\e[0m"
#define RED "\e[0;31m"
#define PUP "\e[0;35m"
#define BLUE "\e[0;94m"
#define REDB "\e[41m"

using namespace std;

enum TokenType { IDENT, CONST, ASSIGNMENT_OP, ADD_OP, SUB_OP, MUL_OP, DIV_OP, LEFT_PAREN, RIGHT_PAREN, SEMICOLON, UNKNOWN, END_OF_FILE };
// string tokenArr[12] = {	"", "", ":=", "+", "-", "x", "/", "(", ")", ";", "", "" };

int next_token;
string token_string;

unordered_map<string, pair<bool, int>> SymbolTable; // name, {isInitial, value}
stack<pair<bool, int>> s; // 초기화 여부, 값
int idCnt = 0, constCnt = 0, opCnt = 0;

queue<pair<int, string>> errorQue; // errorCode, token_string

string input = "";
int pos = -1;
char cur_char;

void inputString(string);

// LexicalAnalyzer
void advance();
void getNonBlank();
string getConst();
string getIdent();
void lexical();

// SyntaxAnalyzer
void Statements(); // <statements> -> <statement> | <statement><semicolon><statements>
void Statement(); // <statement> -> <ident><assignment_op><expression>
void Expression(); // <expression> -> <term><term_tail>
void Term_tail(); // <term_tail> -> <add/sub><term><term_tail> | lambda
void Term(); // <term> -> <factor><factor_tail>
void Factor_tail(); // <factor_tail> -> <mult/div><factor><factor_tail> | lambda
void Factor(); // <factor> -> <left_paren><expression><right_paren> | <ident> | <const>


int main(int argc, char* argv[])
{
	if (argc > 1)
		inputString(argv[1]);

	Statements();

	return 0;
}

void inputString(string fileName)
{
	ifstream file(fileName);
	string temp;
	if (file.is_open())
		while (getline(file, temp))
			input += temp;
		
}

void advance()
{
	pos++;
	cur_char = (pos < input.size()) ? input[pos] : -1;
}
void getNonBlank()
{
	while (cur_char != -1 && cur_char <= 32)
		advance();
}
string getConst()
{
	string result = "";
	while (cur_char && isdigit(cur_char))
	{
		result += cur_char;
		advance();
	}
	return result;
}
string getIdent()
{
	string result = "";
	while (cur_char && (isalnum(cur_char) || cur_char == '_')) 
	{
		result += cur_char;
		advance();
	}
	return result;
}
void lexical()
{
	getNonBlank();
	
	if (cur_char == -1) 
	{
		next_token = END_OF_FILE;
		token_string = "";
	}
	else if (isdigit(cur_char))
	{
		next_token = CONST;
		token_string = getConst();
	}
	else if (isalpha(cur_char) || cur_char == '_')
	{
		next_token = IDENT;
		token_string = getIdent();
	}
	else if (cur_char == '+')
	{
		advance();
		next_token = ADD_OP;
		token_string = "+";
	}
	else if (cur_char == '-')
	{
		advance();
		next_token = SUB_OP;
		token_string = "-";
	}
	else if (cur_char == '*')
	{
		advance();
		next_token = MUL_OP;
		token_string = "*";
	}
	else if (cur_char == '/')
	{
		advance();
		next_token = DIV_OP;
		token_string = "/";
	}
	else if (cur_char == ':')
	{
		advance();
		if (cur_char == '=')
		{
			advance();
			next_token = ASSIGNMENT_OP;
			token_string = ":=";
		}
	}
	else if (cur_char == ';')
	{
		advance();
		next_token = SEMICOLON;
		token_string = ";";
	}
	else if (cur_char == '(')
	{
		advance();
		next_token = LEFT_PAREN;
		token_string = "(";
	}
	else if (cur_char == ')')
	{
		advance();
		next_token = RIGHT_PAREN;
		token_string = ")";
	}
	else
	{
		next_token = UNKNOWN;
		token_string = "";
	}

	if (next_token == ADD_OP || next_token == SUB_OP || next_token == MUL_OP || next_token == DIV_OP) // ERROR1 : 연속된 연산자 검사
	{
        while (1)
		{
            getNonBlank();
			
            if (cur_char == '+')
			{
				errorQue.push({1, "+"});
				advance();
			}
			else if (cur_char == '-')
			{
				errorQue.push({1, "-"});
				advance();
			}
			else if (cur_char == '*')
			{
				errorQue.push({1, "*"});
				advance();
			}
			else if (cur_char == '/')
			{
				errorQue.push({1, "/"});
				advance();
			}
			else
                break;
        }
	}

	// print lexeme
	if (next_token == SEMICOLON)
		cout << "\b" << token_string;
	else
		cout << token_string << " ";
	
}

void Statements()
{
	if (next_token == END_OF_FILE) return;

	lexical();
	Statement();

	cout << "\n";
	cout << BLUE "ID: " << idCnt << "; " << "CONST: " << constCnt << "; " << "OP: " << opCnt << ";\n" NC;
	if (errorQue.empty())
		cout << RED "(OK)\n" NC;
	while(!errorQue.empty()) // ERROR 출력
	{
		if (errorQue.front().first == 1)
		{
			cout << RED "(Warning) “중복 연산자(" << errorQue.front().second << ") 제거”\n" NC;
			errorQue.pop();
		}
		else if (errorQue.front().first == 2)
		{
			cout << RED "(Error) 정의되지 않은 변수(" << errorQue.front().second << ")가 참조됨\n" NC;
			errorQue.pop();
		}
	}

	idCnt = 0; constCnt = 0; opCnt = 0;

	if (next_token == SEMICOLON)
		Statements();
	
	cout << PUP "Result ==> ";
	for (auto it : SymbolTable)
	{
		if (it.second.first == true)
			cout << it.first << ": " << it.second.second << "; ";
		else
			cout << it.first << ": UNKNOWN; ";
	}
	cout << "\n" NC;

	exit(0);
}
void Statement()
{
	if (next_token == IDENT)
	{
		idCnt++;
		string name = token_string;
		lexical();
		if (next_token == ASSIGNMENT_OP)
		{
			lexical();
			Expression();
			if (!s.empty())
			{
				pair<bool, int> value = s.top(); s.pop();
				SymbolTable.insert(make_pair(name, value));
			}
		}
		else
			cout << "ERROR\n";
	}
	else
		cout << "ERROR\n";
}
void Expression()
{
	Term();
	Term_tail();
}
void Term_tail()
{
	pair<bool, int> operand1;
	pair<bool, int> operand2;
	if (next_token == ADD_OP)
	{
		opCnt++;
		lexical();
		Term();
		if(!s.empty())
		{
			operand2 = s.top(); s.pop();
			operand1 = s.top(); s.pop();
			s.push({operand1.first & operand2.first, operand1.second + operand2.second});
		}
		Term_tail();
	}
	else if (next_token == SUB_OP)
	{
		opCnt++;
		lexical();
		Term();
		if(!s.empty())
		{
			operand2 = s.top(); s.pop();
			operand1 = s.top(); s.pop();
			s.push({operand1.first & operand2.first, operand1.second - operand2.second});
		}
		Term_tail();
	}
	else
		return;
}
void Term()
{
	Factor();
	Factor_tail();
}
void Factor_tail()
{
	pair<bool, int> operand1;
	pair<bool, int> operand2;
	if (next_token == MUL_OP)
	{
		opCnt++;
		lexical();
		Factor();
		if(!s.empty())
		{
			operand1 = s.top(); s.pop();
			operand2 = s.top(); s.pop();
			s.push({operand1.first & operand2.first, operand1.second * operand2.second});
		}
		Factor_tail();
	}
	else if (next_token == DIV_OP)
	{
		opCnt++;
		lexical();
		Factor();
		if(!s.empty())
		{
			operand1 = s.top(); s.pop();
			operand2 = s.top(); s.pop();
			s.push({operand1.first & operand2.first, operand1.second / operand2.second});
		}
		Factor_tail();
	}
	else
		return;
}
void Factor()
{
	if (next_token == LEFT_PAREN)
	{
		lexical();
		Expression();
		if (next_token != RIGHT_PAREN) 
			cout << "ERROR\n";
		lexical();
	}
	else if (next_token == IDENT) 
	{
		idCnt++;
		if (SymbolTable.find(token_string) != SymbolTable.end())
			s.push(SymbolTable[token_string]);
		else
		{
			errorQue.push({2, token_string}); // ERROR2 : 정의하지 않은 변수 참조
			pair<bool, int> value = {false, 2147483647};
			SymbolTable.insert(make_pair(token_string, value));
			s.push(SymbolTable[token_string]);
		}
		lexical();
	}
	else if (next_token == CONST)
	{
		constCnt++;
		s.push({true, stoi(token_string)});
		lexical();
	}
	else
		cout << "ERROR\n";
}
