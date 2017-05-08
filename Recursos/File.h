/**
 * @author Fernández Nicolás (niferz@hotmail.com)
 * @date Abril, 2016
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
 * \param file : Puntero al archivo en posición determinada
 * \param character : Caractere a buscar
 */
int File_position_next_character( FILE * file , char character )
{
	
	if( file == NULL)
		return -1;
	
	fpos_t position;
	
	fgetpos( file , &position );
	
	int result = 0;
	char ch;
	
	while( ( ch = fgetc(file) ) != EOF )
	{
		
		if( ch == character )
		{
			
			fsetpos( file , &position );
			
			return result;
			
		}
		
		result++;
		
	}
	
	fsetpos( file , &position );
	
	return result * (-1) ;
	
}

/// @todo !!! Posible problema con \b
/**
 * \brief Retorna una cadena de texto, conteniendo todos los caracteres
 * \b entre:
 * la posición actual del señalador del archivo y el caractere
 * especificado o el final del archivo 
 * 
 * \param file : Puntero al archivo en posición determinada
 * \param caractere : Caractere (simbolo ASCII) hasta el que se copiara
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
	
	char * copy = malloc( copy_size + 1 );
	
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
