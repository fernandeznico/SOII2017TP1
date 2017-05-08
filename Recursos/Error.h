/**
 * @author Fernández Nicolás (niferz@hotmail.com)
 * @date Abril, 2016
 *
 * @brief Impresión de errores y cerrado del programa
 * 
 * \file Error.h
 */

#ifndef ERROR_H
#define ERROR_H

#define SI 1
#define NO 0

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Interpreta valores negativos como la ocurrencia de un error,
 * imprime el mensaje de error contenido en stderr y cierra el programa
 * o no según se especifica; para valores no negativos no hace nada
 * \a Motivo: Permite formatear la impresion de errores por \c perror
 * 
 * @param error: valor a verificar
 * @param cerrar: 1 para cerrar el programa en caso de valor negativo
 * en 'error' y 0 para no
 * @return 1 para valores negativos o de lo contrario 0; para valores
 * negativos de 'error' y 1 en 'cerrar' cierra el programa
 * @bug Se debería simplemente testear si el stderr tiene contenido.
 * Modificar nombre a: \c "Error_imprimir_stderr_cerrar_si_int_negativo
 */
int Error_int( int error , int cerrar )
{
	
	if( error < 0 )
	{
		
		perror( " " );
		
		if( cerrar == SI )
		{
			
			exit(1);
			
		}
		else
		{
			
			return 1 ;
			
		}
		
	}
	
	return 0;
	
}

/**
 * @see \c Error_int
 * @bug Modificar nombre a: \c "Error_imprimir_stderr_cerrar_si_*_null
 */
int Error_pnt( void * error , int cerrar )
{
	
	if( error == NULL )
	{
		
		return Error_int( -1 , cerrar );
		
	}
	
	return 0;
	
}

#endif
