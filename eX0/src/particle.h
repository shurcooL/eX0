#pragma once
#ifndef __Particle_H__
#define __Particle_H__

#define PARTICLE_INITIAL_SIZE	8
#define PARTICLE_INCREMENT_SIZE	4
#define PARTICLE_TICK_TIME		0.050

struct Particle_st
{
	// DEBUG: It might not be plausible to have a class (Vector2) inside,
	// since we'll be making an array of these structs...
	Vector2		oPosition;
	Vector2		oVelocity;
	float		fAcceleration;		// TODO: This is a constant for particle type, doesn't belong here
	int			iWhatType;
	float		fMaxDamage;
	double		dTime;			// The time the particle is currently at (oPosition), next till will happen PARTICLE_TICK_TIME seconds later
	double		dDieAt;			// The time at which the particle will die
	uint8		iWillHit;
	Vector2		oHitNormal;
	uint8		iOwnerID;
	float		fRenderTimeOffset;
};

class CParticle
{
public:
	CParticle();
	~CParticle();

	void Tick();
	void Render();
	void RenderFOVMask();
	void AddParticle(double dTimeTEST, float fX, float fY, float fVelX, float fVelY, int iWhatType, float fMaxDamage, double dLife, uint8 iOwnerID);
	void Reset();

	enum ParticleTypes { DUMMY = 0, BULLET, BOUNCY_BULLET, SMOKE_CLOUD, WALL_HIT_SPARK, BLOOD_SPLATTER };

private:
	int NextAvailParticle();
	void SetArraySize(int iSize);
	void GetInterpolatedPos(Vector2 *fIntPos, int iParticle);

	void HitCheck(int nParticle);
	void CollisionHandling(int iParticle);

	Particle_st	*	oParticles;
	int				iNumParticles;
	//float			fParticleTicks;
};

#endif // __Particle_H__
