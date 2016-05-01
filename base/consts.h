#ifndef CONSTS_H
#define CONSTS_H

#define SUCCESS	1
#define FAILURE 0

typedef enum _Segment_Type {
	LINE_SEGMENT,
	CURVE_SEGMENT
} Segment_Type;

typedef enum _Compound_Comp_Flags {
	/**
	 * If set, the arguments are words; If not set, they are bytes.
	 */
	ARG_1_AND_2_ARE_WORDS		=	(1 << 0),
	/**
	 * If set, the arguments are xy values; If not set, they are points.
	 */
	ARGS_ARE_XY_VALUES			=	(1 << 1),
	/**
	 * If set, round the xy values to grid;
	 * If not set do not round xy values to grid (relevant only to bit 1 is set)
	 */
	ROUND_XY_TO_GRID			=	(1 << 2),
	/**
	 * If set, there is a simple scale for the component. If not set, scale is 1.0.
	 */
	WE_HAVE_A_SCALE				=	(1 << 3),
	/**
	 * If set, at least one additional glyph follows this one.
	 */
	MORE_COMPONENTS				=	(1 << 5),
	/**
	 * If set the x direction will use a different scale than the y direction.
	 */
	WE_HAVE_AN_X_AND_Y_VALUE	=	(1 << 6),
	/**
	 * If set there is a 2-by-2 transformation that will be used to scale the component.
	 */
	WE_HAVE_A_TWO_BY_TWO		=	(1 << 7),
	/**
	 * If set, instructions for the component character follow the last component.
	 */
	WE_HAVE_INSTRUCTIONS		=	(1 << 8),
	/**
	 * Use metrics from this component for the compound glyph.
	 */
	USE_MY_METRICS				=	(1 << 9),
	/**
	 * If set, the components of this compound glyph overlap.
	 */
	OVERLAP_COMPOUND			=	(1 << 10),
} Compound_Comp_Flags;

typedef enum _Coord_Flags {
	/**
	 * If set, the point is on the curve;
	 * Otherwise, it is off the curve.
	 */
	ON_CURVE =			(1 << 0),
	/**
	 * If set, the corresponding x-coordinate is 1 byte long;
	 * Otherwise, the corresponding x-coordinate is 2 bytes long
	 */
	X_SHORT_VECTOR =	(1 << 1),
	/**
	 * If set, the corresponding y-coordinate is 1 byte long;
	 * Otherwise, the corresponding y-coordinate is 2 bytes long
	 */
	Y_SHORT_VECTOR =	(1 << 2),
	/**
	 * If set, the next byte specifies the number of additional
	 * times this set of flags is to be repeated. In this way, the number
	 * of flags listed can be smaller than the number of points in a character.
	 */
	REPEAT =			(1 << 3),
	/**
	 * This flag has one of two meanings, depending on how the
	 * x-Short Vector flag is set.
	 * If the x-Short Vector bit is set, this bit describes the sign
	 * of the value, with a value of 1 equaling positive and a zero value negative.
	 *
	 * If the x-short Vector bit is not set, and this bit is set,
	 * then the current x-coordinate is the same as the previous x-coordinate.
	 *
	 * If the x-short Vector bit is not set, and this bit is not set,
	 * the current x-coordinate is a signed 16-bit delta vector.
	 * In this case, the delta vector is the change in x
	 */
	X_DUAL =			(1 << 4),
	/**
	 * This flag has one of two meanings, depending on how the
	 * y-Short Vector flag is set.
	 *
	 * If the y-Short Vector bit is set, this bit describes the sign
	 * of the value, with a value of 1 equalling positive and a zero value negative.
	 *
	 * If the y-short Vector bit is not set, and this bit is set,
	 * then the current y-coordinate is the same as the previous y-coordinate.
	 *
	 * If the y-short Vector bit is not set, and this bit is not set,
	 * the current y-coordinate is a signed 16-bit delta vector.
	 * In this case, the delta vector is the change in y
	 */
	Y_DUAL =			(1 << 5),
} Coord_Flags;

/**
 * loca table offset types.
 */
typedef enum _Offset_Type {
	SHORT_OFFSETS =	0,
	LONG_OFFSETS =	1
} Offset_Type;

/**
 * TTF_Table possible statuses.
 */
typedef enum _Table_Status {
	STATUS_NONE,
	STATUS_LOADED,
	STATUS_FREED
} Table_Status;

#define TAG_LENGTH	4

#endif /* CONSTS_H */
