//this header file is for inedge class to represent the action of the state

#ifndef INEDGE_TYPE
#define INEDGE_TYPE

using namespace std;

class InEdge
{
public:
	int InEdgeID;
	char V;
	char direction;
	int moves;
	InEdge();
};

//Constructor
InEdge::InEdge()
{
	InEdgeID=0;
	moves=0;
}

#endif