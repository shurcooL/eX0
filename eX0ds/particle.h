#define PARTICLE_INITIAL_SIZE	8
#define PARTICLE_INCREMENT_SIZE	4
#define PARTICLE_TICK_TIME		PLAYER_TICK_TIME//(PLAYER_TICK_TIME * 10)

struct Particle_t
{
	// DEBUG: It might not be plausible to have a class (Vector2) inside,
	// since we'll be making an array of these structs...
	Vector2		oPosition;
	Vector2		oVelocity;
	int			iWhatType;
	float		fMaxDamage;
	float		fTicks;
	float		fLife;
	float		fDieAt;
	int			iWillHit;
	int			iOwnerID;
};

class CParticle
{
public:
	CParticle();
	~CParticle();

	void Tick();
	//void Render();
	//void RenderFOVMask();
	void CollisionHandling(int iParticle);
	void AddParticle(float fX, float fY, float fVelX, float fVelY, int iWhatType, float fMaxDamage, float fLife, int iOwnerID);
	void Reset();

	enum ParticleTypes { DUMMY = 0, BULLET, BOUNCY_BULLET, SMOKE_CLOUD };

private:
	int NextAvailParticle();
	void SetArraySize(int iSize);
	void GetInterpolatedPos(Vector2 *fIntPos, int iParticle);

	Particle_t*	oParticles;
	int			iNumParticles;
	//float		fParticleTicks;
};
