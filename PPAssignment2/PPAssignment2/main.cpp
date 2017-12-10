#include <iostream>
#include <omp.h>
#include <ctime>
#include <fstream>
#include <string>
#include "board.h"
#include <cstring>
#include <string>
#include <list>
#include <Windows.h>
#include <algorithm>

using namespace std;

//typedef unsigned long long int UINT64;
void jokerGenerate(string onhand,int joker, list<string> allWord);
boolean firstGame(list<string> allWord, int numberJoker, string onhand);
CRITICAL_SECTION cs; 

Board board[22][22];

//UINT64 NumOfValidWord;
//UINT64 NumOfPosibleWord;
//list<const char*> validWord;//list of valid word
//list<string> allWord;//list of all word
int numCPU;
char tileOnHand[7];
int gamePlay=0;

int numJoker=0;//check joker
//char lettersToPlace[15];
//int numberOfLetters=0;
int BestScore=0,PlayerScore=0;//Bonus=0,tileUsed=0; //bonus only use when all 7 letter used.
string BestWord="";
//int BestWordPosition[4]={0,0,0,0};  //start x, start y, end x, end y

int sx,sy,ex,ey;//same as BestWordPosition

int bfirstx,bfirsty,blastx,blasty;

string jokerword;
string bestjokerword;

class Edge;

class Word{
public:
	Word() {
		for(int i=0;i<26;i++){ 
			edge[i] = NULL; 
			is_word = false;
		}
	}
	~Word() {
		for(int i=0;i<26;i++) {
			if(edge[i]) 
				delete edge[i];
		}
	}
	string word;
	bool is_word; //if the word is valid gv it a true value
	Edge* edge[26]; //the index corresponds to the character in alphabetic order
};
class Edge{
public:
	Edge() {word=NULL;}
	~Edge() {if(word) delete word;}
	Word* word;
};

class Dictionary{
public:
	Dictionary();
	~Dictionary();
	void AddWord(char* w);
	int Search2(char* w, list<const char*> &validWord);
	int checkValid(char* w);
private:
	Word* Root; //stores the pointer to Root Word
};

Dictionary::Dictionary(){
	Root = new Word();
}
Dictionary::~Dictionary(){
	delete Root;
}

// adds a word w to the dicitionary  
void Dictionary::AddWord(char* w){
	Word* node = Root;

	for(int i=0; w[i] != '\0'; i++){
		if(node->edge[w[i]-'A']){
			node = node->edge[w[i]-'A']->word;
			if(w[i+1] == '\0'){
				node->word = w;
			}
		}else{
			node->edge[w[i]-'A'] = new Edge();
			node = node->edge[w[i]-'A']->word = new Word();
			node->word.assign(w, i+1);
			if(w[i+1] == '\0'){ //if end of w, mark the node
				node->is_word=true;
			}}}
}

//search and display all possible word in there MODIFIED FUNCTION WITHOUT JOKER (COMPLETE)
//here some part i change to parallel
int Dictionary::Search2(char* w, list<const char*> &validWord){
	//traverse the tree and find the match
	Word* node = Root;
	volatile bool flag=true;
	const char* tmp;
	//string tmpW;
	//string tmpL=w;

	for(int i=0; w[i] != '\0'; i++){
		if(w[i]=='*')//wales
		{
			for(int dw=0;dw<26;dw++)
			{
				char letter=65+dw;
				//if(tmpW.size()>0)
					//copy(tmpW.begin(),tmpW.end(),w);
				w[i]=letter;
				Search2(w, validWord);
				w[i]='*';
			}
			return 0;
		}//wales
	}

	for(int i=0; w[i] != '\0'; i++){

		/*if(w[i]=='*')//wales
		{
			for(int dw=0;dw<26;dw++)
			{
				char letter=65+dw;				
				w[i]=letter;
				Search2(w);
			}
		}//wales*/

		if(node->edge[w[i]-'A']){
			node = node->edge[w[i]-'A']->word;
			if(node->is_word){ //Check if the word is valid
				if(validWord.size()>0){ //if the validWord list is not empty

					for(list<const char*>::iterator it=validWord.begin();it!=validWord.end();++it){
						char* tmpV;
						tmpV=(char*)*it;
						if(strcmp(tmpV,node->word.c_str()) == 0){ //if same word match. jump out the loop
							flag=false; //found same word, set flag.
							break;
						}
						//delete[] tmpV;
					}

					/*list<const char*>::iterator itV=validWord.begin();
					#pragma omp parallel//here, but check if slower maybe change to sequencial
					for(int i2=0+omp_get_thread_num();i2<NumOfValidWord;i2=i2+numCPU)
					{
						if(!flag)
						{
							break;
						}
						const char* tmpV=new const char();
						#pragma omp critical (validspace)
						{
							tmpV=*itV;
							itV++;
						}
						if(strcmp(tmpV,node->word.c_str()) == 0){ //if same word match. jump out the loop
							flag=false; //found same word, set flag.
							break;
						}
					}*/

					if(flag){ //if no matches word in the validWord list, insert a new 1.
						//#pragma omp critical (x1space)
						{
							validWord.push_back(node->word.c_str()); //insert.
							//NumOfValidWord++;
							//cout<< node->word<<", ";
						}
					}
					flag=true; //reset flag.
				}
				else { //if it's empty
					//#pragma omp critical (x1space)
					{
						validWord.push_back(node->word.c_str());
						//NumOfValidWord++;
						//cout << node->word <<", ";
					}
				}
			}
			if(w[i+1]=='\0') return 0;
		} else break;
	}
	//cout << w << " not found!";
	return -1;
}

//search and display all possible word in there MODIFIED FUNCTION INCLUDE JOKER (INCOMPLETE)
int Dictionary::checkValid(char* w){
	//traverse the tree and find the match
	Word* node = Root;
	Word* tmpNode;
	for(int i=0; w[i] != '\0'; i++){

		if(node->edge[w[i]-'A']){
			node = node->edge[w[i]-'A']->word;
			if(w[i+1]=='\0' && node->is_word)  //if the string completely compare, then return 0.
				return 0;
		}
		else break;
	}
	//cout << w << " not found!\n";
	return -1;
}

Dictionary MDict;

void setBoard()//setting the board
{
	for(int a=0;a<22;a++)
	{
		for(int b=0;b<22;b++)
		{
			if(a==0)//labeling
			{
				char *tmp=new (char[3]);
				tmp=itoa(b,tmp,10);
				for(int i1=0;i1<3;i1++)
					board[0][b].letter[i1]=tmp[i1];
				delete[] tmp;
			}
			else if(b==0)//labeling
			{
				board[a][0].letter[0]=96+a;
				board[a][0].letter[1]=NULL;
			}
			else if((a==1&&b==4) || (a==1&&b==11) || (a==1&&b==18) ||
				(a==4&&b==1) || (a==4&&b==7) || (a==4&&b==15) || (a==4&&b==21) ||
				(a==6&&b==10) || (a==6&&b==12) || (a==7&&b==4) || (a==7&&b==11) || (a==7&&b==18) || 
				(a==10&&b==6) || (a==10&&b==10) || (a==10&&b==12) || (a==10&&b==16) ||
				(a==11&&b==1) || (a==11&&b==7) || (a==11&&b==15) || (a==11&&b==21) ||
				(a==12&&b==6) || (a==12&&b==10) || (a==12&&b==12) || (a==12&&b==16) ||
				(a==15&&b==4) || (a==15&&b==11) || (a==15&&b==18) || (a==16&&b==10) || (a==16&&b==12) ||
				(a==18&&b==1) || (a==18&&b==7) || (a==18&&b==15) || (a==18&&b==21) ||
				(a==21&&b==4) || (a==21&&b==11) || (a==21&&b==18))//set 2L bonus
			{
				board[a][b].letter="!";
			}
			else if((a==2&&b==2) || (a==2&&b==9) || (a==2&&b==13) || (a==2&&b==20) ||
				(a==3&&b==3) || (a==3&&b==10) || (a==3&&b==12) || (a==3&&b==19) ||
				(a==5&&b==5) || (a==5&&b==17) || (a==6&&b==6) || (a==6&&b==16) ||
				(a==7&&b==7) || (a==7&&b==15) || (a==8&&b==8) || (a==8&&b==14) ||
				(a==9&&b==2) || (a==9&&b==20) || (a==10&&b==3) || (a==10&&b==19) ||
				(a==12&&b==3) || (a==12&&b==19) || (a==13&&b==2) || (a==13&&b==20) ||
				(a==14&&b==8) || (a==14&&b==14) || (a==15&&b==7) || (a==15&&b==15) ||
				(a==16&&b==6) || (a==16&&b==16) || (a==17&&b==5) || (a==17&&b==17) ||
				(a==19&&b==3) || (a==19&&b==10) || (a==19&&b==12) || (a==19&&b==19) ||
				(a==20&&b==2) || (a==20&&b==9) || (a==20&&b==13) || (a==20&&b==20) ||
				(a==11&&b==11))//set 2W bonus
			{
				board[a][b].letter="$";
			}
			else if((a==2&&b==5) || (a==2&&b==17) || 
				(a==5&&b==2) || (a==5&&b==9) || (a==5&&b==13) || (a==5&&b==20) || 
				(a==9&&b==5) || (a==9&&b==9) || (a==9&&b==13) || (a==9&&b==17) || 
				(a==13&&b==5) || (a==13&&b==9) || (a==13&&b==13) || (a==13&&b==17) || 
				(a==17&&b==2) || (a==17&&b==9) || (a==17&&b==13) || (a==17&&b==20) || 
				(a==20&&b==5) || (a==20&&b==17))//set 3L bonus
			{
				board[a][b].letter="@";
			}
			else if((a==1&&b==8) || (a==1&&b==14) || (a==4&&b==4) || (a==4&&b==11) || (a==4&&b==18) || 
				(a==8&&b==1) || (a==8&&b==21) || (a==11&&b==4) || (a==11&&b==18) || (a==14&&b==1) || (a==14&&b==21) || 
				(a==18&&b==4) || (a==18&&b==11) || (a==18&&b==18) || (a==21&&b==8) || (a==21&&b==14))//set 3W bonus
			{
				board[a][b].letter="%";
			}
			else if((a==3&&b==6) || (a==3&&b==16) || (a==6&&b==3) || (a==6&&b==19) ||
				(a==16&&b==3) || (a==16&&b==19) || (a==19&&b==6) || (a==19&&b==16))//set 4L bonus
			{
				board[a][b].letter="#";
			}
			else if((a==1&&b==1) || (a==1&&b==21) || (a==21&&b==1) || (a==21&&b==21))//set 4W bonus
			{
				board[a][b].letter="^";
			}
			else
				board[a][b].letter="-";
		}
	}
	board[0][0].letter=" ";
	// ! = Double Letter, @ = Triple Letter, # = Four letter, $ = Double Word, % = Triple Word, ^ = Four word
}

void displayBoard()
{	
	for(int a=0;a<22;a++)
	{
		for(int b=0;b<22;b++)
		{
			cout<<board[a][b].letter;
			if(a==0 || b<10)
				cout<<" ";
			else
				cout<<"  ";
		}
		cout<<endl;
	}
}

DWORD WINAPI readFile(LPVOID lpParam){//here i sponse 1 more thread to readfile
	clock_t begin, end;

	ifstream file("dictionary.txt"); //read file

	string temp_str;
	int counter = 0;
	char* writable;

	begin = clock();
	while (!file.eof())
	{
		file >> temp_str;
		//cast the string into char* type(enhance later)
		writable = new char[temp_str.size() + 1];
		copy(temp_str.begin(), temp_str.end(), writable);
		writable[temp_str.size()] = '\0';

		MDict.AddWord(writable); //add the word into trie
		delete[] writable; //free the writable(for next loop use)
		counter++;
	}
	end = clock();
	//cout<<"Time Taken: "<<(double)(end - begin) / CLOCKS_PER_SEC<<endl<<"Total word: "<<counter<<endl;

	return 0;
}

//Perform permutation and store into an array list, already sorted
void print_all_permutations(const string& s, list<string> &allWord)
{
	clock_t begin, end;
	string s1 = s;
	//replace(s1.begin(), s1.end(), '*', 'Z'); //joker not yet settle. replace with 'Z' first
	sort(s1.begin(), s1.end());

	do {begin =clock();
		//#pragma omp critical (perspace)
		{
			//EnterCriticalSection(&cs);
			allWord.push_back(s1);
			//NumOfPosibleWord++;
			//LeaveCriticalSection(&cs);
		}
	} while (next_permutation(s1.begin(), s1.end()));
	end=clock();
	//cout<<NumOfPosibleWord<<' '<<(double)(end - begin) / CLOCKS_PER_SEC<<endl;
}

/*DWORD WINAPI renew(LPVOID LpParam){
	//remove the content inside validWord & possibleWord after 1 game.
	#pragma omp parallel sections
	{
		#pragma omp section
		{
			validWord.clear();
		}
		#pragma omp section
		{
			allWord.clear();
		}
	}
	//NumOfValidWord=0;
	//NumOfPosibleWord=0;
	return 0;
}*/

int letterScore(char L){
	switch(L){
	case 'A': case 'E':
	case 'I': case 'O':
	case 'N': case 'R':
	case 'T': case 'L':
	case 'S': case 'U':
		return 1;
		break;

	case 'D': case 'G':
		return 2;
		break;

	case 'B': case 'C':
	case 'M': case 'P':
		return 3;
		break;

	case 'F': case 'H':
	case 'V': case 'W':
	case 'Y':
		return 4;
		break;

	case 'K':
		return 5;
		break;

	case 'J': case 'X':
		return 8;
		break;

	case 'Q': case 'Z':
		return 10;
		break;

	default:
		return 0;
		break;
	}
}

int calculationH(string word,int startx,int starty,int endx,int endy, char lettersToPlace[15])//calculate
{
	jokerword="???????????????";
	for(int a=0;a<word.size();a++)
	{
		if(!isalpha(word.at(a)))return 0;
	}
	/*if(numJoker>=1)//wales//check joker
	{
		string tmp1=word;
		string tmp2;
		int cnt=0;
		int tmpJ=numJoker;

		if(gamePlay==0)
			tmp2=tileOnHand;
		else
			tmp2=lettersToPlace;

		while(tmp1.size()>0)
		{
			for(int b=0;b<tmp2.size();b++)
			{
				//if(!isalpha(tmp1.at(0)))return 0;
				if(tmp1.at(0)==tmp2.at(b))
				{
					tmp1.erase(0,1);
					tmp2.erase(b,1);
					cnt++;
					break;
				}
				word.replace(cnt,1,"*");
				tmp1.erase(0,1);
				tmpJ--;
				cnt++;
			}
			if(tmpJ==0)
				break;
		}

	}//wales

	else
	{
		//for(int a=0;a<word.size();a++)
			//if(!isalpha(word.at(0)))return 0;
	}*/

	if(numJoker>=1 && lettersToPlace!="????????")//wales
    {
        string tmp1=word;
		string tmp2=lettersToPlace;
        int cnt=0;
        while(tmp1.size()>numJoker)
        {
            for(int a=0;a<tmp1.size();a++)
            {
                if(tmp2.at(cnt)=='*')//if the position of the word is '*'
                {
                    cnt++;
                    break;
                }
                else if(tmp2.at(cnt)==tmp1.at(a))//if the position of the word and ans is same
                {
                    tmp2.erase(cnt,1);
                    tmp1.erase(a,1);
                    break;
                }
                else if(a==tmp1.size()-1)
                    cnt++;
            }
        }
        for(int a=0;a<tmp1.size();a++)
            replace(word.begin(), word.end(), tmp1.at(a), '*');
		#pragma omp critical(jokerspace)
		{
			jokerword=word;
		}
    }//wales

	// ! = Double Letter, @ = Triple Letter, # = Four letter, $ = Double Word, % = Triple Word, ^ = Four word
	int score=0;
	for(int posiW=0;posiW<word.length();posiW++)
	{
		int x=startx;
		int y=starty;
		if(startx-endx==0)
		{
			y+=posiW;
		}
		else if(starty-endy==0)
		{
			x+=posiW;
		}
		if(board[x][y].letter[0]==word.at(posiW))
			score+=board[x][y].score;
		else if(board[x][y].letter[0]=='!')
			score+=letterScore(word.at(posiW))*2;
		else if(board[x][y].letter[0]=='@')
			score+=letterScore(word.at(posiW))*3;
		else if(board[x][y].letter[0]=='#')
			score+=letterScore(word.at(posiW))*4;
		else
			score+=letterScore(word.at(posiW));
	}
	for(int posiW=0;posiW<word.length();posiW++)
	{		
		int x=startx;
		int y=starty;
		if(startx-endx==0)
			y+=posiW;
		else if(starty-endy==0)
			x+=posiW;
		if(board[x][y].letter[0]=='$')
			score=score*2;
		else if(board[x][y].letter[0]=='%')
			score=score*3;
		else if(board[x][y].letter[0]=='^')
			score=score*4;
	}
	if(word.size()>6 && lettersToPlace!="????????")
	{
		if(gamePlay==0)
			score+=50;
		else
		{
			string tmp1=lettersToPlace;
			string tmp2=word;
			int cnt=0;
			boolean flag=true;
			while(tmp2.size()>0 && flag)
			{
				for(int a=0;a<tmp1.size();a++)
				{
					if(tmp1.at(a)==tmp2.at(cnt))
					{
						tmp2.erase(cnt,1);
						tmp1.erase(a,1);
						a=0;
						break;
					}
					if(a==tmp1.size()-1)
						flag=false;
				}
			}
			if(tmp1.size()==0)
				score+=50;
		}
	}
	return score;
} 

int calculateScore_firstRound(const string& word){ //only for the first round
	int wordScore=0;
	int starty;
	int endy;

	for(int a=0;a<word.size();a++)//arrange the placement in middle
	{
		starty=11-a;
		endy=starty+word.size()-1;
		wordScore=calculationH(word,11,starty,11,endy, tileOnHand);
		if(BestScore<wordScore)
		{
			BestWord=word;
			sx=11;
			sy=starty;
			ex=11;
			ey=endy;
			BestScore=wordScore;
			bestjokerword=jokerword;
		}
	}

	return wordScore;
}

void jokerGenerate(string onhand,int joker, list<string> allWord)
{
	for(int a=0;a<7;a++)
	{
		if(onhand.at(a)=='*')
		{			
			//#pragma omp parallel
			{
				//#pragma omp for
				for(int dw=0;dw<26;dw++)
				{
					int tmpJ=joker;
					string tmpH=onhand;
					char *letter=new (char[2]);
					letter[0]=65+dw;
					letter[1]=NULL;
					tmpH.replace(a,1,letter);
					tmpJ--;
					if(tmpJ>0)
						jokerGenerate(tmpH,tmpJ, allWord);
					else
					{
						firstGame(allWord, tmpJ, onhand);
					}
					//delete[] letter;
				}
			}
		}
	}
}

/*boolean firstGameJ(list<string> allWord, int numberJoker, string onhand){
	list<const char*> validWord;
	//list<string> allWord;
	//if(numberJoker>0)
	{
		//InitializeCriticalSectionAndSpinCount(&cs,0);
		//jokerGenerate(onhand,numberJoker, allWord);
		//DeleteCriticalSection(&cs);
		//return true;
	}
	//else
		print_all_permutations(onhand, allWord);

	list<string>::iterator itA=allWord.begin();
	for(int x=0;x<allWord.size();x++){ //according to the word in the list to compare all permutated word	
		string tmpSt;
		char* writable2=new char();
		//#pragma omp critical (xspace)
		{
			tmpSt=*itA;
			itA++;
		}
		writable2 = new char[tmpSt.size() + 1];
		copy(tmpSt.begin(), tmpSt.end(), writable2);
		writable2[tmpSt.size()] = '\0';
		MDict.Search2(writable2, validWord); //search the word
		delete[] writable2; //free up variable for next loop use
	}

	//find the longest or highest score word from validWord list
	BestScore=0;
	list<const char*>::iterator itV=validWord.begin();
	//#pragma omp parallel for
	for(int x=0;x<validWord.size();x++){
		const char* tmpV;
		//#pragma omp critical (yspace)
		{
			tmpV=*itV;
			itV++;
		}
		calculateScore_firstRound(tmpV);
	}
	//now we have the highest score and position, place the word into board
	int place=0;
	for(int x = BestWordPosition[0] ; x < (BestWordPosition[0]+BestWord.length()) ; x++){
		board[11][x].letter=new (char[2]);
		board[11][x].letter[0]=BestWord[place];
		board[11][x].letter[1]=NULL;
	} 
	if(Bonus==0)
		PlayerScore+=(BestScore*2); //first round earn another double score.
	else{
		PlayerScore+=((BestScore-50)*2);
		PlayerScore+=50;
	}

	for(int a=0;a<BestWord.length();a++)
	{
		board[11][sy+a].letter=new (char[2]);
		board[11][sy+a].letter[0]=BestWord.at(a);
		board[11][sy+a].letter[1]=NULL;
		if(bestjokerword.at(a)=='*')
			board[11][sy+a].score=0;
		else
			board[11][sy+a].score=letterScore(BestWord.at(a));
	}
	bfirsty=sy;
	blasty=sy+BestWord.length();
	bfirstx=11;
	blastx=11;
	PlayerScore+=BestScore;
	//#pragma omp parallel sections
	{
		//#pragma omp section
		{
			validWord.clear();
		}
		//#pragma omp section
		{
			allWord.clear();
		}
	}
	return true;
}*/

boolean firstGame(list<string> allWord, int numberJoker, string onhand){
	list<const char*> validWord;
	//list<string> allWord;
	//if(numberJoker>0)
	{
		//InitializeCriticalSectionAndSpinCount(&cs,0);
		//jokerGenerate(onhand,numberJoker, allWord);
		//DeleteCriticalSection(&cs);
		//return true;
	}
	//else
		print_all_permutations(onhand, allWord);

	/*if(numberJoker>0)
	{
		firstGameJ(allWord,numberJoker,onhand);
		return true;
	}*/

	for(list<string>::iterator itA=allWord.begin();itA!=allWord.end();++itA){ //according to the word in the list to compare all permutated word	
		string tmpSt;
		char* writable2;
		//#pragma omp critical (xspace)
		{
			tmpSt=*itA;
		}
		writable2 = new char[tmpSt.size() + 1];
		copy(tmpSt.begin(), tmpSt.end(), writable2);
		writable2[tmpSt.size()] = '\0';
		MDict.Search2(writable2, validWord); //search the word
		delete[] writable2; //free up variable for next loop use
	}

	//find the longest or highest score word from validWord list
	BestScore=0;
	//#pragma omp parallel for
	for(list<const char*>::iterator itV=validWord.begin();itV!=validWord.end();++itV){
		const char* tmpV;
		//#pragma omp critical (yspace)
		{
			tmpV=*itV;
		}
		calculateScore_firstRound(tmpV);
	}
	/*//now we have the highest score and position, place the word into board
	int place=0;
	for(int x = BestWordPosition[0] ; x < (BestWordPosition[0]+BestWord.length()) ; x++){
		board[11][x].letter=new (char[2]);
		board[11][x].letter[0]=BestWord[place];
		board[11][x].letter[1]=NULL;
	} 
	if(Bonus==0)
		PlayerScore+=(BestScore*2); //first round earn another double score.
	else{
		PlayerScore+=((BestScore-50)*2);
		PlayerScore+=50;
	}*/
	for(int a=0;a<BestWord.length();a++)
	{
		board[11][sy+a].letter=new (char[2]);
		board[11][sy+a].letter[0]=BestWord.at(a);
		board[11][sy+a].letter[1]=NULL;
		if(bestjokerword.at(a)=='*')
			board[11][sy+a].score=0;
		else
			board[11][sy+a].score=letterScore(BestWord.at(a));
	}
	bfirsty=sy;
	blasty=sy+BestWord.length();
	bfirstx=11;
	blastx=11;
	PlayerScore+=BestScore;
	//#pragma omp parallel sections
	{
		//#pragma omp section
		{
			validWord.clear();
		}
		//#pragma omp section
		{
			allWord.clear();
		}
	}
	return true;
}

void assignNewLetters(char lettersToPlace[15], int &numberOfLetters){
	for(int x=0;x<7;x++){ //copy the tile
		lettersToPlace[x]=tileOnHand[x];
		numberOfLetters++;
	}
}
void removeOldLetters(char lettersToPlace[15], int &numberOfLetters){
	for(int x=0;x<15;x++) //remove the tile
		lettersToPlace[x]=NULL;
	numberOfLetters=0;
}

void verticalPosition(int a, int b)//vertical part
{
	list<const char*> validWord;
	list<string> allWord;
	char letterOB;
	char lettersToPlace[15];
	int numberOfLetters=0;

	//HANDLE h1=CreateThread(NULL,0,renew,NULL,0,NULL);
	removeOldLetters(lettersToPlace, numberOfLetters);
	assignNewLetters(lettersToPlace, numberOfLetters);

	letterOB=board[a][b].letter[0];

	lettersToPlace[numberOfLetters]=letterOB;
	//WaitForSingleObject(h1,INFINITE);
	print_all_permutations(lettersToPlace, allWord);

	//#pragma omp parallel for
	for(list<string>::iterator itA=allWord.begin();itA!=allWord.end();++itA){ //according to the word in the list to compare all permutated word	
		string tmpSt;
		char* writable2;
		//#pragma omp critical (xspace)
		{
			tmpSt=*itA;;
		}
		writable2 = new char[tmpSt.size() + 1];
		copy(tmpSt.begin(), tmpSt.end(), writable2);
		writable2[tmpSt.size()] = '\0';
		MDict.Search2(writable2, validWord); //search the word
		delete[] writable2; //free up variable for next loop use
	}

	//#pragma omp parallel for
	for(list<const char*>::iterator itV=validWord.begin();itV!=validWord.end();++itV){
		string tmpV;
		//#pragma omp critical (yspace)
		{
			tmpV=(string)*itV;
		}

		int cnt1=count(tmpV.begin(),tmpV.end(),letterOB);

		if(cnt1==0)//if the key word is not inside the board
		{
			continue;
		}

		else if(cnt1==1)//if only 1 key word is not inside the board
		{
			int startx;
			startx=a-tmpV.find(letterOB);

			volatile boolean VFlag=true;
			
			for(int ve=startx-1;ve<startx+tmpV.size()+1;ve++)//check the placement
			{
				if(ve<0 || ve>22)//if out of bound
				{
					VFlag=false;
					break;
				}
				if(ve>0 && ve<22)
				{
					if(isalpha(board[ve][b].letter[0]))
					{
						if(ve==a)//if the word is same as keyword
							continue;
						VFlag=false;
						break;
					}
					else 
					{
						/*if(b-1<1 || b+1>21)//if out of bound
						{
							VFlag=false;
							break;
						}*/
						if(b-1>0 && b+1<22)
						{
							if(isalpha(board[ve][b-1].letter[0]) || isalpha(board[ve][b+1].letter[0]))//if beside the placement got word
							{
								int cntL=b;
								int cntR=b;

								//while(!isalpha(isalpha(board[ve][cntL].letter[0])))
								{

								}

								VFlag=false;
								break;
							}
						}
					}
				}
			}
			
			if(VFlag && startx>0 && startx+tmpV.size()-1<22)
			{
				int score=calculationH(tmpV,startx,b,startx+tmpV.size()-1,b,lettersToPlace);
				#pragma omp critical(bestspace)
				{
					if(BestScore<score)
					{
						BestWord=tmpV;
						sx=startx;
						sy=b;
						ex=startx+tmpV.size();
						ey=b;
						BestScore=score;
						bestjokerword=jokerword;
					}
				}
			}
		}

		else if(cnt1>1)//if the key word is same as the letter in the tileOnHand
		{
			for(int i1=0;i1<cnt1;i1++)
			{
				if(tmpV.at(i1)==letterOB)
				{
					int startx;
					startx=a-i1;

					volatile boolean VFlag=true;

					for(int ve=startx-1;ve<startx+tmpV.size()+1;ve++)
					{
						if(ve<0 || ve>22)//if out of bound
						{
							VFlag=false;
							break;
						}
						if(ve>0 && ve<22)
						{
							if(isalpha(board[ve][b].letter[0]))
							{
								if(ve==a)
									continue;
								VFlag=false;
								break;
							}
							else 
							{
								/*if(b-1<1 || b+1>21)
								{
									VFlag=false;
									break;
								}*/
								if(b-1>0 && b+1<22)
								{
									if(isalpha(board[ve][b-1].letter[0]) || isalpha(board[ve][b+1].letter[0]))
									{
										VFlag=false;
										break;
									}
								}
							}
						}
					}

					if(VFlag && startx>0 && startx+tmpV.size()-1<22)
					{
						int score=calculationH(tmpV,startx,b,startx+tmpV.size()-1,b,lettersToPlace);
						#pragma omp critical (bestspace)
						{
							if(BestScore<score)
							{
								BestWord=tmpV;
								sx=startx;
								sy=b;
								ex=startx+tmpV.size();
								ey=b;
								BestScore=score;
								bestjokerword=jokerword;
							}
						}
					}
				}
			}
		}
	}
	//#pragma omp parallel sections
	{
		//#pragma omp section
		{
			//validWord.clear();
		}
		//#pragma omp section
		{
			//allWord.clear();
		}
	}
}

void horizontalPosition(int a, int b)//horizontal
{
	list<const char*> validWord;
	list<string> allWord;
	char letterOB;
	char lettersToPlace[15];
	int numberOfLetters=0;

	//HANDLE h1=CreateThread(NULL,0,renew,NULL,0,NULL);
	//#pragma omp parallel sections
	{
		//#pragma omp section
		{
			//validWord.clear();
		}
		//#pragma omp section
		{
			//allWord.clear();
		}
	}
	removeOldLetters(lettersToPlace, numberOfLetters);
	assignNewLetters(lettersToPlace, numberOfLetters);

	letterOB=board[a][b].letter[0];

	lettersToPlace[numberOfLetters]=letterOB;
	//WaitForSingleObject(h1,INFINITE);
	print_all_permutations(lettersToPlace, allWord);

	//#pragma omp parallel for
	for(list<string>::iterator itA=allWord.begin();itA!=allWord.end();++itA){ //according to the word in the list to compare all permutated word	
		string tmpSt;
		char* writable2;
		//#pragma omp critical (xspace)
		{
			tmpSt=*itA;;
		}
		writable2 = new char[tmpSt.size() + 1];
		copy(tmpSt.begin(), tmpSt.end(), writable2);
		writable2[tmpSt.size()] = '\0';
		MDict.Search2(writable2, validWord); //search the word
		delete[] writable2; //free up variable for next loop use
	}

	//#pragma omp parallel for
	for(list<const char*>::iterator itV=validWord.begin();itV!=validWord.end();++itV){
		string tmpV;
		tmpV=(string)*itV;

		int cnt2=0;
		volatile boolean flagW=false;
		for(int h1=0;h1<tmpV.size();h1++)
		{
			for(int h2=0;h2<numberOfLetters;h2++)
			{
				if ((tmpV.at(h1)==lettersToPlace[h2]))
				{
					cnt2++;
				}
			}
		}

		if(cnt2==tmpV.size())
			flagW=true;

		int cnt1=count(tmpV.begin(),tmpV.end(),letterOB);

		if(cnt1==0)
		{
			continue;
		}

		else if(cnt1==1)
		{
			int starty;
			starty=b-tmpV.find(letterOB);
			volatile boolean VFlag=true;

			for(int ho=starty-1;ho<starty+tmpV.size()+1;ho++)
			{
				if(ho<0 || ho>22)
				{
					VFlag=false;
					break;
				}
				if(ho>0 && ho<22)
				{
					if(isalpha(board[a][ho].letter[0]))
					{
						if(ho==b)
							continue;
						VFlag=false;
						break;
					}
					else
					{
						/*if(a-1<0 || a+1>21)
						{
							VFlag=false;
							break;
						}*/
						if(a-1>0 && a+1<22)
						{
							if(isalpha(board[a-1][ho].letter[0]) || isalpha(board[a+1][ho].letter[0]))
							{
								VFlag=false;
								break;
							}
						}
					}
				}
			}

			if(VFlag && starty>0 && starty+tmpV.size()-1<22)
			{
				int score=calculationH(tmpV,a,starty,a,starty+tmpV.size()-1,lettersToPlace);
				#pragma omp critical (bestspace)
				{
					if(BestScore<score)
					{
						BestWord=tmpV;
						sx=a;
						sy=starty;
						ex=a;
						ey=starty+tmpV.size();
						BestScore=score;
						bestjokerword=jokerword;
					}
				}
			}
		}

		else if(cnt1>1)
		{
			for(int i1=0;i1<cnt1;i1++)
			{
				if(tmpV.at(i1)==letterOB)
				{
					int starty;
					starty=b-i1;
					boolean VFlag=true;

					for(int ho=starty-1;ho<starty+tmpV.size()+1;ho++)
					{
						if(ho<0 || ho>22)
						{
							VFlag=false;
							break;
						}
						if(ho>0 && ho<22)
						{
							if(isalpha(board[a][ho].letter[0]))
							{
								if(ho==b)
									continue;
								VFlag=false;
								break;
							}
							else
							{
								/*if(a-1<0 || a+1>21)
								{
									VFlag=false;
									break;
								}*/
								if(a-1>0 && a+1<22)
								{
									if(isalpha(board[a-1][ho].letter[0]) || isalpha(board[a+1][ho].letter[0]))
									{
										VFlag=false;
										break;
									}
								}
							}
						}
					}

					if(VFlag && starty>0 && starty+tmpV.size()-1<22)
					{
						int score=calculationH(tmpV,a,starty,a,starty+tmpV.size()-1,lettersToPlace);
						#pragma omp critical (bestspace)
						{
							if(BestScore<score)
							{
								BestWord=tmpV;
								sx=a;
								sy=starty;
								ex=a;
								ey=starty+tmpV.size();
								BestScore=score;
								bestjokerword=jokerword;
							}
						}
					}
				}
			}
		}
	}
	//#pragma omp parallel sections
	{
		//#pragma omp section
		{
			//validWord.clear();
		}
		//#pragma omp section
		{
			//allWord.clear();
		}
	}
}


void head1vertical(int a, int b, list<const char*>validWord)
{
	for(list<const char*>::iterator itV=validWord.begin();itV!=validWord.end();++itV){
		//int cnt=0;
		//while(isalpha(board[a][b+cnt].letter[0]) && !isalpha(board[a-1][b+cnt+1].letter[0]))
		{
			string tmpV=(string)*itV;
			char* check=new char[3];
			check[0]=tmpV.at(tmpV.size()-1);
			check[1]=board[a][b].letter[0];
			check[2]=NULL;
			if(MDict.checkValid(check)==0 && b-(tmpV.size()-1)>0)
			{
				int wordScore=calculationH(tmpV,a-1,b-(tmpV.size()-1),a-1,b,tileOnHand)+calculationH(check,a-1,b,a,b,"????????");
				#pragma omp critical (bestspace)
				{
					if(BestScore<wordScore)
					{
						BestWord=tmpV;
						sx=a-1;
						sy=b-(tmpV.size()-1);
						ex=a-1;
						ey=b;
						BestScore=wordScore;
						bestjokerword=jokerword;
					}
				}
				//cnt++;
			}
			else
				break;
		}
	}
}

void head2vertical(int a, int b, list<const char*> validWord)
{
	for(list<const char*>::iterator itV=validWord.begin();itV!=validWord.end();++itV){
		//int cnt=0;
		//while(isalpha(board[a][b+cnt].letter[0]) && !isalpha(board[a+1][b+cnt+1].letter[0]))
		{
			string tmpV=(string)*itV;
			char* check=new char[3];
			check[1]=tmpV.at(tmpV.size()-1);
			check[0]=board[a][b].letter[0];
			check[2]=NULL;
			if(MDict.checkValid(check)==0 && b-(tmpV.size()-1)>0)
			{
				int wordScore=calculationH(tmpV,a+1,b-(tmpV.size()-1),a+1,b,tileOnHand)+calculationH(check,a,b,a+1,b,"????????");
				#pragma omp critical (bestspace)
				{
					if(BestScore<wordScore)
					{
						BestWord=tmpV;
						sx=a+1;
						sy=b-(tmpV.size()-1);
						ex=a+1;
						ey=b;
						BestScore=wordScore;
						bestjokerword=jokerword;
					}
				}
				//cnt++;
			}
			else
				break;
		}
	}
}

void headvertical(int a, int b)
{
	list<const char*> validWord;
	list<string> allWord;

	print_all_permutations(tileOnHand, allWord);
	for(list<string>::iterator itA=allWord.begin();itA!=allWord.end();++itA){ //according to the word in the list to compare all permutated word	
		string tmpSt;
		char* writable2;
		tmpSt=*itA;
		writable2 = new char[tmpSt.size() + 1];
		copy(tmpSt.begin(), tmpSt.end(), writable2);
		writable2[tmpSt.size()] = '\0';
		MDict.Search2(writable2, validWord); //search the word
		delete[] writable2; //free up variable for next loop use
	}
	head1vertical(a,b,validWord);
	head2vertical(a,b,validWord);
}

void tail1vertical(int a, int b, list<const char*>validWord)
{
	for(list<const char*>::iterator itV=validWord.begin();itV!=validWord.end();++itV){
		//int cnt=0;
		//while(isalpha(board[a][b+cnt].letter[0]) && !isalpha(board[a-1][b+cnt-1].letter[0]))
		{
			string tmpV=(string)*itV;
			char* check=new char[3];
			check[0]=tmpV.at(0);
			check[1]=board[a][b].letter[0];
			check[2]=NULL;
			if(MDict.checkValid(check)==0 && b+tmpV.size()-1<22)
			{
				int wordScore=calculationH(tmpV,a-1,b,a-1,b+(tmpV.size()-1),tileOnHand)+calculationH(check,a-1,b,a,b,"????????");
				#pragma omp critical (bestspace)
				{
					if(BestScore<wordScore)
					{
						BestWord=tmpV;
						sx=a-1;
						sy=b;
						ex=a-1;
						ey=b+(tmpV.size()-1);
						BestScore=wordScore;
						bestjokerword=jokerword;
					}
				}
				//cnt--;
			}
			else
				break;
		}
	}
}

void tail2vertical(int a, int b, list<const char*> validWord)
{
	for(list<const char*>::iterator itV=validWord.begin();itV!=validWord.end();++itV){
		int cnt=0;
		//while(isalpha(board[a][b+cnt].letter[0]) && !isalpha(board[a+1][b+cnt-1].letter[0]))
		{
			string tmpV=(string)*itV;
			char* check=new char[3];
			check[1]=tmpV.at(0);
			check[0]=board[a][b+cnt].letter[0];
			check[2]=NULL;
			if(MDict.checkValid(check)==0 && b+tmpV.size()+1>22)
			{
				int wordScore=calculationH(tmpV,a+1,b,a+1,b+(tmpV.size()-1),tileOnHand)+calculationH(check,a,b,a+1,b,"????????");
				#pragma omp critical (bestspace)
				{
					if(BestScore<wordScore)
					{
						BestWord=tmpV;
						sx=a+1;
						sy=b;
						ex=a+1;
						ey=b+(tmpV.size()-1);
						BestScore=wordScore;
						bestjokerword=jokerword;
					}
				}
				//cnt--;
			}
			else
				break;
		}
	}
}

void tailvertical(int a, int b)
{
	list<const char*> validWord;
	list<string> allWord;

	print_all_permutations(tileOnHand, allWord);
	for(list<string>::iterator itA=allWord.begin();itA!=allWord.end();++itA){ //according to the word in the list to compare all permutated word	
		string tmpSt;
		char* writable2;
		tmpSt=*itA;
		writable2 = new char[tmpSt.size() + 1];
		copy(tmpSt.begin(), tmpSt.end(), writable2);
		writable2[tmpSt.size()] = '\0';
		MDict.Search2(writable2, validWord); //search the word
		delete[] writable2; //free up variable for next loop use
	}
	tail1vertical(a,b,validWord);
	tail2vertical(a,b,validWord);
}

void head1horizontal(int a, int b, list<const char*>validWord)
{
	for(list<const char*>::iterator itV=validWord.begin();itV!=validWord.end();++itV){
		//int cnt=0;
		//while(isalpha(board[a+cnt][b].letter[0]) && !isalpha(board[a+cnt+1][b-1].letter[0]))
		{
			string tmpV=(string)*itV;
			char* check=new char[3];
			check[0]=tmpV.at(tmpV.size()-1);
			check[1]=board[a][b].letter[0];
			check[2]=NULL;
			if(MDict.checkValid(check)==0 && a-(tmpV.size()-1)>0)
			{
				int wordScore=calculationH(tmpV,a-(tmpV.size()-1),b-1,a,b-1,tileOnHand)+calculationH(check,a,b-1,a,b,"????????");
				#pragma omp critical (bestspace)
				{
					if(BestScore<wordScore)
					{
						BestWord=tmpV;
						sx=a-(tmpV.size()-1);
						sy=b-1;
						ex=a;
						ey=b-1;
						BestScore=wordScore;
						bestjokerword=jokerword;
					}
				}
				//cnt--;
			}
			else
				break;
		}
	}
}

void head2horizontal(int a, int b, list<const char*> validWord)
{
	for(list<const char*>::iterator itV=validWord.begin();itV!=validWord.end();++itV){
		int cnt=0;
		//while(isalpha(board[a][b+cnt].letter[0]) && !isalpha(board[a+1][b+cnt-1].letter[0]))
		{
			string tmpV=(string)*itV;
			char* check=new char[3];
			check[1]=tmpV.at(tmpV.size()-1);
			check[0]=board[a][b+cnt].letter[0];
			check[2]=NULL;
			if(MDict.checkValid(check)==0 && a-(tmpV.size()-1)>0)
			{
				int wordScore=calculationH(tmpV,a-(tmpV.size()-1),b+1,a,b+1,tileOnHand)+calculationH(check,a,b,a,b+1,"????????");
				#pragma omp critical (bestspace)
				{
					if(BestScore<wordScore)
					{
						BestWord=tmpV;
						sx=a-(tmpV.size()-1);
						sy=b+1;
						ex=a;
						ey=b+1;
						BestScore=wordScore;
						bestjokerword=jokerword;
					}
				}
				//cnt--;
			}
			else
				break;
		}
	}
}

void headhorizontal(int a, int b)
{
	list<const char*> validWord;
	list<string> allWord;

	print_all_permutations(tileOnHand, allWord);
	for(list<string>::iterator itA=allWord.begin();itA!=allWord.end();++itA){ //according to the word in the list to compare all permutated word	
		string tmpSt;
		char* writable2;
		tmpSt=*itA;
		writable2 = new char[tmpSt.size() + 1];
		copy(tmpSt.begin(), tmpSt.end(), writable2);
		writable2[tmpSt.size()] = '\0';
		MDict.Search2(writable2, validWord); //search the word
		delete[] writable2; //free up variable for next loop use
	}
	head1horizontal(a,b,validWord);
	head2horizontal(a,b,validWord);
}

void tail1horizontal(int a, int b, list<const char*>validWord)
{
	for(list<const char*>::iterator itV=validWord.begin();itV!=validWord.end();++itV){
		//int cnt=0;
		//while(isalpha(board[a+cnt][b].letter[0]) && !isalpha(board[a+cnt+1][b-1].letter[0]))
		{
			string tmpV=(string)*itV;
			char* check=new char[3];
			check[0]=tmpV.at(0);
			check[1]=board[a][b].letter[0];
			check[2]=NULL;
			if(MDict.checkValid(check)==0 && a+(tmpV.size()-1)<22)
			{
				int wordScore=calculationH(tmpV,a,b-1,a+tmpV.size()-1,b-1,tileOnHand)+calculationH(check,a,b-1,a,b,"????????");
				#pragma omp critical (bestspace)
				{
					if(BestScore<wordScore)
					{
						BestWord=tmpV;
						sx=a;
						sy=b-1;
						ex=a+(tmpV.size()-1);
						ey=b-1;
						BestScore=wordScore;
						bestjokerword=jokerword;
					}
				}
				//cnt--;
			}
			else
				break;
		}
	}
}

void tail2horizontal(int a, int b, list<const char*> validWord)
{
	for(list<const char*>::iterator itV=validWord.begin();itV!=validWord.end();++itV){
		int cnt=0;
		//while(isalpha(board[a][b+cnt].letter[0]) && !isalpha(board[a+1][b+cnt-1].letter[0]))
		{
			string tmpV=(string)*itV;
			char* check=new char[3];
			check[1]=tmpV.at(0);
			check[0]=board[a][b+cnt].letter[0];
			check[2]=NULL;
			if(MDict.checkValid(check)==0 && a+(tmpV.size()-1)<22)
			{
				int wordScore=calculationH(tmpV,a,b+1,a+(tmpV.size()-1),b+1,tileOnHand)+calculationH(check,a,b,a,b+1,"????????");
				#pragma omp critical (bestspace)
				{
					if(BestScore<wordScore)
					{
						BestWord=tmpV;
						sx=a;
						sy=b+1;
						ex=a+(tmpV.size()-1);
						ey=b+1;
						BestScore=wordScore;
						bestjokerword=jokerword;
					}
				}
				//cnt--;
			}
			else
				break;
		}
	}
}

void tailhorizontal(int a, int b)
{
	list<const char*> validWord;
	list<string> allWord;

	print_all_permutations(tileOnHand, allWord);
	for(list<string>::iterator itA=allWord.begin();itA!=allWord.end();++itA){ //according to the word in the list to compare all permutated word	
		string tmpSt;
		char* writable2;
		tmpSt=*itA;
		writable2 = new char[tmpSt.size() + 1];
		copy(tmpSt.begin(), tmpSt.end(), writable2);
		writable2[tmpSt.size()] = '\0';
		MDict.Search2(writable2, validWord); //search the word
		delete[] writable2; //free up variable for next loop use
	}
	head1horizontal(a,b,validWord);
	head2horizontal(a,b,validWord);
}

void appendVerticalRight(int a,int b){ //position of last letter in WordOB
	char lettersToPlace[15];
	int numberOfLetters=0;
	list<const char*> validWord;
	list<string> allWord;
	int fx=0,lx=0,fy=0,ly=0;
	char Connect_point=' ';
	cout<<"\ncan append on"<<b<<' '<<a<<" vertical right =";

	//first, get the original word on board
	string wordOB="",newWord="";
	for(int x=b;x>0 && isalpha(board[a][x].letter[0]);x--){
		wordOB = board[a][x].letter[0] + wordOB;
	}
	cout<<"wordOB: "<<wordOB<<endl;
	//clear the list
	removeOldLetters(lettersToPlace, numberOfLetters);
	assignNewLetters(lettersToPlace, numberOfLetters);

	print_all_permutations(lettersToPlace, allWord);//permutate the tileonhand
	
	//check all permutated word and check if they are valid, then add into validWord list
	list<string>::iterator itA=allWord.begin();
	//#pragma omp parallel for
	for(int x=0;x<allWord.size();x++){ //according to the word in the list to compare all permutated word	
		string tmpSt;
		char* writable2=new char();
		#pragma omp critical (xspace)
		{
			tmpSt=*itA;
			itA++;
		}
		writable2 = new char[tmpSt.size() + 1];
		copy(tmpSt.begin(), tmpSt.end(), writable2);
		writable2[tmpSt.size()] = '\0';
		MDict.Search2(writable2, validWord); //search the word
		delete[] writable2; //free up variable for next loop use
	}

	//now append the letter of every word in validWord list into the WordOB and check if is a valid word
	list<const char*>::iterator itV=validWord.begin();  //check all valid word
	//#pragma omp parallel for
	for(int x=0;x<validWord.size();x++){
		string tmpV;
		#pragma omp critical (yspace)
		{
			tmpV=(string)*itV;
			itV++;
		}
		//parallel here
		for(int v=0; v<tmpV.length(); v++){ //for every valid word, choose each of the letter and append to the word on board
			char* writable3=new char();
			Connect_point=tmpV[v];
			newWord = wordOB + tmpV[v]; //append letter of valid word in position v into end of wordOB
			writable3 = new char[newWord.size() + 1];
			copy(newWord.begin(), newWord.end(), writable3);
			writable3[newWord.size()] = '\0';
			if(MDict.checkValid(writable3)==0){ //search the new word, if is valid word search space n calculate score
				//get the position of the word that connect the wordOB
				volatile boolean v_flag=true;
				//check insert space
				fx=lx=b+1;
				fy=a-v;
				ly=tmpV.length()-v+a-1;
				for(int point_y = a-v-1 ;point_y <= tmpV.length()-v+a && v_flag;point_y++){ //check on the row that need to insert
					if(point_y < 1 || point_y >22){  //out of bound
						v_flag=false;
						break;
					}
					if(isalpha(board[point_y][b+1].letter[0])){
						v_flag=false;
						break;
					}
				}
				//check rightside
				for(int point_y = a-v ;point_y < tmpV.length()-v+a && v_flag;point_y++){
					if(point_y < 1 || point_y >22){  //out of bound
						v_flag=false;
						break;
					}
					if(b+1 < 22)
					if(isalpha(board[point_y][b+2].letter[0])){
						v_flag=false;
						break;
					}
				}
				//check leftside
				for(int point_y = a-v ;point_y < tmpV.length()-v+a && v_flag;point_y++){
					if(point_y < 1 || point_y >22){  //out of bound
						v_flag=false;
						break;
					}
					if(point_y == a || point_y == a-1 || point_y == a+1) //ald checked
						continue;
					if(isalpha(board[point_y][b].letter[0])){
						v_flag=false;
						break;
					}
				}
				if(v_flag){
					int scoreNew=calculationH(tmpV,fy,fx,ly,lx,lettersToPlace);
					int scoreOld=0;
					for(int x=b;x>0 && isalpha(board[a][x].letter[0]);x--){
						scoreOld += board[a][x].score;
					}
					for(int gg=0;gg<numberOfLetters;gg++){
						if(Connect_point==lettersToPlace[gg]){
							int lScore=letterScore(Connect_point);
							if(board[a][b+1].letter[0]=='!')
								scoreOld+=lScore*2;
							else if(board[a][b+1].letter[0]=='@')
								scoreOld+=lScore*3;
							else if(board[a][b+1].letter[0]=='#')
								scoreOld+=lScore*4;
							else if(board[a][b+1].letter[0]=='$')
								scoreOld=(scoreOld+lScore)*2;
							else if(board[a][b+1].letter[0]=='%')
								scoreOld=(scoreOld+lScore)*3;
							else if(board[a][b+1].letter[0]=='^')
								scoreOld=(scoreOld+lScore)*4;
							else
								scoreOld+=lScore;
							break;
						}
					}
					int score=scoreOld+scoreNew;
					#pragma omp critical (bestspace)
					{
						if(BestScore<score)
						{
							BestWord=tmpV;
							sx=fy;
							sy=fx;
							ex=ly;
							ey=lx;
							BestScore=score;
						}
					}
					cout<<writable3<<": "<<tmpV<<"="<<calculationH(tmpV,fy,fx,ly,lx,lettersToPlace)<<'\t';
				}
			}
			delete[] writable3; //free up variable for next loop use
		}
	}
}

void appendVerticalLeft(int a,int b){
	char lettersToPlace[15];
	int numberOfLetters=0;
	list<const char*> validWord;
	list<string> allWord;
	int fx=0,lx=0,fy=0,ly=0;
	char Connect_point=' ';

	cout<<"\ncan append on"<<b<<' '<<a<<" vertical left =";
	string wordOB="",newWord="";
	for(int x=b;x<22 && isalpha(board[a][x].letter[0]);x++){
		wordOB += board[a][x].letter[0];
	}
	cout<<"wordOB: "<<wordOB<<endl;
	
	//HANDLE h1=CreateThread(NULL,0,renew,NULL,0,NULL);
	removeOldLetters(lettersToPlace, numberOfLetters);
	assignNewLetters(lettersToPlace, numberOfLetters);

	//WaitForSingleObject(h1,INFINITE);
	print_all_permutations(lettersToPlace, allWord);

	list<string>::iterator itA=allWord.begin();
	//#pragma omp parallel for
	for(int x=0;x<allWord.size();x++){ //according to the word in the list to compare all permutated word	
		string tmpSt;
		char* writable2=new char();
		#pragma omp critical (xspace)
		{
			tmpSt=*itA;
			itA++;
		}
		writable2 = new char[tmpSt.size() + 1];
		copy(tmpSt.begin(), tmpSt.end(), writable2);
		writable2[tmpSt.size()] = '\0';
		MDict.Search2(writable2, validWord); //search the word
		delete[] writable2; //free up variable for next loop use
	}

	//now append the letter of every word in validWord list into the front of WordOB and check if is a valid word
	list<const char*>::iterator itV=validWord.begin();  //check all valid word
	//#pragma omp parallel for
	for(int x=0;x<validWord.size();x++){
		string tmpV;
		#pragma omp critical (yspace)
		{
			tmpV=(string)*itV;
			itV++;
		}
		//parallel here
		for(int v=0; v<tmpV.length(); v++){ //for every valid word, choose each of the letter and append to the word on board
			char* writable3=new char();
			newWord = tmpV[v] + wordOB; //append letter of valid word in position v in front of wordOB
			writable3 = new char[newWord.size() + 1];
			copy(newWord.begin(), newWord.end(), writable3);
			writable3[newWord.size()] = '\0';
			if(MDict.checkValid(writable3)==0){ //search the new word, if is valid word
				//get the position of the word that connect the wordOB
				volatile boolean v_flag=true;
				//check insert space
				fx=lx=b-1;
				fy=a-v;
				ly=tmpV.length()-v+a-1;
				for(int point_y = a-v-1 ;point_y <= tmpV.length()-v+a && v_flag;point_y++){ //check on the row that need to insert
					if(point_y < 1 || point_y >22){  //out of bound
						v_flag=false;
						break;
					}
					if(isalpha(board[point_y][b-1].letter[0])){
						v_flag=false;
						break;
					}
				}
				//check leftside
				for(int point_y = a-v ;point_y < tmpV.length()-v+a && v_flag;point_y++){
					if(point_y < 1 || point_y >22){  //out of bound
						v_flag=false;
						break;
					}
					if(b-1 > 1)
					if(isalpha(board[point_y][b-2].letter[0])){
						v_flag=false;
						break;
					}
				}
				//check rightside
				for(int point_y = a-v ;point_y < tmpV.length()-v+a && v_flag;point_y++){
					if(point_y < 1 || point_y > 22){  //out of bound
						v_flag=false;
						break;
					}
					if(point_y == a || point_y == a-1 || point_y == a+1) //ald checked
						continue;
					if(isalpha(board[point_y][b].letter[0])){
						v_flag=false;
						break;
					}
				}
				if(v_flag){
					Connect_point=tmpV[v];
					int scoreNew=calculationH(tmpV,fy,fx,ly,lx,lettersToPlace);
					int scoreOld=0;
					for(int x1=b;x<22 && isalpha(board[a][x1].letter[0]);x1++){
						scoreOld += board[a][x1].score;
					}
					cout<<"OldScore: "<<scoreOld<<'\t';
					for(int gg=0;gg<numberOfLetters;gg++){
						if(Connect_point==lettersToPlace[gg]){
							int lScore=letterScore(Connect_point);
							if(board[a][b-1].letter[0]=='!')
								scoreOld+=lScore*2;
							else if(board[a][b-1].letter[0]=='@')
								scoreOld+=lScore*3;
							else if(board[a][b-1].letter[0]=='#')
								scoreOld+=lScore*4;
							else if(board[a][b-1].letter[0]=='$')
								scoreOld=(scoreOld+lScore)*2;
							else if(board[a][b-1].letter[0]=='%')
								scoreOld=(scoreOld+lScore)*3;
							else if(board[a][b-1].letter[0]=='^')
								scoreOld=(scoreOld+lScore)*4;
							else
								scoreOld+=lScore;
							break;
						}
					}
					cout<<"total OldScore: "<<scoreOld<<'\t';
					int score=scoreOld+scoreNew;
					#pragma omp critical (bestspace)
					{
						if(BestScore<score)
						{
							BestWord=tmpV;
							sx=fy;
							sy=fx;
							ex=ly;
							ey=lx;
							BestScore=score;
						}
					}
					cout<<writable3<<": "<<tmpV<<"="<<calculationH(tmpV,fy,fx,ly,lx,lettersToPlace)<<'\t';
				}
			}
			delete[] writable3; //free up variable for next loop use
		}
	}
}

void appendHorizontalTop(int a,int b){
	char lettersToPlace[15];
	int numberOfLetters=0;
	list<const char*> validWord;
	list<string> allWord;
	int fx=0,lx=0,fy=0,ly=0;
	char Connect_point=' ';

	cout<<"can append on"<<b<<' '<<a<<" horizontal top =";
	string wordOB="",newWord="";
	
	for(int y1=a;y1<22 && isalpha(board[y1][b].letter[0]);y1++){
		wordOB += board[y1][b].letter[0];
	}
	cout<<"wordOB: "<<wordOB<<endl;
	
	removeOldLetters(lettersToPlace, numberOfLetters);
	assignNewLetters(lettersToPlace, numberOfLetters);

	//WaitForSingleObject(h1,INFINITE);
	print_all_permutations(lettersToPlace, allWord);

	list<string>::iterator itA=allWord.begin();
	//#pragma omp parallel for
	for(int x=0;x<allWord.size();x++){ //according to the word in the list to compare all permutated word	
		string tmpSt;
		char* writable2=new char();
		#pragma omp critical (xspace)
		{
			tmpSt=*itA;
			itA++;
		}
		writable2 = new char[tmpSt.size() + 1];
		copy(tmpSt.begin(), tmpSt.end(), writable2);
		writable2[tmpSt.size()] = '\0';
		MDict.Search2(writable2, validWord); //search the word
		delete[] writable2; //free up variable for next loop use
	}

	//now append the letter of every word in validWord list into the front of WordOB and check if is a valid word
	list<const char*>::iterator itV=validWord.begin();  //check all valid word
	//#pragma omp parallel for
	for(int x=0;x<validWord.size();x++){
		string tmpV;
		#pragma omp critical (yspace)
		{
			tmpV=(string)*itV;
			itV++;
		}
		//parallel here
		for(int v=0; v<tmpV.length(); v++){ //for every valid word, choose each of the letter and append to the word on board
			char* writable3=new char();
			newWord = tmpV[v] + wordOB; //append letter of valid word in position v in front of wordOB
			writable3 = new char[newWord.size() + 1];
			copy(newWord.begin(), newWord.end(), writable3);
			writable3[newWord.size()] = '\0';
			if(MDict.checkValid(writable3)==0){ //search the new word, if is valid word
				//get the position of the word that connect the wordOB
				volatile boolean v_flag=true;
				//check insert space
				fy=ly=a-1;
				fx=b-v;
				lx=tmpV.length()-v+b-1;
				for(int point_x = b-v-1 ;point_x <= tmpV.length()-v+b && v_flag;point_x++){ //check on the row that need to insert
					if(point_x < 1 || point_x >22){  //out of bound
						v_flag=false;
						break;
					}
					if(isalpha(board[a-1][point_x].letter[0])){
						v_flag=false;
						break;
					}
				}
				//check top
				for(int point_x = b-v ;point_x < tmpV.length()-v+b && v_flag;point_x++){
					if(point_x < 1 || point_x >22){  //out of bound
						v_flag=false;
						break;
					}
					if(a-1 > 1)
					if(isalpha(board[a-2][point_x].letter[0])){
						v_flag=false;
						break;
					}
				}
				//check bottom
				for(int point_x = b-v ;point_x < tmpV.length()-v+b && v_flag;point_x++){
					if(point_x < 1 || point_x > 22){  //out of bound
						v_flag=false;
						break;
					}
					if(point_x == b || point_x == b-1 || point_x == b+1) //ald checked
						continue;
					if(isalpha(board[a][point_x].letter[0])){
						v_flag=false;
						break;
					}
				}
				if(v_flag){
					Connect_point=tmpV[v];
					int scoreNew=calculationH(tmpV,fy,fx,ly,lx,lettersToPlace);
					int scoreOld=0;
					for(int y1=a;x<22 && isalpha(board[y1][b].letter[0]);y1++){
						scoreOld += board[y1][b].score;
					}
					cout<<"OldScore: "<<scoreOld<<'\t';
					for(int gg=0;gg<numberOfLetters;gg++){
						if(Connect_point==lettersToPlace[gg]){
							int lScore=letterScore(Connect_point);
							if(board[a-1][b].letter[0]=='!')
								scoreOld+=lScore*2;
							else if(board[a-1][b].letter[0]=='@')
								scoreOld+=lScore*3;
							else if(board[a-1][b].letter[0]=='#')
								scoreOld+=lScore*4;
							else if(board[a-1][b].letter[0]=='$')
								scoreOld=(scoreOld+lScore)*2;
							else if(board[a-1][b].letter[0]=='%')
								scoreOld=(scoreOld+lScore)*3;
							else if(board[a-1][b].letter[0]=='^')
								scoreOld=(scoreOld+lScore)*4;
							else
								scoreOld+=lScore;
							break;
						}
					}
					cout<<"total OldScore: "<<scoreOld<<'\t';
					int score=scoreOld+scoreNew;
					#pragma omp critical (bestspace)
					{
						if(BestScore<score)
						{
							BestWord=tmpV;
							sx=fy;
							sy=fx;
							ex=ly;
							ey=lx;
							BestScore=score;
						}
					}
					cout<<writable3<<": "<<tmpV<<"="<<calculationH(tmpV,fy,fx,ly,lx,lettersToPlace)<<'\t';
				}
			}
			delete[] writable3; //free up variable for next loop use
		}
	}

}

void appendHorizontalBottom(int a,int b){
	char lettersToPlace[15];
	int numberOfLetters=0;
	list<const char*> validWord;
	list<string> allWord;
	int fx=0,lx=0,fy=0,ly=0;
	char Connect_point=' ';

	cout<<"can append on"<<b<<' '<<a<<" horizontal bottom =";
	string wordOB="",newWord="";
	for(int y=a;y>0 && isalpha(board[y][b].letter[0]);y--){
		wordOB = board[y][b].letter[0] + wordOB;
	}
	cout<<"wordOB: "<<wordOB<<endl;
	
	//clear the list
	removeOldLetters(lettersToPlace, numberOfLetters);
	assignNewLetters(lettersToPlace, numberOfLetters);

	print_all_permutations(lettersToPlace, allWord);//permutate the tileonhand
	
	//check all permutated word and check if they are valid, then add into validWord list
	list<string>::iterator itA=allWord.begin();
	//#pragma omp parallel for
	for(int x=0;x<allWord.size();x++){ //according to the word in the list to compare all permutated word	
		string tmpSt;
		char* writable2=new char();
		#pragma omp critical (xspace)
		{
			tmpSt=*itA;
			itA++;
		}
		writable2 = new char[tmpSt.size() + 1];
		copy(tmpSt.begin(), tmpSt.end(), writable2);
		writable2[tmpSt.size()] = '\0';
		MDict.Search2(writable2, validWord); //search the word
		delete[] writable2; //free up variable for next loop use
	}

	//now append the letter of every word in validWord list into the WordOB and check if is a valid word
	list<const char*>::iterator itV=validWord.begin();  //check all valid word
	//#pragma omp parallel for
	for(int x=0;x<validWord.size();x++){
		string tmpV;
		#pragma omp critical (yspace)
		{
			tmpV=(string)*itV;
			itV++;
		}
		//parallel here
		for(int v=0; v<tmpV.length(); v++){ //for every valid word, choose each of the letter and append to the word on board
			char* writable3=new char();
			newWord = wordOB + tmpV[v]; //append letter of valid word in position v into end of wordOB
			writable3 = new char[newWord.size() + 1];
			copy(newWord.begin(), newWord.end(), writable3);
			writable3[newWord.size()] = '\0';
			if(MDict.checkValid(writable3)==0){ //search the new word, if is valid word search space n calculate score
				//get the position of the word that connect the wordOB
				volatile boolean v_flag=true;
				//check insert space
				fy=ly=a+1;
				fx=b-v;
				lx=tmpV.length()-v+b-1;
				for(int point_x = b-v-1 ;point_x <= tmpV.length()-v+b && v_flag;point_x++){ //check on the row that need to insert
					if(point_x < 1 || point_x >22){  //out of bound
						v_flag=false;
						break;
					}
					if(isalpha(board[a+1][point_x].letter[0])){
						v_flag=false;
						break;
					}
				}
				//check bottom
				for(int point_x = b-v ;point_x < tmpV.length()-v+b && v_flag;point_x++){
					if(point_x < 1 || point_x >22){  //out of bound
						v_flag=false;
						break;
					}
					if(a+1 < 21)
					if(isalpha(board[a+2][point_x].letter[0])){
						v_flag=false;
						break;
					}
				}
				//check top
				for(int point_x = b-v ;point_x < tmpV.length()-v+b && v_flag;point_x++){
					if(point_x < 1 || point_x > 22){  //out of bound
						v_flag=false;
						break;
					}
					if(point_x == b || point_x == b-1 || point_x == b+1) //ald checked
						continue;
					if(isalpha(board[a][point_x].letter[0])){
						v_flag=false;
						break;
					}
				}
				if(v_flag){
					Connect_point=tmpV[v];
					int scoreNew=calculationH(tmpV,fy,fx,ly,lx,lettersToPlace);
					int scoreOld=0;
					for(int y1=a;y1>0 && isalpha(board[y1][b].letter[0]);y1--){
						scoreOld += board[y1][b].score;
					}
					cout<<"OldScore: "<<scoreOld<<'\t';
					for(int gg=0;gg<numberOfLetters;gg++){
						if(Connect_point==lettersToPlace[gg]){
							int lScore=letterScore(Connect_point);
							if(board[a+1][b].letter[0]=='!')
								scoreOld+=lScore*2;
							else if(board[a+1][b].letter[0]=='@')
								scoreOld+=lScore*3;
							else if(board[a+1][b].letter[0]=='#')
								scoreOld+=lScore*4;
							else if(board[a+1][b].letter[0]=='$')
								scoreOld=(scoreOld+lScore)*2;
							else if(board[a+1][b].letter[0]=='%')
								scoreOld=(scoreOld+lScore)*3;
							else if(board[a+1][b].letter[0]=='^')
								scoreOld=(scoreOld+lScore)*4;
							else
								scoreOld+=lScore;
							break;
						}
					}
					cout<<"total OldScore: "<<scoreOld<<'\t';
					int score=scoreOld+scoreNew;
					#pragma omp critical (bestspace)
					{
						if(BestScore<score)
						{
							BestWord=tmpV;
							sx=fy;
							sy=fx;
							ex=ly;
							ey=lx;
							BestScore=score;
						}
					}
					cout<<writable3<<": "<<tmpV<<"="<<calculationH(tmpV,fy,fx,ly,lx,lettersToPlace)<<'\t';
				}
			}
			delete[] writable3; //free up variable for next loop use
		}
	}
}

void normalCase(){
	BestScore=0;
	//volatile int flag=1;
	for(int a=bfirstx;a<blastx+1;a++)
	{
		#pragma omp parallel
		for(int b=bfirsty+omp_get_thread_num();b<blasty+1;b+=numCPU)
		{
			if(isalpha(board[a][b].letter[0]))
			{
				/*if(flag==0)break;
				if(a-1<0 || b-1<0 || a+1>21 || b+1>21)
				{
					flag=0;
					break;
				}*/
				if(a-1>0 && b-1>0 && a+1<22 && b+1<22)
				{
					if(!isalpha(board[a-1][b].letter[0]) && !isalpha(board[a+1][b].letter[0]))//vertical
					{
						verticalPosition(a, b);
					}
					if(!isalpha(board[a][b-1].letter[0]) && !isalpha(board[a][b+1].letter[0]))//horizontal
					{
						horizontalPosition(a, b);
					}
					if(!isalpha(board[a-1][b].letter[0]) && !isalpha(board[a+1][b].letter[0]) && !isalpha(board[a][b-1].letter[0]))
					{
						headvertical(a, b);
					}
					if(!isalpha(board[a-1][b].letter[0]) && !isalpha(board[a+1][b].letter[0]) && !isalpha(board[a][b+1].letter[0]))
					{
						tailvertical(a, b);
					}
					if(!isalpha(board[a][b-1].letter[0]) && !isalpha(board[a][b+1].letter[0]) && !isalpha(board[a-1][b].letter[0]))
					{
						headhorizontal(a, b);
					}
					if(!isalpha(board[a][b-1].letter[0]) && !isalpha(board[a][b+1].letter[0]) && !isalpha(board[a-1][b].letter[0]))
					{
						tailhorizontal(a, b);
					}
					if((!isalpha(board[a-1][b].letter[0]) && isalpha(board[a+1][b].letter[0])) || (isalpha(board[a-1][b].letter[0]) && !isalpha(board[a+1][b].letter[0])))//confirm is vertical
					{
						if(!isalpha(board[a][b-1].letter[0]) && !isalpha(board[a][b+1].letter[0])){ //check if it is head or tail in vertical
							if(!isalpha(board[a-2][b].letter[0]) && isalpha(board[a+1][b].letter[0])){  //append horizontal top
								appendHorizontalTop(a,b);
							}
							else if(!isalpha(board[a+2][b].letter[0]) && isalpha(board[a-1][b].letter[0])){ //append horizontal bottom
								appendHorizontalBottom(a,b);
							}
						}
					}
					if((!isalpha(board[a][b-1].letter[0]) && isalpha(board[a][b+1].letter[0])) || (isalpha(board[a][b-1].letter[0]) && !isalpha(board[a][b+1].letter[0])))//confirm is Horizontal
					{
						if(!isalpha(board[a-1][b].letter[0]) && !isalpha(board[a+1][b].letter[0])){ //check if it's head or tail in horizontal
							if(!isalpha(board[a][b-2].letter[0]) && isalpha(board[a][b+1].letter[0])){  //append on vertical head
								appendVerticalLeft( a, b);
							}
							else if(!isalpha(board[a][b+2].letter[0]) && isalpha(board[a][b-1].letter[0])){ //append on vertical tail
								appendVerticalRight( a, b);
							}
						}
					}
				}
			}
		}
	}
	
	for(int a=0;a<BestWord.length();a++)
	{
		if(sx-ex==0)//placement best word horizontal
		{
			if(board[sx][sy+a].letter[0]==BestWord.at(a))
				continue;
			board[sx][sy+a].letter=new (char[2]);
			board[sx][sy+a].letter[0]=BestWord.at(a);
			board[sx][sy+a].letter[1]=NULL;
			if(bestjokerword.at(a)=='*')
				board[sx+a][sy].score=0;
			else
				board[sx][sy+a].score=letterScore(BestWord.at(a));
		}
		else if(sy-ey==0)//placement best word vertical
		{
			if(board[sx+a][sy].letter[0]==BestWord.at(a))
				continue;
			board[sx+a][sy].letter=new (char[2]);
			board[sx+a][sy].letter[0]=BestWord.at(a);
			board[sx+a][sy].letter[1]=NULL;
			if(bestjokerword.at(a)=='*')
				board[sx+a][sy].score=0;
			else
				board[sx+a][sy].score=letterScore(BestWord.at(a));
		}
	}

	if(bfirsty>sy)
		bfirsty=sy;
	if(blasty<sy+BestWord.length()-1)
		blasty=sy+BestWord.length()-1;
	if(bfirstx>sx)
		bfirstx=sx;
	if(blastx<sx+BestWord.length()-1)
		blastx=sx+BestWord.length()-1;

	PlayerScore+=BestScore;
}

void main()
{	
	list<string> allWord;
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );
	numCPU = sysinfo.dwNumberOfProcessors;//get number of CPU

	volatile boolean endgame=false;
	HANDLE h1=CreateThread(NULL,0,readFile,NULL,0,NULL);//sponse thread to readfile

	setBoard();
	displayBoard();

	clock_t begin, end;
	begin=clock();

	/*if(numJoker>0)
	{
		string tmpTOH=tileOnHand;
		int tmpJ=numJoker;
		InitializeCriticalSectionAndSpinCount(&cs,0);
		jokerGenerate(tmpTOH,tmpJ);
		DeleteCriticalSection(&cs);
	}
	else
		print_all_permutations(tileOnHand, allWord);*/

	//WaitForSingleObject(h1,INFINITE);

	//compare from all permutation word
	/*list<string>::iterator itA=allWord.begin();
	for(int x=0;x<allWord.size();x++){ //according to the word in the list to compare all permutated word	
		string tmpSt;
		char* writable2=new char();
		#pragma omp critical (xspace)
		{
			tmpSt=*itA;
			itA++;
		}
		writable2 = new char[tmpSt.size() + 1];
		copy(tmpSt.begin(), tmpSt.end(), writable2);
		writable2[tmpSt.size()] = '\0';
		MDict.Search2(writable2, validWord); //search the word
		delete[] writable2; //free up variable for next loop use
	}
	//use the valid word to place in the board with highest score.*/

	while(!endgame)
	{
		cout<<"Please key in the letters on your hand: "<<endl;
		cin>>tileOnHand;
		numJoker=0;
		for(int a=0;a<7;a++)
		{
			tileOnHand[a]=toupper(tileOnHand[a]);
			if(tileOnHand[a]=='*')
				numJoker++;
		}
		WaitForSingleObject(h1,INFINITE);
		begin=clock();
		if(board[11][11].letter[0]=='$'){
				firstGame(allWord, numJoker, tileOnHand);  //search highest score placement
				gamePlay++;
		}
		else
		{
			//WaitForSingleObject(h1,INFINITE);
			normalCase();
			gamePlay++;
		}
		end=clock();
		cout<<endl;
		displayBoard();
		//h1=CreateThread(NULL,0,renew,NULL,0,NULL);

		cout<<"Best word and Score: "<<BestWord<<' '<<BestScore<<endl;
		cout<<"Player Score "<<PlayerScore<<endl;
		cout<<(double)(end - begin) / CLOCKS_PER_SEC<<endl;
	}

	system("pause");
}