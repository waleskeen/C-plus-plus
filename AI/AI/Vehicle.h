//this header file is for vehicle class

#ifndef VEHICLE_TYPE
#define VEHICLE_TYPE

#include <cstring>

class Vehicle
{
public:
	char VehicleID;
	int positionxy[6][2];
	char position;
	int length;
	bool operator >= (Vehicle);
	bool operator == (Vehicle);
};

bool Vehicle::operator >=(Vehicle V2)
{
	if (VehicleID>=V2.VehicleID)
		return true;
	else
		return false;
}

bool Vehicle::operator ==(Vehicle V2)
{
	if (VehicleID==V2.VehicleID)
		return true;
	else
		return false;
}

#endif