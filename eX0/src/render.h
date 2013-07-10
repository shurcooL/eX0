// render the static scene
void RenderStaticScene(void);

// render the HUD
void RenderHUD(void);

// render all players
void RenderPlayers();

// render all particles
void RenderParticles();

// renders the interactive scene
void RenderInteractiveScene();

void RenderOffsetCamera(bool bLocalPlayerReferenceFrame);

// Renders the FOV zone
void RenderFOV();

// Creates the FOV mask
void RenderCreateFOVMask();

// Renders the FOV mask from a smoke grenade
void RenderSmokeFOVMask(Mgc::Vector2 oSmokePosition, float fSmokeRadius);
