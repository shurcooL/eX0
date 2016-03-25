package gpc_test

import (
	"path/filepath"

	"github.com/shurcooL/eX0/eX0-go/gpc"
	"github.com/shurcooL/go-goon"
)

func Example() {
	polygon, err := gpc.ParseFile(filepath.Join("testdata", "test_orientation.wwl"))
	if err != nil {
		panic(err)
	}

	goon.Dump(polygon)

	// Output:
	// (gpc.Polygon)(gpc.Polygon{
	// 	Contours: ([]gpc.Contour)([]gpc.Contour{
	// 		(gpc.Contour)(gpc.Contour{
	// 			Vertices: ([]mgl64.Vec2)([]mgl64.Vec2{
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-210),
	// 					(float64)(-210),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(210),
	// 					(float64)(-210),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(210),
	// 					(float64)(210),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-210),
	// 					(float64)(210),
	// 				}),
	// 			}),
	// 		}),
	// 		(gpc.Contour)(gpc.Contour{
	// 			Vertices: ([]mgl64.Vec2)([]mgl64.Vec2{
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(180),
	// 					(float64)(180),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(180),
	// 					(float64)(120),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(120),
	// 					(float64)(180),
	// 				}),
	// 			}),
	// 		}),
	// 		(gpc.Contour)(gpc.Contour{
	// 			Vertices: ([]mgl64.Vec2)([]mgl64.Vec2{
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(0),
	// 					(float64)(180),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-15),
	// 					(float64)(150),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(15),
	// 					(float64)(150),
	// 				}),
	// 			}),
	// 		}),
	// 		(gpc.Contour)(gpc.Contour{
	// 			Vertices: ([]mgl64.Vec2)([]mgl64.Vec2{
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-150),
	// 					(float64)(150),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-134),
	// 					(float64)(150),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-134),
	// 					(float64)(166),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-150),
	// 					(float64)(166),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-150),
	// 					(float64)(150),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-166),
	// 					(float64)(150),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-166),
	// 					(float64)(134),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-150),
	// 					(float64)(134),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-150),
	// 					(float64)(118),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-134),
	// 					(float64)(118),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-134),
	// 					(float64)(134),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-150),
	// 					(float64)(134),
	// 				}),
	// 				(mgl64.Vec2)(mgl64.Vec2{
	// 					(float64)(-150),
	// 					(float64)(150),
	// 				}),
	// 			}),
	// 		}),
	// 	}),
	// })
}
