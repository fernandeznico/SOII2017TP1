/**
 * @author Fernández Nicolás (nicofernandez@alumnos.unc.edu.ar)
 * @date Abril, 2016
 * @version 1.2017 beta
 *
 * @brief Cliente para la obtención de datos de telemetría de los
 * servidores AWS
 * 
 * \file Cliente.c
 */

#include <stdio.h>
#include <stdio_ext.h> /// __purge
#include <time.h> /// time()

#include "../Recursos/File.h"
#include "../Recursos/Sockets.h"
#include "../Recursos/Error.h"
#include "../Recursos/String.h"

char prompt[22];

/**
 * @brief Para mantener un formato constante del prompt
 */
void Imprimir_prompt( );

/**
 * @brief Solicita los datos del servidor AWS hasta lograr una conexion
 * exitosa (llama a Conectar_con_el_servidor)
 * 
 * @return Regresa el valor del file descriptor asociado a la conexión
 * el cual es un valor positivo o -1 en caso de fallar la conexión
 */
int Establecer_conexion( );

/**
 * @brief Conecta con el servidor AWS conociendo su ip y puerto
 * 
 * @param ip: Cadena que contiene la ip en formato "255.255.255.255"
 * @param puerto: numero de puerto en que se comunica el servidor
 * @return Regresa el valor del file descriptor asociado a la conexión
 * el cual es un valor positivo o -1 en caso de fallar la conexión
 */
int Conectar_con_el_servidor( char * ip , int puerto );

/**
 * @brief Llama a Sockets_Recibir_archivo_por_UDP para guardar
 * localmente una copia del archivo de telemetría en el servidor, el
 * cual debe ser informado previamente para iniciar el envío e ignora
 * los ingresos de nuevos comandos hasta finalizar
 * 
 * @param sockfd : File descriptor asociado a la conexión
 * @param addrUDP : Contiene los datos de referencia de la dirección
 * de conexión
 * @param nombre : Numero de la estacion pedida que sera el nombre del
 * archivo
 */
void Descargar( int sockfdUDP , struct sockaddr_in addrUDP , char * n );

int main( int argc , char **argv )
{
	
	memset( prompt , '\0' , sizeof( prompt ) );
	prompt[0] = '>';
	
	char mensaje_enviar[TAM];
	memset( mensaje_enviar , '\0' , TAM );
	char msj_in[TAM];
	char * msj_in_long;
	
	///TCP:
	int sockfdTCP = Establecer_conexion( );
	
	///UDP:
	int puerto = 0;
	struct sockaddr_in addrUDP;
	int sockfdUDP = Sockets_Crear_Socket_INET_UDP( &addrUDP , &puerto );
		Error_int( sockfdUDP , SI );
	printf( "\n Servidor UDP INET iniciado en el puerto %i." , puerto );
	sprintf( mensaje_enviar , "%i" , puerto );
	Sockets_Enviar_mensaje_TCP( sockfdTCP , mensaje_enviar );
	
	///Contrasenia:
	Error_int( Sockets_Leer_mensaje_TCP( sockfdTCP , msj_in , TAM ) ,
			   SI );
	printf( "\n %s" , msj_in );
	/*
	fgets( mensaje_enviar , TAM , stdin );
		mensaje_enviar[ strlen( mensaje_enviar ) - 1 ] = '\0';
	Error_int( Sockets_Enviar_mensaje_TCP( sockfdTCP ,
										   mensaje_enviar ) ,
			   SI );
	*/

Error_int( Sockets_Enviar_mensaje_TCP( sockfdTCP , "root" ) , SI );

	msj_in_long = Sockets_Leer_mensaje_largo_TCP_FREE( sockfdTCP );
	Error_pnt( msj_in_long , SI );
	printf( "\n %s\n" , msj_in_long );
	if( strcmp( msj_in_long , "Clave incorrecta" ) == 0 )
		return 1;
	
	///Envio recepción:
	do
	{
		
		///Ingreso del comando y envio del mismo
		Imprimir_prompt( );
		fgets( mensaje_enviar , TAM , stdin );
		mensaje_enviar[ strlen( mensaje_enviar ) - 1 ] = '\0';
			if( !strcmp( mensaje_enviar , "" ) )
				continue;///Solo presiono enter
		Error_int( Sockets_Enviar_mensaje_TCP( sockfdTCP ,
											   mensaje_enviar ) ,
				   SI );
		
		///En caso de recibir un archivo
		char * argumento = mensaje_enviar;
		char * orden = String_Cortar_hasta_FREE( &argumento , " " );
		if ( !strcmp( orden , "descargar" ) )
			Descargar( sockfdUDP , addrUDP , argumento );
		Mem_desassign( (void **)&orden );
		
		///Respuesta al comando
		Mem_desassign( (void **)&msj_in_long );
		msj_in_long = Sockets_Leer_mensaje_largo_TCP_FREE( sockfdTCP );
		Error_pnt( msj_in_long , SI );
		printf( "\n %s" , msj_in_long );
		
	} while( strcmp( mensaje_enviar , "desconectar" ) );
	
	shutdown( sockfdTCP , 2 );
	shutdown( sockfdUDP , 2 );
	
	printf( "\n" );
	
	return EXIT_SUCCESS;
	
}

int Conectar_con_el_servidor( char * ip , int puerto )
{
	
	struct hostent * server = Sockets_Verificar_host_IPv4( ip );
		if( Error_pnt( server , NO ) )
			return -1;
	
	int sockfdTCP = Sockets_Crear_Y_Conectar_Socket_INET_TCP( server ,
															  puerto );
		if( Error_int( sockfdTCP , NO ) )
			return -1;

	printf( "\n Conexión establecida con el servidor AWS (%s:%i)" ,
			 ip ,
			 puerto );
	
	return sockfdTCP;
	
}

void Imprimir_prompt( )
{
	
	printf( "\n%s " , prompt );
	
}

int Establecer_conexion( )
{
	/*
	char comando[TAM];
	char connect[7];
	char datos_de_conexion[TAM - 7];
	char vacio[2];
	char usuario[22];
	char ip[15];
	int port;
	*/
	int sockfd;
	
	do
	{
		/*
		do
		{
		
			printf( "\n Use 'connect usuario@numero_ip:port'"
					" sin comillas para conectar.\n"
					" Ej: 'connect juan@127.0.0.1:6020'" );
			
			connect[0] = '\0';
			usuario[0] = '\0';
			ip[0] = '\0';
			port = 0;
			vacio[0] = '\0';
			
			Imprimir_prompt( );
			
			fgets( comando , TAM , stdin );
			
			sscanf( comando ,
				   "%s %s %s" ,
					connect ,
					datos_de_conexion ,
					vacio );
			
			int pos = 0;
			pos = strcspn( datos_de_conexion , "@" );
			strncpy( usuario , datos_de_conexion , pos );
			usuario[pos] = '\0';
			
			char * ip_y_puerto = &datos_de_conexion[ pos + 1 ];
			pos = strcspn( ip_y_puerto , ":" );
			strncpy( ip , ip_y_puerto , pos );
			ip[pos] = '\0';
			
			char * puerto = &ip_y_puerto[ pos + 1 ];
			
			sscanf( puerto , "%i" , &port );
			
		} while( strcmp( connect , "connect" )
				 || strcmp( vacio , "" ) );
		
		sockfd = Conectar_con_el_servidor( ip , port );
		*/

sockfd = Conectar_con_el_servidor( "127.0.0.1" , 6020 );

	} while( sockfd < 0 );
	
	/*
	sprintf( prompt , "%s@%s:" , usuario , ip );
	*/

sprintf( prompt , "%s@%s:" , "root" , "127.0.0.1" );

	
	return sockfd;
	
}

void Descargar
( int sockfdUDP , struct sockaddr_in addrUDP , char * nombre )
{
	
	char ruta[strlen(nombre) + 12 ];
	ruta[0] = '.';
	ruta[1] = '/';
	ruta[2] = '\0';
	strcat( ruta , "Descargas" );
	strcat( ruta , "/" );
	strcat( ruta , nombre );
	
	Error_int( Sockets_Recibir_archivo_por_UDP( sockfdUDP ,
												addrUDP ,
												ruta ,
												SI ) ,
			   SI );
	__fpurge(stdin);
	
}
