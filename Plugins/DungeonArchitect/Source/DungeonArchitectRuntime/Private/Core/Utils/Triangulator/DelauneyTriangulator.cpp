//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "DelauneyTriangulator.h"

void DelauneyTriangulator::AddPoint(const FVector2D& Point)
{
	Points.Add(Point);
}

void DelauneyTriangulator::Triangulate()
{
	PerformTriangulation();
}
