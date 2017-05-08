/**
 * @author Fernández Nicolás (niferz@hotmail.com)
 * @date Abril, 2016
 *
 * @brief Manejo de cadenas: Lectura, búsqueda de caracteres, etc
 * 
 * \file String.h
 */

#ifndef STRING_H
#define STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * String_Flotante_a_cadena_FREE( float numero )
{
	
	char numero_str[50];
	memset( numero_str , '\0' , 50 );
	sprintf( numero_str , "%f" , numero );
	
	char * retorno = malloc( strlen( numero_str ) );
	strcpy( retorno , numero_str );
	
	return retorno;
	
}

char * String_Entero_a_cadena_FREE( int numero )
{
	
	unsigned int digitos = 0;
	if( numero == 0 )
		digitos = 1;
	else
	{
		
		int numero_copia = numero;
		while( (numero_copia /= 10) != 0 )
			digitos++;
		
	}
	
	char * cadena = malloc( digitos );
	
	sprintf( cadena , "%i" , numero );
	
	return cadena;
	
}

/**
 * @brief Cuenta la cantidad de columnas en una cadena separadas por
 * un conjunto de caracteres determinados
 * 
 * @param cadena : en la cual buscar
 * @param separadores : conjunto de caracteres que separan
 * @return cantidad de columnas
 * 
 * @bug separadores continuos cuentan columas vacias
 */
unsigned int
String_Cantidad_de_columnas( char * cadena , char * separadores )
{
	
	if( cadena == NULL || separadores == NULL )
		return 0;
	
	unsigned int cantidad = 0;
	
	unsigned int nro_sep = strlen( separadores );
	char * puntero;
	unsigned int separador_nro;
	
	for( puntero = &cadena[0] ; puntero[0] != '\0' ; puntero++ )
	{
		
		for( separador_nro = 0 ; separador_nro < nro_sep ; separador_nro++ )
		{
			
			if( puntero[0] == separadores[separador_nro] )
				cantidad++;
			
		}
		
	}
	
	///Si empieza con un separador, resto uno
	for( separador_nro = 0 ; separador_nro < nro_sep ; separador_nro++ )
		if( cadena[0] == separadores[separador_nro] )
			cantidad--;
	
	///Si termina no termina con un separador, cuento una columna mas
	puntero--;
	for( separador_nro = 0 ; separador_nro < nro_sep ; separador_nro++ )
		if( puntero[0] != separadores[separador_nro] )
			return cantidad + 1;
	
	return cantidad;
	
}

/**
 * @brief da la posicion en la que se ecuentra el siguiente caracter
 * dentro de un conjunto
 * 
 * @param cadena : a buscar
 * @param caracteres : conjunto que buscar
 * @return posicion de la primer ocurrencia o -1 si no encuentra
 */
int String_Posicion_siguiente_char( char * cadena , char * caracteres )
{
	
	if( cadena == NULL )
		return -1;

	int tam_cadena = strlen( cadena );
	int cant_car = strlen( caracteres );
	int pos_cadena;
	int caractere_nro;
	for( pos_cadena = 0 ; pos_cadena < tam_cadena ; pos_cadena++ )
		for( caractere_nro = 0 ; caractere_nro < cant_car ; caractere_nro++ )
			if( cadena[pos_cadena] == caracteres[caractere_nro] )
				return pos_cadena;
	
	return -1;
	
}

char * String_copiar_n_FREE( char * cadena ,  int n )
{
	
	if( n <= 0 )
		return "";
	
	int tam_cadena = strlen(cadena);
	if( n >= tam_cadena )
	{
		
		char * copia = malloc( tam_cadena );
		strcpy( copia , cadena );
		return copia;
		
	}
	
	char * copia = malloc( n + 1 );
	copia[n] = '\0';
	int pos;
	for( pos = 0 ; pos < n ; pos++)
		copia[pos] = cadena[pos];
	
	return copia;
	
}

/**
 * @brief retorna una copia de la cedena hasta un caracter en un
 * conjnto dado o el fin de la cadena y cambia el puntero de la cadena
 * a la posicion siguiente al caracter o null si es fin de cadena
 * 
 * @param cadena : a cortar
 * @param caracteres : conjunto de caracteres que limitan la copia
 * @return copia de la cadena desde el inicio hasta un caracter
 */
char * String_Cortar_hasta_FREE( char ** cadena , char * caracteres )
{
	
	if( cadena == NULL )
		return NULL;

	int pos;
	pos = String_Posicion_siguiente_char( *cadena , caracteres );
	
	if( pos == -1 )
	{
		
		char * copia = malloc( strlen( *cadena ) );
		strcpy( copia , *cadena );
		*cadena = NULL;
		return copia;
		
	}
	
	char * copia = String_copiar_n_FREE( *cadena , pos );
	*cadena = *cadena + pos + 1;
	return copia;
	
}

#endif
