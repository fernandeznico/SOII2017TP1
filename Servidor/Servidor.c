/**
 * @author Fernández Nicolás (nicofernandez@alumnos.unc.edu.ar)
 * @date Abril, 2016
 * @version 1.2017 beta
 *
 * @brief Servidor AWS que provee los datos de telemetría
 * correspondientes (de la estación en la que se ejecuta) a los
 * distintos clientes que la soliciten
 * 
 * @file Servidor.c
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
 * @param addrUDP: para guardar los datos de dirección referidos al
 * cliente
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
char * Comando_FREE
( char comando[] , int sockfdUDP , struct sockaddr_in addrUDP );

/**
 * @brief Carga los nombres de las comlumnas de la bd en un vector
 * de cadenas
 *
 * @return Nombres y cantidad de columnas en un struct text
 */
text * Cabecera_FREE( );

/**
 * @brief Carga los nombres de las estaciones en un struct text
 *
 * @return Nombres y cantidad de estaciones en un struct text
 */
text * Estaciones_FREE( );

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
		
		while(Cliente( servidor , &conexion , &sockfdUDP , &addrUDP ));
		
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
				cmd_re = Comando_FREE( mensaje_leer ,
									   sockfdUDP ,
									   addrUDP );
				strcpy( mensaje_enviar , cmd_re );
				Mem_desassign( (void **)&cmd_re );
				
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
	
	for(ifa = ifaddr , n = 0 ; ifa != NULL ; ifa = ifa->ifa_next , n++)
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
	Error_int( Sockets_Leer_mensaje_TCP( *conexion , msj_leer , TAM ) ,
			   SI );
	char puerto_UDP_str[ strlen(msj_leer) ];
	strcpy( puerto_UDP_str , msj_leer );
	int puerto_UDP = atoi	( puerto_UDP_str );
	///Conecto por UDP:
	struct hostent * servidorUDP = Sockets_Verificar_host_IPv4(dir.ip);
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

text * Cabecera_FREE( )
{
	
	FILE * bd = fopen( "datos_meteorologicos.CSV" , "r" );
		if( bd == NULL ){
			return NULL;
		}
	
	///Asigno memoria al vector de cadenas
	File_move_to_next_ocurrence_char( bd , 13 , 1 );
	File_move_to_next_ocurrence_char( bd , 13 , 1 );
	char * linea3 = File_read_until_next_ocurrence_char_FREE( bd , 13 );
	char * linea3_mem = linea3;
	linea3 = Corregir_simbolos_FREE( linea3 );
	Mem_desassign( (void **)&linea3_mem );
	linea3_mem = linea3;
	unsigned int cantidad_de_columnas;
	cantidad_de_columnas = String_Cantidad_de_columnas( linea3 , "," );
	text * cabecera;
	cabecera = Mem_Create_text_null( cantidad_de_columnas );
	
	unsigned int pos_cab;
	for( pos_cab = 0 ; pos_cab < cabecera->parts ; pos_cab++ )
	{
		
		char * str = String_Cortar_hasta_FREE( &linea3 , "," );
		printf( "\n CABECERA: pos_cab = %u | str = %s" , pos_cab , str );
		cabecera->t[pos_cab] = Mem_Create_string( strlen( str ) );
		strcpy( cabecera->t[pos_cab] , str );
		
	}
	
	Mem_desassign( (void **)&linea3_mem );
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
	File_move_to_next_ocurrence_char( bd , 13 , /*add =*/ 1 );
	File_move_to_next_ocurrence_char( bd , 13 , /*add =*/ 1 );
	File_move_to_next_ocurrence_char( bd , 13 , /*add =*/ 1 );
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
		Mem_desassign( (void **)&nro_estacion );
		Mem_desassign( (void **)&linea );
		linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
		linea_cortada = linea;
		
	}
	
	Mem_desassign( (void **)&linea );
	
	fclose( bd );
	
	return cantidad;
	
}

text * Estaciones_FREE( )
{
	
	unsigned int cant_estaciones = Cantidad_de_estaciones();
	
	FILE * bd = fopen( "datos_meteorologicos.CSV" , "r" );
		if( bd == NULL )
			return NULL;
	
	text * estaciones;
	estaciones = Mem_Create_text_null( cant_estaciones );
	
	///Cargo la linea 4 (primera con datos)
	File_move_to_next_ocurrence_char( bd , 13 , /*add =*/ 1 );
	File_move_to_next_ocurrence_char( bd , 13 , /*add =*/ 1 );
	File_move_to_next_ocurrence_char( bd , 13 , /*add =*/ 1 );
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
			char * nombre = String_Cortar_hasta_FREE( &linea_cortada ,
													  "," );
			String_Mover_hasta( &linea_cortada , "," , /*add =*/ 1 );
			String_Mover_hasta( &linea_cortada , "," , /*add =*/ 1 );
			int campos = 0;
			char * campo;
			while( linea_cortada != NULL )
			{
				
				campo = String_Cortar_hasta_FREE(&linea_cortada , ",");
				if( strcmp( campo , "--" ) != 0 )
					campos++;
				Mem_desassign( (void **)&campo );
				
			}
			Mem_desassign( (void **)&linea );
			char * estacion = (char *)Mem_assign( strlen(nro_estacion)
												  + strlen(nombre)
												  + 5 );
			sprintf( estacion ,
					"%s|%s|%i" ,
					 nro_estacion ,
					 nombre ,
					 campos );
			Mem_desassign( (void **)&nro_estacion );
			Mem_desassign( (void **)&nombre );
			estaciones->t[pos] = estacion;
			
			pos++;
			
		}
		
		linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
		linea_cortada = linea;
		
	}
	
	Mem_desassign( (void **)&linea );
	
	fclose( bd );
	
	return estaciones;
	
}

char * Listar_FREE( )
{
	
	text * cabecera;
	cabecera = Cabecera_FREE( );
		if( cabecera == NULL )
			return String_Crear( "Base de datos perdida." );
	
	Print_text_all_parts( cabecera );
	
	text * estaciones;
	estaciones = Estaciones_FREE( );
		if( estaciones == NULL )
			return String_Crear( "Base de datos perdida." );
	
	Print_text_all_parts( estaciones );
	
	///Asigno memoria a la lista
	unsigned int estacion;
	unsigned int tam_lista = 0;
	for( estacion = 0 ; estacion < estaciones->parts ; estacion++ )
	{
		
		char * puntero = estaciones->t[estacion];
		String_Mover_hasta( &puntero , "|" , /*add =*/ 1 );
		String_Mover_hasta( &puntero , "|" , /*add =*/ 1 );
		int campos;
		sscanf( puntero , "%i" , &campos );
		tam_lista += 2 + campos;
		
	}
	text * lista = Mem_Create_text_null( tam_lista );
	
	///Cargo la lista
	unsigned int pos_lista = 0;
	for( estacion = 0 ; estacion < estaciones->parts ; estacion++ )
	{
		
		char * puntero = estaciones->t[estacion];
		char * str = String_Cortar_hasta_FREE(&puntero , "|");
		lista->t[pos_lista] = Mem_Create_string( strlen( str ) + 1 );
		strcpy( lista->t[pos_lista] , str );
		strcat( lista->t[pos_lista] , " " );
		Mem_desassign( (void *)&str );
		
		pos_lista++;
		str = String_Cortar_hasta_FREE(&puntero , "|");
		lista->t[pos_lista] = Mem_Create_string( strlen( str ) + 1 );
		strcpy( lista->t[pos_lista] , str );
		strcat( lista->t[pos_lista] , "\n" );
		Mem_desassign( (void *)&str );
		
		pos_lista++;
		int campos;
		sscanf( puntero , "%i" , &campos );
		unsigned int nro_campo;
		for( nro_campo = 0 ; nro_campo < campos ; nro_campo++ )
		{
			
			unsigned int tam = strlen( cabecera->t[nro_campo + 4] ) + 2;
			lista->t[pos_lista] = Mem_Create_string( tam );
			lista->t[pos_lista][0] = '\t';
			lista->t[pos_lista][1] = '\0';
			strcat( lista->t[pos_lista] , cabecera->t[nro_campo + 4] );
			strcat( lista->t[pos_lista] , "\n" );
			pos_lista++;
			
		}
		
	}
	
	Mem_Delete_text( &cabecera );
	Mem_Delete_text( &estaciones );
	
	///Asigno memoria al retorno
	unsigned int tam_retorno = 0;
	for( pos_lista = 0 ; pos_lista < tam_lista ; pos_lista++ )
		tam_retorno += strlen( lista->t[pos_lista] );
	char * retorno = Mem_Create_string( tam_retorno );
	
	///Cargo el retorno
	for( pos_lista = 0 ; pos_lista < tam_lista ; pos_lista++ )
		strcat( retorno , lista->t[pos_lista] );
	
	Mem_Delete_text( &lista );
	
	return retorno;
	
}

char * Descargar
( char * nro_estacion , int sockfdUDP , struct sockaddr_in addrUDP )
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
	Mem_desassign( (void **)&linea );
	linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
	fprintf( archivo , "%s\n" , linea );
	Mem_desassign( (void **)&linea );
	linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
	fprintf( archivo , "%s\n" , linea );
	Mem_desassign( (void **)&linea );
	int tam_bd = File_size( bd );
	while( ftell( bd ) != tam_bd )
	{
		
		linea = File_read_until_next_ocurrence_char_FREE( bd , 13 );
		char * linea_sin_cortar = linea;
		char * nro_estacion_linea = String_Cortar_hasta_FREE( &linea ,
															  "," );
		if( strcmp( nro_estacion , nro_estacion_linea ) == 0 )
			fprintf( archivo , "%s\n" , linea_sin_cortar );
		Mem_desassign( (void **)&linea_sin_cortar );
		Mem_desassign( (void **)&nro_estacion_linea );
		
	}
	
	///Envio el archivo
	if( Error_int( Sockets_Enviar_archivo_por_UDP( sockfdUDP ,
												   addrUDP ,
												   nro_estacion ,
												   SI ) ,
				   NO ) )
		return "No existen datos de la estación solicitada";
	
	return "Envío de datos realizado";
	
}

char * Diario_precipitacion_FREE( char * nro_estacion )
{
	
	FILE * bd = fopen( "datos_meteorologicos.CSV" , "r" );
		if( bd == NULL )
			return String_Crear( "Base de datos perdida" );
	
	char ** retorno = (char **)malloc( 31 * sizeof(char *) );
	char cabecera[] = "\tDia\tAcumulado[mm]";
	///@todo posible problema al desalojar la memoria !!!
	retorno[0] = cabecera;
	unsigned int retorno_dia = 1;
	unsigned int tam_retorno_cadena = 0;
	tam_retorno_cadena += strlen( cabecera ) + 1;
	
	///Recorro t'odo el archivo de datos (a partir de la 4ta fila)
	File_move_to_next_ocurrence_char( bd , 13 , /*add =*/ 1 );
	File_move_to_next_ocurrence_char( bd , 13 , /*add =*/ 1 );
	File_move_to_next_ocurrence_char( bd , 13 , /*add =*/ 1 );
	char ultimo_dia[3];
	memset( ultimo_dia , '\0' , 3 );
	float precipitacion_acum = 0;
	int tam_bd = File_size( bd );
	while( ftell( bd ) != tam_bd )
	{
		
		///Trabajo solo con las lineas de nro_estacion
		char * linea = File_read_until_next_ocurrence_char_FREE( bd ,
																 13 );
		char * linea_mem = linea;
		char * nro_estacion_linea = String_Cortar_hasta_FREE( &linea ,
															  "," );
		if( strcmp( nro_estacion , nro_estacion_linea ) == 0 )
		{
			///@todo memoria
			String_Mover_hasta( &linea , "," , /*add =*/ 1 );
			String_Mover_hasta( &linea , "," , /*add =*/ 1 );
			char * fecha = String_Cortar_hasta_FREE( &linea , "," );
			char * fecha_mem = fecha;
			char * dia = String_Cortar_hasta_FREE( &fecha , "/" );
			Mem_desassign( (void **)&fecha_mem );
			String_Mover_hasta( &linea , "," , /*add =*/ 1 );
			String_Mover_hasta( &linea , "," , /*add =*/ 1 );
			String_Mover_hasta( &linea , "," , /*add =*/ 1 );
			char * precipitacion_str = String_Cortar_hasta_FREE(&linea ,
																"," );
			float precipitacion = 0;
			sscanf( precipitacion_str , "%f" , &precipitacion );
			Mem_desassign( (void **)&precipitacion_str );
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
				retorno_fila = Mem_assign( strlen( prec_acum_str )
											+ strlen( dia )
											+ 2 );
				retorno_fila[0] = '\0';
				strcat( retorno_fila , "\t" );
				strcat( retorno_fila , ultimo_dia );
				strcat( retorno_fila , "\t" );
				strcat( retorno_fila , prec_acum_str );
				Mem_desassign( (void **)&prec_acum_str );
				retorno[retorno_dia++] = retorno_fila;
				tam_retorno_cadena += strlen( retorno_fila ) + 1;
				
				strcpy( ultimo_dia , dia );
				
			}
			
			Mem_desassign( (void **)&dia );
			
		}//if
		
		Mem_desassign( (void **)&linea_mem );
		Mem_desassign( (void **)&nro_estacion_linea );
		
	}
	
	///Paso el retorno a una sola cadena
	char * retorno_cadena = Mem_assign( tam_retorno_cadena );
	retorno_cadena[0] = '\0';
	unsigned int pos;
	for( pos = 0 ; pos < retorno_dia ; pos++ )
	{
		
		strcat( retorno_cadena , retorno[pos] );
		strcat( retorno_cadena , "\n" );
		///@todo text struct
		Mem_desassign( (void **)&(retorno[pos]) );
		
	}
	///@todo text struct
	Mem_desassign( (void **)&retorno );
	
	return retorno_cadena;
	
}

char * Comando_FREE
( char comando[] , int sockfdUDP , struct sockaddr_in addrUDP )
{
	
	char * cmd_cut = comando;
	switch( cmd_cut[0] )
	{
		
		case 'd':
			
			if( strcmp( cmd_cut , "desconectar" ) == 0 )
				return String_Crear( "Desconexión recibida." );
			
			///Comprobar 'descargar' o 'diario_precipitacion'
			char * orden = String_Cortar_hasta_FREE( &cmd_cut , " " );
				if( orden == NULL )
					break;
			if( strcmp( orden , "descargar" ) == 0 )
			{
				
				Mem_desassign( (void **)&orden );
				return String_Crear( Descargar( cmd_cut ,
												sockfdUDP ,
												addrUDP )
									);
			}
			if( strcmp( orden , "diario_precipitacion" ) == 0
				|| strcmp( orden , "diario_precipitación" ) == 0 )
			{
				
				Mem_desassign( (void **)&orden );
				return Diario_precipitacion_FREE( cmd_cut );
				
			}
			break;
			
		case 'l':
			
			if( strcmp( cmd_cut , "listar" ) == 0 )
				return Listar_FREE( );
			break;
		
	}
	
	return String_Crear( "Comando no reconocido" );
	
}
