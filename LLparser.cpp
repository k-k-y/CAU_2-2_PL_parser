#include <iostream>
#include <cctype>
#include <fstream>
#include <string>
#include <unordered_map>
#include <stack>

using namespace std;

enum TokenType { IDENT, CONST, ASSIGNMENT_OP, ADD_OP, SUB_OP, MUL_OP, DIV_OP, LEFT_PAREN, RIGHT_PAREN, SEMICOLON, UNKNOWN };

int next_token;
string token_string;

unordered_map<string, int> SymbolTable; // name, value
stack<int> s;

string input;
int pos = -1;
char cur_char;

// LexicalAnalyzer
void advance();
void getNonBlank();
string getConst();
string getIdent();
void lexical();

// SyntaxAnalyzer
void Statement(); // <statement> -> <ident><assignment_op><expression>
void Expression(); // <expression> -> <term><term_tail>
void Term_tail(); // <term_tail> -> <add/sub><term><term_tail> | lambda
void Term(); // <term> -> <factor><factor_tail>
void Factor_tail(); // <factor_tail> -> <mult/div><factor><factor_tail> | lambda
void Factor(); // <factor> -> <left_paren><expression><right_paren> | <ident> | <const>


int main(void)
{
	input = "asd := (1+2*3)-2;";

	lexical();
	Statement();

	for (pair<string, int> elem : SymbolTable)
		cout << elem.first << " " << elem.second << "\n";

	return 0;
}

void print_stack()
{
	stack<int> temp = s;
	while (temp.size() > 0)
	{
		cout << temp.top() << " ";
		temp.pop();
	}
	cout << "\n";
}

void advance()
{
	pos++;
	cur_char = (pos < input.size()) ? input[pos] : '\0';
}
void getNonBlank()
{
	while (cur_char <= 32)
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
	while (cur_char && (isalnum(cur_char) || cur_char == '_')) // 문자, 숫자, _
	{
		result += cur_char;
		advance();
	}
	return result;
}
void lexical()
{
	getNonBlank();

	if (!cur_char) // 끝
	{
		next_token = EOF;
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

	cout <<next_token << token_string << "\n";
}

void Statement()
{
	if (next_token == IDENT)
	{
		string name = token_string;
		lexical();
		if (next_token == ASSIGNMENT_OP)
		{
			lexical();
			Expression();
			int value = s.top(); s.pop();
			SymbolTable.insert({ name, value });
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
	int operand1, operand2;
	if (next_token == ADD_OP)
	{
		lexical();
		Term();

		operand2 = s.top(); s.pop();
		operand1 = s.top(); s.pop();
		s.push(operand1 + operand2);

		Term_tail();
	}
	else if (next_token == SUB_OP)
	{
		lexical();
		Term();

		operand2 = s.top(); s.pop();
		operand1 = s.top(); s.pop();
		s.push(operand1 - operand2);

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
	int operand1, operand2;
	if (next_token == MUL_OP)
	{
		lexical();
		Factor();

		operand1 = s.top(); s.pop();
		operand2 = s.top(); s.pop();
		s.push(operand1 * operand2);

		Factor_tail();
	}
	else if (next_token == DIV_OP)
	{
		lexical();
		Factor();

		operand1 = s.top(); s.pop();
		operand2 = s.top(); s.pop();
		s.push(operand1 / operand2);

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
		if (next_token != RIGHT_PAREN) // 괄호쌍 판단
			cout << "ERROR\n";
		lexical();
	}
	else if (next_token == IDENT && SymbolTable.find(token_string) != SymbolTable.end()) // IDENT 정의 여부 판단
	{
		s.push(SymbolTable[token_string]);
		lexical();
	}
	else if (next_token == CONST)
	{
		s.push(stoi(token_string));
		lexical();
	}
	else
		cout << "ERROR\n";
}