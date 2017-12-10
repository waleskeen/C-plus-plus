#ifndef BOARD//setting board
#define BOARD
#include <cstring>

using namespace std;

class Board
{
public:
	char *letter;
	int score;
	bool operator >= (Board);
	bool operator == (Board);
	Board();
};

Board::Board()
{
	letter=new (char[2]);
	score=-1;
}

bool Board::operator >= (Board B2)
{
	if(strcmp(letter,B2.letter)>=0)
		return true;
	else
		return false;
}

bool Board::operator == (Board B2)
{
	if(strcmp(letter,B2.letter)==0)
		return true;
	else
		return false;
}

#endif;