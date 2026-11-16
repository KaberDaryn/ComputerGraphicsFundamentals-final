#pragma once

// Inner wall faces preserve the positions of all existing wall-mounted decor.
// Every shell edge derives from this one rectangle, including the corner join.
namespace RoomLayout {
inline constexpr float left=-4.65f, right=5.0f, back=-3.40f, front=4.50f;
inline constexpr float wallThickness=.14f, wallHeight=4.35f;
inline constexpr float floorY=-.015f, platformThickness=.16f;
inline constexpr float skirtDepth=.055f, skirtHeight=.17f;
inline constexpr float outerLeft=left-wallThickness;
inline constexpr float outerBack=back-wallThickness;
inline constexpr float width=right-outerLeft, depth=front-outerBack;
inline constexpr float centerX=(outerLeft+right)*.5f, centerZ=(outerBack+front)*.5f;
inline constexpr float wallTop=floorY+wallHeight;
}
