package main

import (
	"log"
	"math"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/shurcooL/eX0/eX0-go/packet"
	"github.com/shurcooL/go/gists/gist6545684"
)

const (
	PLAYER_HALF_WIDTH        = 7.74597
	PLAYER_COL_DET_TOLERANCE = 0.005
)

// It takes State #n and Command #n to produce State #n+1.
func (l *logic) nextState(state sequencedPlayerPosVel, move packet.Move) sequencedPlayerPosVel {
	newState := nextStateWithoutCollision(state, move)

	for nTries := 30; nTries > 0; nTries-- {
		// check for collision
		noCollision, oClosestPoint, oShortestDistance := colHandCheckPlayerPos(&l.level.polygon, newState.X, newState.Y)
		if noCollision {
			break
		}

		// Resolve collision by pushing away from nearest wall until no longer colliding.
		// TODO: Improve resolution with multiple walls at once.
		diff := mgl32.Vec2{newState.X - oClosestPoint.X(), newState.Y - oClosestPoint.Y()}
		newState.X += diff.X()*PLAYER_HALF_WIDTH/oShortestDistance - diff.X()
		newState.Y += diff.Y()*PLAYER_HALF_WIDTH/oShortestDistance - diff.Y()
	}

	newState.VelX = newState.X - state.X
	newState.VelY = newState.Y - state.Y

	return newState
}

func nextStateWithoutCollision(state sequencedPlayerPosVel, move packet.Move) sequencedPlayerPosVel {
	const TOP_SPEED = 3.5

	var TargetVel mgl32.Vec2

	if move.MoveDirection == -1 {
	} else if move.MoveDirection >= 0 && move.MoveDirection < 8 {
		direction := float64(move.Z) + Tau*float64(move.MoveDirection)/8
		speed := TOP_SPEED
		if move.Stealth != 0 {
			speed -= 2.25
		}

		TargetVel[0] = float32(math.Sin(direction) * speed)
		TargetVel[1] = float32(math.Cos(direction) * speed)
	} else {
		log.Printf("WARNING: Invalid nMoveDirection = %v!\n", move.MoveDirection)
	}

	var CurrentVel = mgl32.Vec2{state.VelX, state.VelY}
	var Delta = TargetVel.Sub(CurrentVel)
	if DeltaLength := float64(Delta.Len()); DeltaLength >= 0.001 {
		Delta = Delta.Normalize()

		var Move1 = DeltaLength * DeltaLength * 0.03
		var Move2 = math.Min(0.2, DeltaLength)

		CurrentVel = CurrentVel.Add(Delta.Mul(float32(math.Max(Move1, Move2))))
	}

	if commandRate == 1 {
		CurrentVel = TargetVel.Mul(20)
	}

	return sequencedPlayerPosVel{
		playerPosVel: playerPosVel{
			X:    state.X + CurrentVel[0],
			Y:    state.Y + CurrentVel[1],
			Z:    move.Z,
			VelX: CurrentVel[0],
			VelY: CurrentVel[1],
		},
		SequenceNumber: state.SequenceNumber + 1,
	}
}

// checks to see if player pos is ok, or inside a wall
func colHandCheckPlayerPos(polygon *gist6545684.Polygon, fX, fY float32) (noCollision bool, oClosestPoint mgl32.Vec2, oShortestDistance float32) {
	var oVector mgl32.Vec2
	var oSegment Seg2
	var oParam float32
	var oDistance float32

	oShortestDistance = math.MaxFloat32

	for iLoop1 := 0; iLoop1 < len(polygon.Contours); iLoop1++ {
		for iLoop2 := 1; iLoop2 < len(polygon.Contours[iLoop1].Vertices); iLoop2++ {

			oVector[0] = float32(polygon.Contours[iLoop1].Vertices[iLoop2-1][0])
			oVector[1] = float32(polygon.Contours[iLoop1].Vertices[iLoop2-1][1])
			oSegment.Origin = oVector
			oVector[0] = float32(polygon.Contours[iLoop1].Vertices[iLoop2][0]) - oVector[0]
			oVector[1] = float32(polygon.Contours[iLoop1].Vertices[iLoop2][1]) - oVector[1]
			oSegment.Direction = oVector
			oVector[0] = fX
			oVector[1] = fY

			// make sure the distance we're looking for is possible
			/*if !ColHandIsSegmentCloseToCircle(oVector[0], oVector[1], PLAYER_HALF_WIDTH + PLAYER_COL_DET_TOLERANCE, oSegment) {
				continue;
			}*/

			// Calculate the distance.
			oDistance = distance(oVector, oSegment, &oParam)

			if oDistance < PLAYER_HALF_WIDTH-PLAYER_COL_DET_TOLERANCE && oDistance < oShortestDistance {
				oShortestDistance = oDistance
				oClosestPoint = oSegment.Origin.Add(oSegment.Direction.Mul(oParam))
				/*
				* iWhichCont = iLoop1;
				* iWhichVert = iLoop2;
				 */
			}
		}

		// do the last segment
		oVector[0] = float32(polygon.Contours[iLoop1].Vertices[len(polygon.Contours[iLoop1].Vertices)-1][0])
		oVector[1] = float32(polygon.Contours[iLoop1].Vertices[len(polygon.Contours[iLoop1].Vertices)-1][1])
		oSegment.Origin = oVector
		oVector[0] = float32(polygon.Contours[iLoop1].Vertices[0][0]) - oVector[0]
		oVector[1] = float32(polygon.Contours[iLoop1].Vertices[0][1]) - oVector[1]
		oSegment.Direction = oVector
		oVector[0] = fX
		oVector[1] = fY

		// make sure the distance we're looking for is possible
		/*if !ColHandIsSegmentCloseToCircle(oVector[0], oVector[1], PLAYER_HALF_WIDTH + PLAYER_COL_DET_TOLERANCE, oSegment) {
			continue;
		}*/

		// Calculate the distance.
		oDistance = distance(oVector, oSegment, &oParam)

		if oDistance < PLAYER_HALF_WIDTH-PLAYER_COL_DET_TOLERANCE && oDistance < oShortestDistance {
			oShortestDistance = oDistance
			oClosestPoint = oSegment.Origin.Add(oSegment.Direction.Mul(oParam))
			/*
			*iWhichCont = iLoop1;
			*iWhichVert = 0;
			 */
		}
	}

	if oShortestDistance < PLAYER_HALF_WIDTH-PLAYER_COL_DET_TOLERANCE {
		return false, oClosestPoint, oShortestDistance
	} else {
		return true, mgl32.Vec2{}, 0
	}
}

// Seg2 represents a line segement with origin and direction vectors.
type Seg2 struct {
	Origin    mgl32.Vec2
	Direction mgl32.Vec2
}

// TODO: Calculating and doing math with squared length is faster.

func distance(point mgl32.Vec2, segment Seg2, param *float32) float32 {
	kDiff := point.Sub(segment.Origin)
	fT := kDiff.Dot(segment.Direction)

	if fT <= 0.0 {
		fT = 0.0
	} else {
		fLen := segment.Direction.Len()
		fSqrLen := fLen * fLen
		if fT >= fSqrLen {
			fT = 1.0
			kDiff = kDiff.Sub(segment.Direction)
		} else {
			fT /= fSqrLen
			kDiff = kDiff.Sub(segment.Direction.Mul(fT))
		}
	}

	if param != nil {
		*param = fT
	}

	return kDiff.Len()
}
