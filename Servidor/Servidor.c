/**
 * @author Fernández Nicolás (niferz@hotmail.com)
 * @date Abril, 2016
 * @version 1.0 beta
 *
 * @brief Servidor AWS que provee los datos de telemetría correspondientes
 * (de la estación en la que se ejecuta) a los distintos clientes que
 * la soliciten
 * 
 * \file Servidor.c
 */

#include "../Recursos/File.h"
#include "../Recursos/Sockets.h"
#include "../Recursos/Error.h"
#include "../Recursos/String.h"

#include <ifaddrs.h>

/**
 * @brief Obtiene las diferentes ip de los dispositivos del equipo y las
 * publica junto con el puerto de comunicación utilizado
 * 
 * @param puerto: Número de puerto para la conexión
 * @return -1 si hubo fallo 0 si no.
 */
int Imprimir_conexiones_disponibles( int puerto );

/**
 * @brief Se pone a la espera de un cliente y en caso de uno acepta la
 * conexión y crea la conexión UDP para paso de archivos
 * 
 * @param servidor: file descriptor del socket que recibe conexiones TCP
 * @param conexion: file descriptor de la conexión con el cliente
 * @param conexion: file descriptor de la conexión UDP
 * @param addrUDP: para guardar los datos de dirección referidos al cliente
 * @return 1 si falló, de lo contrario 0
 */
int Cliente( int , int * , int * , struct sockaddr_in * );

/**
 * @brief Interpreta el comando ingresado por el usuario
 *
 * @param comando : Cadena que contiene el comando a parsear y comparar
 * @param sockfdUDP : socket de la conexion UDP en caso de transferencia
 * @param addrUDP : datos de la conexion UDP en caso de transferencia
 *
 * @return respuesta al comando recibido para enviar al cliente
 */
char * Comando_FREE( char comando[] , int sockfdUDP , struct sockaddr_in addrUDP );

/**
 * @brief Carga los nombres de las comlumnas de la bd en un vector
 * de cadenas
 *
 * @param cantidad_de_columnas : tamanio referencia para la matriz
 * creada
 *
 * @return Nombres de columnas separados en un vector de cadenas
 */
char ** Cabecera_FREE( unsigned int * cantidad_de_columnas );

int main( int argc , char **argv )
{

	int servidor = 0;
	int puerto = 6020;

	Error_int( Sockets_Crear_Socket_INET_TCP( &servidor , &puerto ) ,
			   SI );
	Error_int( Imprimir_conexiones_disponibles( puerto ) , NO );

	char mensaje_enviar[TAM];
	memset( mensaje_enviar , '\0' , TAM );

	while ( 1 )
	{

		int conexion, sockfdUDP;
		struct sockaddr_in addrUDP;

		while( Cliente( servidor , &conexion , &sockfdUDP , &addrUDP ) );

		int contrasenia = 1;
		Sockets_Enviar_mensaje_TCP( conexion , "Clave=" );

		char mensaje_leer[TAM];
		do
		{

			int error = Sockets_Leer_mensaje_TCP( conexion ,
												  mensaje_leer ,
												  TAM );
			if( Error_int( error , NO ) )
				break;

			if( contrasenia )
			{

				contrasenia = 0;
				if( strcmp( mensaje_leer , "root" ) == 0 )
				{

					strcpy( mensaje_enviar ,
						   "Clave verificada con exito\n\n"
						   " · Comandos disponibles:\n\n"
						   "\t- listar: muestra un listado de todas las"
						   " estaciones que hay en la “base de datos”"
						   ", y muestra de que censores tiene datos.\n"
						   "\t- descargar no_estación: descarga un arch"
						   "ivo con todos los datos de no_estación.\n"
						   "\t- diario_precipitacion no_estación: muest"
						   "ra el acumulado diario de la variable prec"
						   "ipitación de no_estación (no_día: acumnula"
						   "do mm).\n"
						   "\t- mensual_precipitacion no_estación: mues"
						   "tra el acumulado mensual de la variable pr"
						   "ecipitación (no_día: acumnulado mm).\n"
						   "\t- promedio variable: muestra el promedio "
						   "de todas las muestras de la variable de ca"
						   "da estación (no_estacion: promedio.\n"
						   "\t- desconectar: termina la sesión del usua"
						   "rio.\1n" );

				}
				else
				{

					strcpy( mensaje_enviar , "Clave incorrecta" );
					strcpy( mensaje_leer , "desconectar" );

				}

			}
			else
			{

				char * cmd_re;
				cmd_re = Comando_FREE( mensaje_leer , sockfdUDP , addrUDP );
				strcpy( mensaje_enviar , cmd_re );

			}

			error = Sockets_Enviar_mensaje_largo_TCP( conexion ,
													  mensaje_enviar );
			if( Error_int( error , NO ) )
				break;

		} while( strcmp( mensaje_leer , "desconectar" ) != 0 );

		shutdown( conexion , 2 );

	}

	close( servidor );

	return EXIT_SUCCESS;

}

int Imprimir_conexiones_disponibles( int puerto )
{

	struct ifaddrs * ifaddr , * ifa;
	int s, n;
	char host[NI_MAXHOST];

	if( getifaddrs( &ifaddr ) == -1 )
	{

		fprintf( stderr ,
				"ERROR: No se encuentran dispositivos de red \
				(getifaddrs)" );
		return -1;

	}

	for( ifa = ifaddr , n = 0 ; ifa != NULL ; ifa = ifa->ifa_next , n++ )
	{

		if( ifa->ifa_addr == NULL )
			continue;

		if( ifa->ifa_addr->sa_family != AF_INET )
			continue;

		s = getnameinfo( ifa->ifa_addr ,
						 sizeof(struct sockaddr_in) ,
						 host ,
						 NI_MAXHOST ,
						 NULL ,
						 0 ,
						 NI_NUMERICHOST );
			if (s != 0) {
				fprintf( stderr ,
						"ERROR: No se pudo leer dispositivo de red \
							(getnameinfo: %s )" ,
						 gai_strerror(s) );
				return -1;
			}

		printf("\n %-8s\t%s:%d", ifa->ifa_name , host , puerto);

	}

	freeifaddrs(ifaddr);

	return 0;

}

int Cliente( servidor , conexion , sockfdUDP , addrUDP )
int servidor ;
int * conexion ;
int * sockfdUDP ;
struct sockaddr_in * addrUDP ;
{

	///Acepto un cliente:
	struct direccion dir;
	struct sockaddr_in cli_addr;
	printf("\n Servidor disponible y a la espera de una conexión.");
	fflush( stdout );
	if( Error_int( Sockets_Aceptar_un_cliente( conexion ,
											   servidor ,
											   cli_addr ,
											  &dir ) ,
				   NO ) )
	{

		close( *conexion );
		return 1;

	}
	printf( "\n Conectado a: %s:%d " , dir.ip , dir.puerto );
	struct direccion mi_dir;
	Sockets_Direccion_del_socket( *conexion , &mi_dir );
	printf( "\n Desde: %s:%d " , mi_dir.ip , mi_dir.puerto );
	///Recibo el puerto del socket UDP:
	char msj_leer[TAM];
	Error_int( Sockets_Leer_mensaje_TCP( *conexion , msj_leer , TAM ) , SI );
	char puerto_UDP_str[ strlen(msj_leer) ];
	strcpy( puerto_UDP_str , msj_leer );
	int puerto_UDP = atoi	( puerto_UDP_str );
	///Conecto por UDP:
	struct hostent * servidorUDP = Sockets_Verificar_host_IPv4( dir.ip );
	if( Error_pnt( servidorUDP , NO ) )
	{

		close( *conexion );
		return 1;

	}
	*sockfdUDP = Sockets_Crear_Y_Conectar_Socket_INET_UDP( servidorUDP ,
														   addrUDP ,
														   puerto_UDP );
	if( Error_int( *sockfdUDP , NO ) )
	{

		close( *conexion );
		return 1;

	}
	Sockets_Direccion_del_sockaddr_in( *addrUDP , &dir );
	printf( "\n UDP: %s:%d " , dir.ip , dir.puerto );
	fflush( stdout );

	return 0;

}

unsigned int Cantidad_de_simbolos( char * cadena )
{
	
	unsigned int cantidad = 0;
	
	int pos;
	for( pos = 0 ; pos < strlen( cadena ) ; pos++ )
	{
		
		if( cadena[pos] < 0 )
			cantidad++;
		
	}
	
	return cantidad;
	
}

char * Corregir_simbolos_FREE( char * cadena )
{
	
	int correcciones = Cantidad_de_simbolos( cadena );
	
	char * cadena_corregida;
	cadena_corregida = malloc( strlen( cadena ) + correcciones );
	
	correcciones = 0;
	
	int pos;
	for( pos = 0 ; pos < strlen(cadena) ; pos++ )
	{
		
		if( cadena[pos] < 0 )
			correcciones++;
		
		switch( cadena[pos] )
		{
			
			case -70:
				strcat( cadena_corregida , "º" );
				break;
			case -13:
				strcat( cadena_corregida , "ó" );
				break;
			case -19:
				strcat( cadena_corregida , "í" );
				break;
			case 13:
				cadena_corregida[pos + correcciones] = '\n';
				break;
			default:
				cadena_corregida[pos + correcciones] = cadena[pos];
			
		}
		
	}
	
	cadena_corregida[strlen( cadena_corregida )] = '\0';
	
	return cadena_corregida;
	
}

char ** Cabecera_FREE( unsigned int * cantidad_de_columnas )
{
	
	FILE * bd = fopen( "datos_meteorologicos.CSV" , "r" );
		if( bd == NULL ){
			return NULL;
		}
	
	///Asigno memoria al vector de cadenas
	free( File_read_until_next_ocurrence_char_FREE( bd , 13 ) );
	free( File_read_until_next_ocurrence_char_FREE( bd , 13 ) );
	char * linea3 = File_read_until_next_ocurrence_char_FREE( bd , 13 );
	char * linea3_mem = linea3;
	linea3 = Corregir_simbolos_FREE( linea3 );
	free( linea3_mem );
	linea3_mem = linea3;
	*cantidad_de_columnas = String_Cantidad_de_columnas( linea3 , "," );
	char ** cabecera;
	cabecera = (char **)malloc( *cantidad_de_columnas * sizeof(char *) );
	
	unsigned int pos_cab;
	for( pos_cab = 0 ; pos_cab < *cantidad_de_columnas ; pos_cab++ )
		cabecera[pos_cab] = String_Cortar_hasta_FREE( &linea3 , "," );
	
	free( linea3_mem );
	fclose( bd );
	
	return cabecera;
	
}

unsigned int Cantidad_de_estaciones()
{
	
	FILE * bd = fopen( "datos_meteorologicos.CSV" , "r" );
		if( bd == NULL ){
			return 0;
		}
	
	unsigned int cantidad = 0;
	
	///Cargo la linea 4 (primera con datos)
	///@todo reemplazar por mover en vez de leer para mejor
	///manejo de memoria
	free( File_read_until_next_ocurrence_char_FREE( bd , 13 ) );
	free( File_read_until_next_ocurrence_char_FREE( bd , 13 ) );
	free( File_read_until_next_ocurrence_char_FREE( bd , 13 ) );
	char * linea;
	char * linea_cortada;
	linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
	linea_cortada = linea;
	char ultimo_nro_estacion[10];
	memset( ultimo_nro_estacion , '\0' , 10 );
	char * nro_estacion;
	///Proceso la linea y cargo la siguiente hasta llegar al final
	int tam_bd = File_size( bd );
	while( ftell( bd ) != tam_bd )
	{
		
		nro_estacion = String_Cortar_hasta_FREE( &linea_cortada , "," );
		if( strcmp( ultimo_nro_estacion , nro_estacion ) != 0 )
		{
			
			cantidad++;
			strcpy( ultimo_nro_estacion , nro_estacion );
			
		}
		free( nro_estacion );
		nro_estacion = NULL;
		free( linea );
		linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
		linea_cortada = linea;
		
	}
	free( linea );
	
	fclose( bd );
	
	return cantidad;
	
}

char ** Estaciones_FREE( unsigned int * cant_estaciones )
{
	
	*cant_estaciones = Cantidad_de_estaciones();
	
	FILE * bd = fopen( "datos_meteorologicos.CSV" , "r" );
		if( bd == NULL ){
			return NULL;
		}
	
	char ** estaciones;
	estaciones = (char **)malloc( *cant_estaciones * sizeof(char *) );
	
	///Cargo la linea 4 (primera con datos)
	///@todo cambiar por mover para mejor manejo de memoria
	free( File_read_until_next_ocurrence_char_FREE( bd , 13 ) );
	free( File_read_until_next_ocurrence_char_FREE( bd , 13 ) );
	free( File_read_until_next_ocurrence_char_FREE( bd , 13 ) );
	char * linea;
	char * linea_cortada;
	linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
	linea_cortada = linea;
	char ultimo_nro_estacion[10];
	memset( ultimo_nro_estacion , '\0' , 10 );
	char * nro_estacion;
	unsigned int pos = 0;
	///Proceso la linea y cargo la siguiente hasta llegar al final
	int tam_bd = File_size( bd );
	while( ftell( bd ) != tam_bd )
	{
		
		nro_estacion = String_Cortar_hasta_FREE( &linea_cortada , "," );
		if( strcmp( ultimo_nro_estacion , nro_estacion ) != 0 )
		{
			
			strcpy( ultimo_nro_estacion , nro_estacion );
			///Cargo numero, nombre y cantidad separado por '|'
			char * nombre = String_Cortar_hasta_FREE( &linea_cortada , "," );
			///@todo cambiar por mover para mejor manejo de memoria
			free( String_Cortar_hasta_FREE( &linea_cortada , "," ) );
			free( String_Cortar_hasta_FREE( &linea_cortada , "," ) );
			int campos = 0;
			char * campo;
			while( linea_cortada != NULL )
			{
				
				campo = String_Cortar_hasta_FREE( &linea_cortada , "," );
				if( strcmp( campo , "--" ) != 0 )
					campos++;
				free( campo );
				
			}
			free( linea );
			char * estacion;
			estacion = malloc( strlen(nro_estacion) + strlen(nombre) + 5 );
			sprintf( estacion , "%s|%s|%i" , nro_estacion , nombre , campos );
			free( nro_estacion );
			free( nombre );
			estaciones[pos] = estacion;
			
			pos++;
			
		}
		
		linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
		linea_cortada = linea;
		
	}
	
	fclose( bd );
	
	return estaciones;
	
}

char * Listar_FREE( )
{
	
	unsigned int cant_columnas = 0;
	char ** cabecera;
	cabecera = Cabecera_FREE( &cant_columnas );
		if( cabecera == NULL )
		{
			char * bd_lose = "Base de datos perdida.";
			char * retorno = malloc( strlen( bd_lose ) );
			strcpy( retorno , bd_lose );
			return retorno;
		}
	
	unsigned int cant_estaciones = 0;
	char ** estaciones;
	estaciones = Estaciones_FREE( &cant_estaciones );
		if( cabecera == NULL )
		{
			char * bd_lose = "Base de datos perdida.";
			char * retorno = malloc( strlen( bd_lose ) );
			strcpy( retorno , bd_lose );
			return retorno;
		}
	
	///Asigno memoria a la lista
	unsigned int estacion;
	unsigned int tam_lista = 0;
	for( estacion = 0 ; estacion < cant_estaciones ; estacion++ )
	{
		
		char * puntero = estaciones[estacion];
		///@todo corregir (memoria):
		free( String_Cortar_hasta_FREE( &puntero , "|" ) );
		free( String_Cortar_hasta_FREE( &puntero , "|" ) );
		int campos;
		sscanf( puntero , "%i" , &campos );
		tam_lista += 2 + campos;
		
	}
	char ** lista = (char **)malloc( tam_lista * sizeof(char *) );
	
	///Cargo la lista
	unsigned int pos_lista = 0;
	for( estacion = 0 ; estacion < cant_estaciones ; estacion++ )
	{
		
		char * puntero = estaciones[estacion];
		char * puntero_mem = puntero;
		lista[pos_lista] = String_Cortar_hasta_FREE( &puntero , "|" );
		strcat( lista[pos_lista] , " " );
		pos_lista++;
		lista[pos_lista] = String_Cortar_hasta_FREE( &puntero , "|" );
		strcat( lista[pos_lista] , "\n" );
		pos_lista++;
		int campos;
		sscanf( puntero , "%i" , &campos );
		free( puntero_mem );
		unsigned int nro_campo;
		for( nro_campo = 0 ; nro_campo < campos ; nro_campo++ )
		{
			
			unsigned int tam = strlen( cabecera[nro_campo + 4] ) + 2;
			lista[pos_lista] = malloc( tam );
			lista[pos_lista][0] = '\t';
			lista[pos_lista][1] = '\0';
			strcat( lista[pos_lista] , cabecera[nro_campo + 4] );
			strcat( lista[pos_lista] , "\n" );
			pos_lista++;
			
		}
		
	}
	
	unsigned int cab_pos;
	for( cab_pos = 0 ; cab_pos < cant_columnas ; cab_pos++ )
		free( cabecera[cab_pos] );
	free( cabecera );
	
	///Asigno memoria al retorno
	unsigned int tam_retorno = 0;
	for( pos_lista = 0 ; pos_lista < tam_lista ; pos_lista++ )
	{
		
		tam_retorno += strlen( lista[pos_lista] );
		
	}
	char * retorno = malloc( tam_retorno );
	retorno[0] = '\0';
	
	///Cargo el retorno
	for( pos_lista = 0 ; pos_lista < tam_lista ; pos_lista++ )
	{
		
		strcat( retorno , lista[pos_lista] );
		free( lista[pos_lista] );
		
	}
	
	free( lista );

	retorno[tam_retorno] = '\0';

	return retorno;
	
}

char * 
Descargar( char * nro_estacion , int sockfdUDP , struct sockaddr_in addrUDP )
{
	
	FILE * archivo = fopen( nro_estacion , "w" );
		if( archivo == NULL )
			return "Intente mas tarde";
	
	FILE * bd = fopen( "datos_meteorologicos.CSV" , "r" );
		if( bd == NULL )
		{
			
			fclose(archivo);
			return "Base de datos perdida";
			
		}
	
	///Cargo el archivo
	char * linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
	fprintf( archivo , "%s\n" , linea );
	free( linea );
	linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
	fprintf( archivo , "%s\n" , linea );
	free( linea );
	linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
	fprintf( archivo , "%s\n" , linea );
	free( linea );
	int tam_bd = File_size( bd );
	while( ftell( bd ) != tam_bd )
	{
		
		linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
		char * linea_sin_cortar = linea;
		char * nro_estacion_linea = String_Cortar_hasta_FREE( &linea , "," );
		if( strcmp( nro_estacion , nro_estacion_linea ) == 0 )
			fprintf( archivo , "%s\n" , linea_sin_cortar );
		free( linea_sin_cortar );
		free( nro_estacion_linea );
		
	}
	
	///Envio el archivo
	if( Error_int( Sockets_Enviar_archivo_por_UDP( sockfdUDP ,
												   addrUDP ,
												   nro_estacion ,
												   SI ,
												   SI ) ,
				   NO ) )
		return "No existen datos de la estación solicitada";
	
	return "Envío de datos realizado";
	
}

char * Diario_precipitacion_FREE( char * nro_estacion )
{
	
	FILE * bd = fopen( "datos_meteorologicos.CSV" , "r" );
		if( bd == NULL )
			return "Base de datos perdida";
	
	char ** retorno = (char **)malloc( 31 * sizeof(char *) );
	char cabecera[] = "\tDia\tAcumulado[mm]";
	retorno[0] = cabecera;
	unsigned int retorno_dia = 1;
	unsigned int tam_retorno_cadena = 0;
	tam_retorno_cadena += strlen( cabecera ) + 1;
	
	///Recorro t'odo el archivo de datos (a partir de la 4ta fila)
	///@todo manejo de memoria:
	free( File_read_until_next_ocurrence_char_FREE( bd , 13 ) );
	free( File_read_until_next_ocurrence_char_FREE( bd , 13 ) );
	free( File_read_until_next_ocurrence_char_FREE( bd , 13 ) );
	char ultimo_dia[3];
	memset( ultimo_dia , '\0' , 3 );
	float precipitacion_acum = 0;
	int tam_bd = File_size( bd );
	while( ftell( bd ) != tam_bd )
	{
		
		///Trabajo solo con las lineas de nro_estacion
		char * linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
		char * linea_mem = linea;
		char * nro_estacion_linea = String_Cortar_hasta_FREE( &linea , "," );
		if( strcmp( nro_estacion , nro_estacion_linea ) == 0 )
		{
			///@todo memoria
			free( String_Cortar_hasta_FREE( &linea , "," ) );
			free( String_Cortar_hasta_FREE( &linea , "," ) );
			char * fecha = String_Cortar_hasta_FREE( &linea , "," );
			char * fecha_mem = fecha;
			char * dia = String_Cortar_hasta_FREE( &fecha , "/" );
			free( fecha_mem );
			free( String_Cortar_hasta_FREE( &linea , "," ) );
			free( String_Cortar_hasta_FREE( &linea , "," ) );
			free( String_Cortar_hasta_FREE( &linea , "," ) );
			char * precipitacion_str = String_Cortar_hasta_FREE( &linea , "," );
			float precipitacion = 0;
			sscanf( precipitacion_str , "%f" , &precipitacion );
			free( precipitacion_str );
			if( strcmp( ultimo_dia , "\0" ) == 0 )
				strcpy( ultimo_dia , dia );
			if( strcmp( dia , ultimo_dia ) == 0 )
				precipitacion_acum += precipitacion;
			else
			{
				
				char * prec_acum_str;
				prec_acum_str = String_Flotante_a_cadena_FREE(
											precipitacion_acum );
				precipitacion_acum = precipitacion;
				char * retorno_fila;
				retorno_fila = malloc( strlen( prec_acum_str )
										+ strlen( dia )
										+ 2 );
				retorno_fila[0] = '\0';
				strcat( retorno_fila , "\t" );
				strcat( retorno_fila , ultimo_dia );
				strcat( retorno_fila , "\t" );
				strcat( retorno_fila , prec_acum_str );
				free( prec_acum_str );
				retorno[retorno_dia++] = retorno_fila;
				tam_retorno_cadena += strlen( retorno_fila ) + 1;
				
				strcpy( ultimo_dia , dia );
				
			}
			
			free( dia );

		}//if

		free( linea_mem );
		free( nro_estacion_linea );
		
	}
	
	///Paso el retorno a una sola cadena
	char * retorno_cadena = malloc( tam_retorno_cadena );
	retorno_cadena[0] = '\0';
	unsigned int pos;
	for( pos = 0 ; pos < retorno_dia ; pos++ )
	{
		
		strcat( retorno_cadena , retorno[pos] );
		strcat( retorno_cadena , "\n" );
		///@todo recuperar la funcion, falla sin sentido
		//free( retorno[pos] ); !!! ???
		
	}
	free( retorno );
	
	return retorno_cadena;
	
}

char *
Comando_FREE( char comando[] , int sockfdUDP , struct sockaddr_in addrUDP )
{
	
	char * cmd_cut = comando;
	switch( cmd_cut[0] )
	{
		
		case 'd':
			
			if( strcmp( cmd_cut , "desconectar" ) == 0 )
			{
				char * desc_rec = "Desconexión recibida.";
				char * retorno = malloc( strlen( desc_rec ) );
				strcpy( retorno , desc_rec );
				retorno[strlen(desc_rec)] = '\0';
				return retorno;
			}
			///Comprobar 'descargar' o 'diario_precipitacion'
			char * orden = String_Cortar_hasta_FREE( &cmd_cut , " " );
			if( strcmp( orden , "descargar" ) == 0 )
			{
				free( orden );
				char * rtrn = Descargar( cmd_cut , sockfdUDP , addrUDP );
				char * retorno = malloc( strlen(rtrn) );
				strcpy( retorno , rtrn );
				retorno[strlen(rtrn)] = '\0';
				return retorno;
			}
			if( strcmp( orden , "diario_precipitacion" ) == 0
				|| strcmp( orden , "diario_precipitación" ) == 0 )
			{
				free( orden );
				return Diario_precipitacion_FREE( cmd_cut );
			}
			break;
			
		case 'l':
			
			if( strcmp( cmd_cut , "listar" ) == 0 )
				return Listar_FREE( );
			break;
		
	}
	
	return "Comando no reconocido";
	
}
