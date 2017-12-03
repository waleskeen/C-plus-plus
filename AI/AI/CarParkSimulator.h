//this file is for the car park simulator game

#ifndef CARPARKSIMULATOR_TYPE
#define CARPARKSIMULATOR_TYPE

#include <iostream>
#include <fstream>
#include "State.h"
#include <string>
#include <list>
#include <ctime>
#include "Vehicle.h"
#include "InEdge.h"

using namespace std;

class CarParkSimulator
{
private:
	int NumberofVehicle;
	State IS;
	Vehicle V;
	InEdge IE;
	int nodes;
	double sec;
	int branch;
	bool IsGameOver(list<State>::iterator);
	void GetMoves(State, list<State>&);
	bool Exist(list<State>::iterator, 
		list<State>&, list<State>&);
public:
	CarParkSimulator();
	bool LoadGame(string, State &);
	void BFS(State S);
};

//Constructor
CarParkSimulator::CarParkSimulator()
{
	nodes=0;
}

//read the file
bool CarParkSimulator::LoadGame(string filename, State &S)
{
	ifstream infile;
	
	infile.open(filename);
	if(!infile)
	{
		infile.close();
		return false;
	}
	else
	{
		//insert the number of vehicle to program	
		infile>>NumberofVehicle;
		while(!infile.eof())
		{
			infile>>V.VehicleID;//insert the vehicle ID
			for(int a=0;a<2;a++)
			{
				for(int b=0;b<2;b++)
					infile>>V.positionxy[a][b];
			}
			infile>>V.position;

			/*
			if the vehicle's position is vertical, 
			then perfrom this condition to store 
			the unuseable varieble to 100 
			*/
			if(V.position=='V')
			{
				int rowNumber = V.positionxy[0][0];
				int colNumber = V.positionxy[0][1]; 
				V.length = (V.positionxy[1][0]-V.positionxy[0][0])+1;
				
				for(int a=0; a<V.length; a++)
				{
					int newRowNumber = rowNumber + a;	
					V.positionxy[a][0] = newRowNumber;
					V.positionxy[a][1] = colNumber; 
				}						
					
				for(int b=0; b<6; b++)
				{
					if(V.positionxy[b][1] != colNumber)
					{
						V.positionxy[b][0] = 100;
						V.positionxy[b][1] = 100;
					}
				}
				
				if(V.positionxy[0][0] >= V.positionxy[V.length][0])
				{
					V.positionxy[V.length][0] = 200;
					V.positionxy[V.length][1] = 200;
				}
			}

			/*
			if the vehicle's position is vertical, 
			then perfrom this condition to store 
			the unuseable varieble to 100 
			*/
			else if(V.position=='H')
			{
				int rowNumber = V.positionxy[0][0];
				int colNumber = V.positionxy[0][1]; 
				V.length = (V.positionxy[1][1]-V.positionxy[0][1])+1;
				
				for(int a=0; a<V.length; a++)
				{
					int newColNumber = colNumber + a;	
					V.positionxy[a][0] = rowNumber;
					V.positionxy[a][1] = newColNumber; 
				}						
					
				for(int b=0; b<6; b++)
				{
					if(V.positionxy[b][0] != rowNumber)
					{
						V.positionxy[b][0] = 100;
						V.positionxy[b][1] = 100;
					}
				}
				if((V.positionxy[0][1] >= V.positionxy[V.length][1]))
				{
					V.positionxy[V.length][0] = 200;
					V.positionxy[V.length][1] = 200;
				}
			}
			S.V.push_back(V);
		}
		S.V.pop_back();//to remove the duplicate vehicle's node
		S.storeindisplay();
		nodes++;
		S.StateID=nodes;
		infile.close();
		return true;
	}
}

//to check the particular state is goal state
bool CarParkSimulator::IsGameOver(list<State>::iterator S)
{
	S->itV=S->V.begin();
	for (S->V.begin();S->itV != S->V.end(); S->itV++)
	{
		if(S->itV->VehicleID=='X')
		{
			//to check weather the vehicle 'X' has been blocked
			for(int a=S->itV->positionxy[S->itV->length-1][1]+1;a<=5;a++)
			{
				if(S->stateaction[2][a]!='-')
					return false;
			}
			break;
		}
	}
	return true;
}

//to move the car
void CarParkSimulator::GetMoves(State S, list<State> &LS)
{
	S.itV=S.V.begin();
	State PARENT,CHILDREN;
	IE.InEdgeID=S.IE.InEdgeID+1;

	for(S.V.begin();S.itV!=S.V.end();S.itV++)
	{
		V=*S.itV;
		PARENT=S;
		CHILDREN=S;
		IE.moves=0;
		if(V.position=='H')
		{
			//assume the vehicle move to left
			IE.direction='L';
			if(IE.direction=='L')
			{
				//if the car is not the over of the left
				if(V.positionxy[0][1]!=0)
				{
					int row=V.positionxy[0][0];
					int col=V.positionxy[0][1];
					for(int a=col-1;a>=0;a--)
					{
						//if the car has been blocked
						if(CHILDREN.stateaction[row][a]!='-')
							break;
						//if the car has not been blocked
						else
						{
							for(int b=0;b<V.length;b++)
							{
								V.positionxy[b][1]--;
							}
							CHILDREN.storeindisplay2(V);
							CHILDREN.V.remove(V);
							CHILDREN.V.push_back(V);
							IE.moves++;

							//to store the action into the state
							CHILDREN.IE.V=V.VehicleID;
							CHILDREN.IE.direction=IE.direction;
							CHILDREN.IE.moves=IE.moves;
							CHILDREN.IE.InEdgeID=IE.InEdgeID;

							CHILDREN.parent=new State;
							*CHILDREN.parent=PARENT;
							LS.push_back(CHILDREN);
						}
					}
				}
			}
			V=*S.itV;
			PARENT=S;
			CHILDREN=S;
			IE.moves=0;

			//assume the vehicle move to right
			IE.direction='R';
			if(IE.direction=='R')
			{				
				//if the car is not the over of the right
				if(V.positionxy[V.length-1][1]!=5)
				{
					int row=V.positionxy[V.length-1][0];
					int col=V.positionxy[V.length-1][1];
					for(int a=col+1;a<=5;a++)
					{
						//if the car has been blocked
						if(CHILDREN.stateaction[row][a]!='-')
							break;
						//if the car has not been blocked
						else
						{
							for(int b=0;b<V.length;b++)
							{
								V.positionxy[b][1]++;
							}
							CHILDREN.storeindisplay2(V);
							CHILDREN.V.remove(V);
							CHILDREN.V.push_back(V);
							IE.moves++;

							//to store the action into the state
							CHILDREN.IE.V=V.VehicleID;
							CHILDREN.IE.direction=IE.direction;
							CHILDREN.IE.moves=IE.moves;
							CHILDREN.IE.InEdgeID=IE.InEdgeID;

							CHILDREN.parent=new State;
							*CHILDREN.parent=PARENT;
							LS.push_back(CHILDREN);
						}
					}
				}
			}
		}
		else if(V.position=='V')
		{
			//assume the vehicle move to top
			IE.direction='T';
			if(IE.direction=='T')
			{
				//if the car is not the over of the top
				if(V.positionxy[0][0]!=0)
				{
					int row=V.positionxy[0][0];
					int col=V.positionxy[0][1];
					for(int a=row-1;a>=0;a--)
					{
						//if the car has been blocked
						if(CHILDREN.stateaction[a][col]!='-')
							break;
						//if the car has not been blocked
						else
						{
							for(int b=0;b<V.length;b++)
							{
								V.positionxy[b][0]--;
							}
							CHILDREN.storeindisplay2(V);
							CHILDREN.V.remove(V);
							CHILDREN.V.push_back(V);
							IE.moves++;

							//to store the action into the state
							CHILDREN.IE.V=V.VehicleID;
							CHILDREN.IE.direction=IE.direction;
							CHILDREN.IE.moves=IE.moves;
							CHILDREN.IE.InEdgeID=IE.InEdgeID;

							CHILDREN.parent=new State;
							*CHILDREN.parent=PARENT;
							LS.push_back(CHILDREN);
						}
					}
				}
			}
			V=*S.itV;
			PARENT=S;
			CHILDREN=S;
			IE.moves=0;

			//assume the vehicle move to bottom
			IE.direction='B';
			if(IE.direction=='B')
			{
				//if the car is not the over of the bottom
				if(V.positionxy[V.length-1][0]!=5)
				{
					int row=V.positionxy[V.length-1][0];
					int col=V.positionxy[V.length-1][1];
					for(int a=row+1;a<=5;a++)
					{
						//if the car has been blocked
						if(CHILDREN.stateaction[a][col]!='-')
							break;
						//if the car has not been blocked
						else
						{
							for(int b=0;b<V.length;b++)
							{
								V.positionxy[b][0]++;
							}
							CHILDREN.storeindisplay2(V);
							CHILDREN.V.remove(V);
							CHILDREN.V.push_back(V);
							IE.moves++;

							//to store the action into the state
							CHILDREN.IE.V=V.VehicleID;
							CHILDREN.IE.direction=IE.direction;
							CHILDREN.IE.moves=IE.moves;
							CHILDREN.IE.InEdgeID=IE.InEdgeID;

							CHILDREN.parent=new State;
							*CHILDREN.parent=PARENT;
							LS.push_back(CHILDREN);
						}
					}
				}
			}
		}
	}
}

//to check the state weather is exist
bool CarParkSimulator::Exist(list<State>::iterator S, 
	list<State> &FL, list<State> &EL)
{
	list<State>::iterator itFL=FL.begin();

	if(FL.empty())
		return false;

	else
	{
		//to check the frontier list
		for(FL.begin();itFL!=FL.end();++itFL)
		{
			for(int a=0;a<6;a++)
			{
				for(int b=0;b<6;b++)
				{
					/*if the particular state is not same as 
					the state from frontier list*/
					if(itFL->stateaction[a][b]!=S->stateaction[a][b])
					{
						list<State>::iterator itEL=EL.begin();
						//to check the explore list
						for(EL.begin();itEL!=EL.end();++itEL)
						{
							for(int c=0;c<6;c++)
							{
								for(int d=0;d<6;d++)
								{
									/*if the particular state is not same as
									the state from frontier list*/
									if(itEL->stateaction[c][d]!=
										S->stateaction[c][d])
									{
										return false;
									}
								}
							}
							return true;
						}
					}
				}
			}
			return true;
		}
	}
}

void CarParkSimulator::BFS(State S)
{
	clock_t start=clock();
	list<State> FrontierList;
	list<State>::iterator itFrontierList;
	list<State> ExploreList;
	list<State>::iterator itExploreList;
	list<State> ChildrenList;
	list<State>::iterator itChildrenList;
	State FS;
	list<State> SolutionList;
	list<State>::iterator itSolutionList;

	bool flag=false;//to check weather it is the goal state

	FrontierList.push_back(S);

	cout<<"Please wait because ";
	cout<<"it needs some time to solve the game."<<endl<<endl;

	//if the state is not the goal state
	//then start the loop
	while(!flag)
	{
		itFrontierList=FrontierList.begin();
		IS=*itFrontierList;

		cout<<"Explore State "<<IS.StateID<<endl;
		IS.Display();

		//to push in the state into explore list 
		//before pop out state from the frontier list
		ExploreList.push_back(IS);
		FrontierList.pop_front();
		GetMoves(IS, ChildrenList);

		itChildrenList=ChildrenList.begin();

		for(ChildrenList.begin();itChildrenList!=ChildrenList.end();
			itChildrenList++)
		{
			//if the state is goal state,
			//then perform this condition
			//to store the environment after the vehicle 'X' is out
			if(IsGameOver(itChildrenList))
			{				
				FS.StateID=itChildrenList->StateID;
				FS.V=itChildrenList->V;
				FS.itV=itChildrenList->itV;
				FS.parent=new State;
				*FS.parent=*itChildrenList;

				for(int a=0;a<6;a++)
				{
					for(int b=0;b<6;b++)
					{
						FS.stateaction[a][b]=
							itChildrenList->stateaction[a][b];
					}
				}

				V=*FS.itV;
				FS.V.remove(V);
				for(int a=V.positionxy[0][1];a<=5;a++)
				{
					if('X'==FS.stateaction[2][a])
					{
						FS.stateaction[2][a]='-';
						IE.moves=6-a;
					}
				}
				FS.StateID=FS.StateID+2;

				branch=itChildrenList->IE.InEdgeID+1;

				IE.InEdgeID=branch;
				IE.V=V.VehicleID;
				IE.direction='R';
				
				FS.IE=IE;

				FS.parent->StateID++;

				flag=true;
				break;
			}
		}

		itChildrenList=ChildrenList.begin();

		//if the children list is not empty
		//then perfrom this loop to store into the frontier list
		while(!ChildrenList.empty())
		{
			itChildrenList=ChildrenList.begin();
			if(!Exist(itChildrenList, FrontierList, ExploreList))
			{
				IS=*itChildrenList;
				nodes++;
				IS.StateID=nodes;
				FrontierList.push_back(IS);
			}
			ChildrenList.pop_front();
		}
	}

	cout<<"GAME IS COMPLETED"<<endl<<endl;

	IS=FS;

	//to store the solution into the solution list
	while(IS.StateID!=1)
	{
		State PARENT;
		PARENT=*IS.parent;
		SolutionList.push_front(IS);
		IS=PARENT;
	}

	SolutionList.push_front(IS);

	itSolutionList=SolutionList.begin();

	//to display the solution and to store the solution into file
	ofstream outfile;
	outfile.open("solution.txt");
	outfile<<branch<<endl;
	for(SolutionList.begin();itSolutionList!=SolutionList.end();
		itSolutionList++)
	{
		IS=*itSolutionList;
		if(IS.StateID==1)
		{
			cout<<"initial state:"<<endl;
		}
		else
		{
			outfile<<IS.IE.V<<" ";
			outfile<<IS.IE.direction<<" "<<IS.IE.moves<<endl;
			cout<<"action "<<IS.IE.InEdgeID<<" = ";
			cout<<IS.IE.V<<" ";
			cout<<IS.IE.direction<<" "<<IS.IE.moves<<endl;
		}
		IS.Display();
	}
	outfile.close();

	sec=(clock()-start)/(double)CLOCKS_PER_SEC;

	cout<<"Max size of frontier and explored (space complexity) = ";
	cout<<ExploreList.size()+FrontierList.size()<<" nodes"<<endl;
	cout<<"Total nodes expanded (time complexity) = ";
	cout<<ExploreList.size()<<" nodes"<<endl;
	cout<<"Runtime = ";
	cout<<sec<<" seconds"<<endl;
}

#endif
