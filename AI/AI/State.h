//thie header file is for the state class

#ifndef STATE_TYPE
#define STATE_TYPE

#include "Vehicle.h"
#include "InEdge.h"
#include <cstring>
#include <list>

using namespace std;

class State
{
public:
	int StateID;
	State *parent;
	InEdge IE;
	list<Vehicle> V;
	list<Vehicle>::iterator itV;
	void storeindisplay();
	void storeindisplay2(Vehicle);
	void Display();
	char stateaction[6][6];
	bool operator >= (State);
	bool operator == (State);
	State();
};

//Constructor
State::State()
{
	StateID=0;
}

//to store all of vehicle inside the array
void State::storeindisplay()
{
	for(int a=0;a<6;a++)
	{
		for(int b=0;b<6;b++)
			stateaction[a][b]='-';
	}
	itV=V.begin();
	for (V.begin(); itV != V.end(); itV++)
	{
		for(int a=0;a<itV->length;a++)
		{
			for(int b=0;b<6;b++)
			{
				for(int c=0;c<6;c++)
				{
					if(b==itV->positionxy[a][0]
					&& c==itV->positionxy[a][1])
						stateaction[b][c]=itV->VehicleID;
				}
			}
		}
	}
}

//to display environment from the array of storeindisplay
void State::Display()
{
	for(int a=0;a<6;a++)
	{
		for(int b=0;b<6;b++)
		{
			cout<<stateaction[a][b]<<' ';
		}
		cout<<endl;
	}
	cout<<endl;
}

void State::storeindisplay2(Vehicle V2)
{
	for(int a=0;a<6;a++)
	{
		for(int b=0;b<6;b++)
		{
			if(stateaction[a][b]==V2.VehicleID)
				stateaction[a][b]='-';
		}

	}
	
	for(int a=0;a<6;a++)
	{
		for(int b=0;b<6;b++)
		{
			for(int c=0;c<6;c++)
			{
				if(b==V2.positionxy[a][0] && c==V2.positionxy[a][1])
					stateaction[b][c]=V2.VehicleID;
			}
		}
	}
}

bool State::operator>=(State S2)
{
	if (StateID>=S2.StateID)
		return true;
	else
		return false;
}

bool State::operator==(State S2)
{
	if (StateID==S2.StateID)
		return true;
	else
		return false;
}

#endif