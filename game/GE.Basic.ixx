export module GE.Basic;

export import :Types;
export import :FixedPoint;
export import :FixedPoint_InterestingMath;

export namespace GE {
enum MouseButtonBits {
	MouseButtonBits_PrimaryPressed,
	MouseButtonBits_SecondaryPressed,
	MouseButtonBits_PrimaryReleased,
	MouseButtonBits_SecondaryReleased,
	MouseButtonBits_PrimaryDown,
	MouseButtonBits_SecondaryDown
} typedef MouseButtonBits;

}