package packet

type WeaponState uint8

const (
	Idle         WeaponState = 0
	Fire         WeaponState = 1
	Reload       WeaponState = 2
	ChangeWeapon WeaponState = 3
)
