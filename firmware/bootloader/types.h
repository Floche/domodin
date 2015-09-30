/*** FILEHEADER ****************************************************************
 *
 *	FILENAME:    types.h
 *	DATE:        21.11.2004
 *	AUTHOR:		 Christian Stadler
 *
 *	DESCRIPTION: Definition of common types.
 *
 ******************************************************************************/

/*** HISTORY OF CHANGE *********************************************************
 *
 *	$Log: /pic/_inc/types.h $
 * 
 * 3     1.11.10 11:42 Stadler
 * - type definitons dependend on compiler
 * 
 * 2     26.02.05 18:44 Stadler
 * added bool type
 * 
 ******************************************************************************/

#ifndef _TYPES_H
#define _TYPES_H

/*---------------------------------------------------------------------------*/
/* Type definitions for Microchip C18 Compiler                               */
/*---------------------------------------------------------------------------*/
#if defined(__18CXX)

typedef unsigned char bool;
typedef signed char sint8;
typedef signed int sint16;
typedef signed long sint32;
typedef unsigned char  uint8;
typedef unsigned int uint16;
typedef unsigned long uint32;

#endif /* #if defined(__18CXX) */



/*---------------------------------------------------------------------------*/
/* Type definitions for CCS C Compiler                                       */
/*---------------------------------------------------------------------------*/
#if (defined(__PCB__) || defined(__PCH__) || defined(__PCM__))

typedef int1 bool;
typedef signed int8 sint8;
typedef signed int16 sint16;
typedef signed int32 sint32;
typedef unsigned int8  uint8;
typedef unsigned int16 uint16;
typedef unsigned int32 uint32;

#endif /* (defined(__PCB__) || defined(__PCH__) || defined(__PCM__)) */

#endif /*_TYPES_H */
