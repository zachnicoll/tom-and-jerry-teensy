/*
 *  CAB202 Teensy Library (cab202_teensy)
 *	macros.h
 *
 *	B.Talbot, September 2015
 *	L.Buckingham, September 2017
 *  Queensland University of Technology
 */
#ifndef MACROS_H_
#define MACROS_H_

/*
 *  Setting data directions in a data direction register (DDR)
 */
#define SET_INPUT(portddr, pin)			(portddr) &= ~(1 << (pin))
#define SET_OUTPUT(portddr, pin)		(portddr) |= (1 << (pin))

/*
 *  Setting, clearing, and reading bits in registers.
 *	reg is the name of a register; pin is the index (0..7)
 *  of the bit to set, clear or read.
 *  (WRITE_BIT is a combination of CLEAR_BIT & SET_BIT)
 */
#define SET_BIT(reg, pin)			(reg) |= (1 << (pin))
#define CLEAR_BIT(reg, pin)			(reg) &= ~(1 << (pin))
#define WRITE_BIT(reg, pin, value)	(reg) = (((reg) & ~(1 << (pin))) | ((value) << (pin)))
#define BIT_VALUE(reg, pin)			(((reg) >> (pin)) & 1)
#define BIT_IS_SET(reg, pin)		(BIT_VALUE((reg),(pin))==1)

/*
 *	Rudimentary math macros
 */
#define ABS(x) (((x) >= 0) ? (x) : -(x))
#define SIGN(x) (((x) > 0) - ((x) < 0))

#endif /* MACROS_H_ */
