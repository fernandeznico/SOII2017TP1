/**
 * @author Fernández Nicolás (nicofernandez@alumnos.unc.edu.ar)
 * @date Abril, 2016
 * @version 0.5.2017 beta
 *
 * @brief Manejo de archivos: Lectura, búsqueda de caracteres, etc
 * 
 * \file File.h
 */

#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Mem.h"

/**
 * @brief Cantidad de caracteres en un archivo
 * 
 * @param file : Archivo a calcular el tamaño (por referencia)
 * @return Cantidad de caracteres del archivo incluidos todos o -1 ERROR
 */
int File_size( FILE * file )
{
	
	if( file == NULL )
		return -1;
	
	int position_backup = ftell( file );
	
	fseek( file , 0L , SEEK_END );
	int size = ftell( file );
	
	fseek( file , position_backup , SEEK_SET );
	
	return size;
	
}

/**
 * \brief Retorna la posición, relativa a la actual del archivo, en la
 * que se encuentra el siguiente caractere solicitado. No modifica la
 * posición del puntero
 * 
 * \param file Puntero al archivo en posición determinada
 * \param character Caractere a buscar
 * 
 * \return -1 en caso de error, 0 en caso de estar apuntando o valor
 * positivo en caso de ser uno de los siguientes
 */
int File_position_next_character( FILE * file , char character )
{
	
	if( file == NULL)
		return -1;
	
	fpos_t position_backup;
	
	fgetpos( file , &position_backup );
	
	int result = 0;
	char ch;
	
	while( ( ch = fgetc(file) ) != EOF )
	{
		
		if( ch == character )
		{
			
			fsetpos( file , &position_backup );
			
			return result;
			
		}
		
		result++;
		
	}
	
	fsetpos( file , &position_backup );
	
	return result * (-1) ;
	
}

/**
 * \brief Lleva el puntero de lectura a la posición en que se encuentra
 * el siguiente caractere 'c', más un valor especificado.\n
 * En caso de no encontrar, no hace nada.
 * 
 * \param file Puntero al archivo en posición determinada
 * \param c Caractere (simbolo ASCII) hasta el que se moverá
 * \param add Valor positivo, con el que se puede apuntar el siguiente
 */
void File_move_to_next_ocurrence_char
( FILE * file , char c , unsigned int add )
{
	
	int pos = File_position_next_character( file , c );
	
		if( pos == 0 )
			return;
	
	fseek( file , pos + add , SEEK_CUR );
	
}

/**
 * \brief Retorna una cadena de texto, conteniendo todos los caracteres
 * \b entre
 * : la posición actual del señalador del archivo y el caractere
 * especificado o el final del archivo 
 * 
 * \param file Puntero al archivo en posición determinada
 * \param c Caractere (simbolo ASCII) hasta el que se copiara
 */
char * File_read_until_next_ocurrence_char_FREE( FILE * file , char c )
{
	
	if( file == NULL )
		return NULL;
	
	int copy_size = File_position_next_character( file , c );
	
	if( copy_size < 0 )
	{
		
		copy_size = copy_size * -1;
		
		c = EOF;
		
	} 
	
	char * copy = (char *)Mem_assign( (copy_size + 1) * sizeof(char) );
	
	char ch;
	int pos = 0;
	while( ( ch = fgetc(file) ) != c )
	{
		
		copy[pos] = ch;
		
		pos++;
		
	}

	copy[pos] = '\0';
	
	
	return copy;
	
}

/**
 * \brief Retorna una cadena de texto, conteniendo todos los caracteres
 * \b entre: la posición actual del señalador del archivo y un salto
 * de línea o el fin del archivo ('\n')
 * 
 * \param file : Puntero al archivo en posición determinada
 */
char * File_read_current_line_FREE( FILE * file )
{
	
	return File_read_until_next_ocurrence_char_FREE( file , '\n' );
	
}

#endif
