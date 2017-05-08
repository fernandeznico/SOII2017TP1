#include <stdio.h>
#include <string.h>
#include <stdlib.h>		/* malloc */

#include "String.h"

int main()
{
	
	char * cadena = "juan,petes,burgo\nno,quiso";
	
	do
	{
		
		printf( "\n %s" , String_Cortar_hasta( &cadena , ",\n" ) );
		
	} while( cadena != NULL );
	
	
	
	printf( "\n" );
	
}
