#pragma once


namespace ChronoShift
{
	/*
	Just runs the physics system.
	Needs: 
	> Rigidbody component on the entity
	> Position, Scale
	> Only supports BoundingBox2D "AABB collider" right now 
	
	- Updates positions based on rigidbody.velocity
	- Finds and resolves collisions
		- No elasticity or anything, just pushes back the 
			minimum amount to avoid overlap

	Note:
	Positions are still according to FlexEngine's top left (0,0)
	so our max min will be topleft botright
	*/
	void UpdatePhysicsSystem();
}