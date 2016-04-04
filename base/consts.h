#ifndef CONSTS_H
#define CONSTS_H

typedef enum _Segment_Type {
	LINE,
	CURVE
} Segment_Type;

typedef enum _Coord_Flags {
	/**
	 * If set, the point is on the curve;
	 * Otherwise, it is off the curve.
	 */
	ON_CURVE =			1 << 0,
	/**
	 * If set, the corresponding x-coordinate is 1 byte long;
	 * Otherwise, the corresponding x-coordinate is 2 bytes long
	 */
	X_SHORT_VECTOR =	1 << 1,
	/**
	 * If set, the corresponding y-coordinate is 1 byte long;
	 * Otherwise, the corresponding y-coordinate is 2 bytes long
	 */
	Y_SHORT_VECTOR =	1 << 2,
	/**
	 * If set, the next byte specifies the number of additional
	 * times this set of flags is to be repeated. In this way, the number
	 * of flags listed can be smaller than the number of points in a character.
	 */
	REPEAT =			1 << 3,
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
	X_DUAL =			1 << 4,
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
	Y_DUAL =			1 << 5,
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
