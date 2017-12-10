#ifndef WORDSCORE
#define WORDSCORE

#include <cstring>
#include <string>

using namespace std;

class WordScore
{
public:
	string word;
	int startx;
	int starty;
	int endx;
	int endy;
	int score;
	void clear();
	bool operator >= (WordScore);
	bool operator == (WordScore);
};

void WordScore::clear()
{
	word="";
	startx=NULL;
	starty=NULL;
	endx=NULL;
	endy=NULL;
	score=NULL;
}

bool WordScore::operator >= (WordScore wc2)
{
	if(score>=wc2.score)
		return true;
	else
		return false;
}

bool WordScore::operator == (WordScore wc2)
{
	if(score==wc2.score)
		return true;
	else
		return false;
}

#endif;