#include <iostream>
#include "CarParkSimulator.h"
#include "State.h"
#include <string>

using namespace std;

void main()
{
	CarParkSimulator C;
	State S;
	string filename;
	bool flag=true;

	cout<<"Welcome To Car Park Simulator."<<endl<<endl;
	cout<<"Please enter the file name for car park file: "<<endl;
	cout<<"(Press ""Q"" for exit)"<<endl;
	getline(cin,filename);

	if(filename.compare("q")==0)
	{
		filename.clear();
		filename="Q";
	}

	//if want to exit game before start game, then run this condition
	if(filename.compare("Q")==0 && filename.size()==1)
	{
		flag=false;
	}

	//if the file does not exist, it will ask user to key in again
	while(!C.LoadGame(filename, S) && flag)
	{
		cout<<"This file does not exist, please try again."<<endl;
		cout<<"Please enter the file name for car park file: "<<endl;
		cout<<"(Press ""Q"" for exit)"<<endl;
		getline(cin,filename);

		if(filename.compare("q")==0)
		{
			filename.clear();
			filename="Q";
		}

		if(filename.compare("Q")==0 && filename.size()==1)
		{
			filename.clear();
			filename="Q";
			flag=false;
		}
	}

	if(flag)
	//if the file exists, it will start the game
	{
		cout<<"This file exists."<<endl;
		C.BFS(S);
	}

	cout<<"Good Bye."<<endl;

	system("pause");
}