/**
 * @author Fernández Nicolás (nicofernandez@alumnos.unc.edu.ar)
 * @date Abril, 2016
 * @version 1.2017 beta
 *
 * @brief Manejo de interfaces de comunicación basadas en sockets
 * 
 * \file Sockets.h
 */

#ifndef Sockets
#define Sockets

#include <string.h> //memset
#include <sys/socket.h> //socket
#include <netdb.h> //gethostbyname
#include <unistd.h> //write , read
#include <stdlib.h> //atoi
#include <arpa/inet.h> //inet_aton

#include "File.h"

#define TAM 256

struct direccion {
	
	char	ip[INET_ADDRSTRLEN];
	int		puerto;
	
};

 
/**
 * @brief Controla que el host solicitado sea posible
 * 
 * @param hostaddr: nombre del host como "localhost" o "255.255.255.255"
 * @return host en formato para ser utilizado en las funciones de socket
 */
struct hostent * Sockets_Verificar_host_IPv4( char * hostaddr )
{
	
	struct in_addr addr;
		if( inet_aton( hostaddr , &addr ) == 0 )
		{
			
			fprintf( stderr , "ERROR: no existe el host" );
			return NULL;
			
		}
	
	struct hostent * server = gethostbyaddr( &addr ,
											 sizeof(addr) ,
											 AF_INET );
		if( server == NULL )
		{
			
			fprintf( stderr , "ERROR: no existe el host" );
			
		}
	
	return server;
	
}

/**
 * @brief Obtiene la ip y puerto de una sockaddr_in
 * 
 * @param address: de donde se obtienen los datos
 * @param dir: donde se guardan
 */
void Sockets_Direccion_del_sockaddr_in
( struct sockaddr_in address , struct direccion * dir )
{
	
	char ip[INET_ADDRSTRLEN];
	
	inet_ntop( AF_INET ,
			   &address.sin_addr.s_addr ,
			   ip ,
			   INET6_ADDRSTRLEN );
	
	strcpy( dir->ip , ip );
	
	dir->puerto = ntohs( address.sin_port );
	
}

/**
 * @brief Obtiene la ip y puerto asociado a un file descriptor
 * 
 * @param address: de donde se obtienen los datos
 * @param dir: donde se guardan
 */
void Sockets_Direccion_del_socket( int sockfd , struct direccion * dir )
{
	
	struct sockaddr_in address;
	int addr_size = sizeof(address);
	
	getsockname( sockfd ,
				(struct sockaddr *)&address ,
				(socklen_t *__restrict)&addr_size );
	
	Sockets_Direccion_del_sockaddr_in( address , dir );
	
}

 
/**
 * @brief Crea y conecta un socket INET TCP
 * 
 * @param server: datos del host
 * @param puerto: puerto de la conexión
 * @return file descriptor del socket creado o -1 en caso de error
 */
int Sockets_Crear_Y_Conectar_Socket_INET_TCP
( struct hostent * server , int puerto )
{
	
	//TCP: SOCK_STREAM
	//Internet: AF_INET
	
	int sockfd;
		sockfd = socket( AF_INET , SOCK_STREAM , 0 );
		if ( sockfd < 0 ) {
			fprintf( stderr , "ERROR: apertura de socket" );
			return -1;
		}
	
	struct sockaddr_in serv_addr;
		memset( &serv_addr , '0' , sizeof( serv_addr ) );
		serv_addr.sin_family = AF_INET;
		bcopy( (char *)server->h_addr ,
			   (char *)&serv_addr.sin_addr.s_addr ,
			    server->h_length );
		serv_addr.sin_port = htons( puerto );
	
	int error_de_conexion = 0;
		error_de_conexion = connect( sockfd ,
									(struct sockaddr *)&serv_addr ,
									 sizeof( serv_addr ) );
		if ( error_de_conexion < 0 ) {
			fprintf( stderr , "ERROR: conexion" );
			return -1;
		}
	
	return sockfd;
	
}

 
/**
 * @brief Crea y conecta un socket INET UDP
 * 
 * @param server: datos del host
 * @param dest_addr: datos de la direccion destino
 * @param puerto: puerto de conexión
 * @return numero del socket creado para realizar operaciones afines 
 */
int Sockets_Crear_Y_Conectar_Socket_INET_UDP
(struct hostent * server , struct sockaddr_in * dest_addr , int puerto)
{
	
	int sockfd;
		sockfd = socket( AF_INET , SOCK_DGRAM , 0 );
		if (sockfd < 0) {
			fprintf( stderr , "apertura de socket" );
			return -1;
		}
	
	///Guardo los datos de la conexión:
	dest_addr->sin_family = AF_INET;
	dest_addr->sin_port = htons( puerto );
	dest_addr->sin_addr = *( (struct in_addr *)server->h_addr );
	memset( &(dest_addr->sin_zero) , '\0' , 8 );
	
	return sockfd;
	
}

void Sockets_Si_puerto_es_aleatorio_obtenerlo
( int sockfd , int * puerto )
{
	
	if( *puerto != 0 )
		return;
		
	//El SO asignó aleatroiamente un puerto libre
	struct direccion dir;
	Sockets_Direccion_del_socket( sockfd , &dir );
	*puerto = dir.puerto;
	
}

/**
 * @brief Crea socket INET TCP
 * 
 * @param sockfd: para guardar el file descriptor del socket creado
 * @param puerto: puerto que se desea recibir conexión
 * @return -1 por erroe, de lo contario 0 
 */
int Sockets_Crear_Socket_INET_TCP( int * sockfd , int * puerto )
{
	
	//TCP: SOCK_STREAM
	//Internet: AF_INET
	
	int error;
	struct sockaddr_in serv_addr;
	
	*sockfd = socket( AF_INET , SOCK_STREAM , 0 );
		if ( *sockfd < 0 ) { 
			fprintf( stderr ,
					"ERROR: No se pudo crear la conexión TCP. "
					"(socket)" );
			return -1;
		}
	
	memset( &serv_addr , 0 , sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons( *puerto );
	
	int yes = 1;
	error = setsockopt( *sockfd ,
						SOL_SOCKET ,
						SO_REUSEADDR ,
						&yes ,
						sizeof(int) );
		if (error == -1) {
			fprintf( stderr ,
					"ERROR: No se pudo configurar la conexión TCP. \
					(setsockopt)" );
			return -1;
		}
	
	error = bind( *sockfd ,
				  (struct sockaddr *)&serv_addr ,
				   sizeof( serv_addr ) );
		if ( error < 0 ) {
			fprintf( stderr ,
					"ERROR: No se pudo configurar la conexión TCP. \
					(ligadura)" );
			return -1;
		}
	
	listen( *sockfd , 0 );
	
	Sockets_Si_puerto_es_aleatorio_obtenerlo( *sockfd , puerto );
	
	return 0;
	
}

/**
 * @brief Crea socket INET UDP
 * 
 * @param serv_addr: para guardar los datos del socket creado
 * @param puerto: puerto que se desea recibir conexión
 * @return file descriptor asociado del socket creado o -1 por error
 */
int Sockets_Crear_Socket_INET_UDP
( struct sockaddr_in * serv_addr , int * puerto )
{
	
	//UDP: SOCK_DGRAM
	//Internet: AF_INET
	
	int sockfd , error;
	
	sockfd = socket( AF_INET , SOCK_DGRAM , 0 );
		if (sockfd < 0) {
			fprintf( stderr ,
					"ERROR: No se pudo configurar la conexión\
					 de paso de arhivos. (socket)" );
			return -1;
		}
	
	int serv_addr_size = sizeof( serv_addr );
	memset( serv_addr , 0 , serv_addr_size );
	serv_addr->sin_family = AF_INET;
	serv_addr->sin_addr.s_addr = INADDR_ANY;
	serv_addr->sin_port = htons( *puerto );
	memset( &(serv_addr->sin_zero) , '\0' , 8 );
	
	int yes = 1;
	error = setsockopt( sockfd ,
						SOL_SOCKET ,
						SO_REUSEADDR ,
						&yes ,
						sizeof(int) );
		if ( error == -1) {
			fprintf( stderr ,
					"ERROR: No se pudo configurar la \
					 conexión de paso de arhivos. \
					(setsockopt)" );
			return -1;
		}
	
	error = bind( sockfd ,
				 (struct sockaddr *)serv_addr ,
				  sizeof(*serv_addr) );
		if ( error < 0 ) {
			fprintf( stderr ,
					"ERROR: No se pudo configurar la conexión \
					 de paso de arhivos. (binding)" );
			return -1;
		}
	
	Sockets_Si_puerto_es_aleatorio_obtenerlo( sockfd , puerto );
	
	return sockfd;
	
}

/**
 * @brief En base a un socket TCP se pone a la espera de recibir un
 * cliente y acepta la conexión con el mismo
 * 
 * @param newsockfd : Nuevo socket para enlazar el cliente
 * @param sockfd : Socket de recepción de clientes
 * @param cli_addr : Datos de la dirección del cliente
 * @param cli_addr : Datos de la dirección del cliente en estructura
 * resumida
 */
int Sockets_Aceptar_un_cliente( newsockfd , sockfd , cli_addr , dir )
int * newsockfd ;
int sockfd ;
struct sockaddr_in cli_addr ;
struct direccion * dir ;
{
	
	int clilen = sizeof( cli_addr );
	
	///Espera a que un cliente solicite conexión:
	*newsockfd = accept( sockfd ,
						(struct sockaddr *)&cli_addr ,
						(socklen_t* __restrict__)&clilen );
					 // (socklen_t* __restrict__)
					 // fue agregado para que no cause error
		if ( *newsockfd < 0 ) {
			fprintf( stderr ,
					"ERROR: Intento de conexión fallido (accept)" );
			return -1;
		}
	
	Sockets_Direccion_del_sockaddr_in( cli_addr , dir );
	
	return 0;
	
}

/**
 * @brief Recibe una cadena del tamaño indicado por UDP
 * 
 * @param sockfd : Socket de comunicacion.
 * @param serv_addr : Datos de la dirección.
 * @param tamanio : Tamaño del mensaje a recibir.
 * @return Mensaje recibido por UDP o 'NULL' en caso de error
 */
void Sockets_Leer_mensaje_UDP
( sockfd , buffer , tamanio , serv_addr , addr_size )
int sockfd ;
char buffer[];
int tamanio ;
struct sockaddr_in * serv_addr ;
int addr_size ;
{
	
	//memset( buffer , '\0' , tamanio-1 );
	
	int error = recvfrom( sockfd ,
						  buffer ,
						  tamanio ,
						  0 ,
						 (struct sockaddr *)serv_addr ,
						 (socklen_t *__restrict)&addr_size );
		if ( error < 0 ) {
			fprintf( stderr ,
					"ERROR: No se pudo leer el mensaje. (UDP)" );
			return;
		}
	
	return;
	
}

/**
 * @param mensaje : Mensaje a enviar por UDP.
 */
int Sockets_Enviar_mensaje_UDP
( sockfd , serv_addr , addr_size , mensaje )
int sockfd ;
struct sockaddr_in * serv_addr ;
int addr_size ;
char * mensaje ;
{
	
	int tamanio = strlen(mensaje);
	char buffer[tamanio];
	strcpy( buffer , mensaje );
	
	int error = sendto( sockfd ,
						buffer ,
						tamanio ,
						0 ,
					   (struct sockaddr *)serv_addr ,
					    addr_size );
		if ( error < 0 ) {
			fprintf( stderr ,
					"ERROR: No se pudo enviar el mensaje. (UDP)" );
			return -1;
		}
	
	return 0;
	
}

/**
 * @param tamanio : Tamaño del mensaje a leer por TCP.
 * @return Mensaje recibido por TCP.
 */
int Sockets_Leer_mensaje_TCP( int sockfd , char buffer[] , int tamanio )
{
	
	memset( buffer , '\0' , tamanio );
	
	///Compruebo la conexión:
	char c;
	ssize_t conexion = recv( sockfd , &c , 1 , MSG_PEEK );
		if( conexion < 1 ) {
			fprintf( stderr , "ERROR: Conexión %i perdida" , sockfd );
			return 1;
		}
	
	int error = read( sockfd , buffer , tamanio );
		if ( error < 0 ) {
			fprintf( stderr , "ERROR: No se pudo leer el mensaje. "
							  "(TCP)" );
			return 1;
		}
	
	return 0;
}

/**
 * @param tamanio : Tamaño del mensaje a leer por TCP.
 * @return Mensaje recibido por TCP.
 */
char * Sockets_Leer_mensaje_largo_TCP_FREE( int sockfd )
{
	
	char tamanio[TAM];
	if( Sockets_Leer_mensaje_TCP( sockfd , tamanio , TAM ) )
		return NULL;
	
	int tam = 0;
	sscanf( tamanio , "%i" , &tam );
	char * mensaje_largo = (char *)Mem_assign( tam * sizeof(char) );
	if( Sockets_Leer_mensaje_TCP( sockfd , mensaje_largo , tam ) )
		return NULL;
	
	mensaje_largo[tam] = '\0';
	
	return mensaje_largo;
	
}

/**
 * @param mensaje : Mensaje a enviar por TCP.
 */
int Sockets_Enviar_mensaje_TCP( int sockfdTCP , char * mensaje )
{
	
	int tamanio = strlen(mensaje);
	char buffer[tamanio];
	memset( buffer , '\0' , tamanio );
	strcpy( buffer , mensaje );
	
	int error = write( sockfdTCP , buffer , strlen(buffer) );
		if ( error < 0 ) {
			fprintf( stderr , "ERROR: No se pudo enviar el mensaje. "
							  "(TCP)" );
			return -1;
		}
	
	return 0;
	
}

/**
 * @brief Envia un mensaje de tamanio variable, enviando primero el
 * tamanio y luego el mensaje
 */
int Sockets_Enviar_mensaje_largo_TCP( int sockfdTCP , char * mensaje )
{
	
	unsigned int tam_mensaje = strlen(mensaje) + 1;
	
	char tamanio[TAM];
	memset( tamanio , '-' , TAM );
	sprintf( tamanio , "%i" , tam_mensaje );
	tamanio[ strlen(tamanio) ] = '-';
	
	Sockets_Enviar_mensaje_TCP( sockfdTCP , tamanio );
	
	return Sockets_Enviar_mensaje_TCP( sockfdTCP , mensaje );
	
}
 
/**
 * @brief A partir de los datos de una conexión UDP preestablecida,
 * envía un archivo por UDP\n
 * Protocolo: Primero envía el tamaño del archivo. En base a eso se 
 * calcula la cantidad de partes\n
 * \tPosteriormente envía una parte y recibe una confirmación 
 * (por ser UDP) hasta enviar todas las partes
 * 
 * @param nombre_del_archivo : Ruta del archivo a enviar
 * @param mostrar_porcentaje : Varaible de desición la cual al ser 
 * activada imprime el pocentaje de descarga transcurrido por cada 
 * parte por pantalla.
 * @return Si no puede leer el archivo retorna -1 (con error), de lo 
 * contrario 0 (sin error).
 */
int Sockets_Enviar_archivo_por_UDP
( sockfd , dest_addr , nombre_del_archivo , mostrar_porcentaje )
int sockfd ;
struct sockaddr_in dest_addr ;
char * nombre_del_archivo ;
int mostrar_porcentaje ;
{
	
	FILE * archivo = fopen( nombre_del_archivo , "rb" );
		if ( archivo == NULL ) {
			fprintf( stderr ,
					"El fichero solicitado (%s) no existe." ,
					 nombre_del_archivo );
			return -1;
		}
	
	int addr_size = sizeof( dest_addr );
	int tamanio_archivo = File_size(archivo);
	
	char mensaje[TAM];
	
	///Envía el tamaño como primer dato:
	memset( mensaje , 0 , TAM );
	snprintf( mensaje , TAM - 1 , "%d" , tamanio_archivo );
	mensaje[ strlen(mensaje) ] = '\0';
	Sockets_Enviar_mensaje_UDP( sockfd ,
							   &dest_addr ,
							    addr_size ,
							    mensaje );
	
	///Se envía una parte y se espera una confirmación,
	///hasta enviar todas las pertes, menos la última:
	int ciclo;
	int partes = ( tamanio_archivo / ( TAM - 1 ) ) + 1;
				 /* (TAM - 1): Debido a que el último será '\0' */
	for( ciclo = 1 ; ciclo < partes ; ciclo++ )
	{
		
		///Lee (TAM - 1) caracteres del archivo:
		fread( mensaje , 1 , TAM - 1 , archivo );
		mensaje[ TAM ] = '\0';
		
		///Envía una parte:
		Sockets_Enviar_mensaje_UDP( sockfd ,
								   &dest_addr ,
								    addr_size ,
								    mensaje );
		
		///Luego espera confirmación:
		Sockets_Leer_mensaje_UDP( sockfd ,
								  TAM ,
								 &dest_addr ,
								  addr_size );
		
		///Imprime el porcentaje:
		if( mostrar_porcentaje ) 
			printf( " %s: %3.2f %%\n" ,
					 nombre_del_archivo ,
					( (float)ciclo * 100.0 / (float)partes ) ) ;
		
	}
	
	///Envío de la última parte (Posee un tamaño diferente):
	int tamanio_ultimo_mensaje = tamanio_archivo
								 - ( ( TAM - 1 ) * ( partes - 1 ) );
	memset( mensaje , '\0' , TAM );
	fread( mensaje , 1 , tamanio_ultimo_mensaje , archivo );
		///Envío:
		Sockets_Enviar_mensaje_UDP( sockfd ,
								   &dest_addr ,
								    addr_size ,
								    mensaje );
		///Confirmación:
		Sockets_Leer_mensaje_UDP(sockfd , TAM , &dest_addr , addr_size);
		///Imprime el porcentaje.
		if(mostrar_porcentaje)
			printf( " %s: 100 %%\n", nombre_del_archivo ) ;
	
	fclose(archivo);
	
	return 0;
	
}

 
/**
 * @brief A partir de los datos de una conexión UDP preestablecida, 
 * recibe un archivo por UDP\n
 * Protocolo: Primero recibe el tamaño del archivo. En base a eso se 
 * calcula la cantidad de partes\n
 * \tPosteriormente recibe una parte y envía una confirmación 
 * (por ser UDP) hasta recibir todas las partes
 * 
 * @param nombre_del_archivo : Ruta para guardar los datos recibidos.
 * @param mostrar_porcentaje : Varaible de desición la cual al ser 
 * activada imprime el pocentaje de descarga transcurrido por cada parte
 * @return Si no puede guardar el archivo retorna 1 (con error), 
 * de lo contrario 0 (sin error).
 */
int Sockets_Recibir_archivo_por_UDP
( sockfdUDP , serv_addr , nombre_del_archivo , mostrar_porcentaje )
int sockfdUDP ;
struct sockaddr_in serv_addr ;
char * nombre_del_archivo ;
int mostrar_porcentaje;
{
	
	struct direccion dir;
	Sockets_Direccion_del_socket( sockfdUDP , &dir );
	Sockets_Direccion_del_sockaddr_in( serv_addr , &dir );
	
	remove( nombre_del_archivo );
	FILE * archivo = fopen( nombre_del_archivo , "wb" );
		if (archivo == NULL){
			fprintf( stderr ,
					"ERROR: No se pudo guardar.\n"
					"Controle que exista: %s" ,
					 nombre_del_archivo );
			return -1;
		}
	
	int addr_size = sizeof( struct sockaddr );
	
	///Recibir tamaño del archivo:
	char tamanio_str[TAM];
	Sockets_Leer_mensaje_UDP( sockfdUDP ,
							  tamanio_str ,
							  TAM ,
							 &serv_addr ,
							  addr_size );
	
	char tamanio_str_vect[strlen(tamanio_str)];
	strcpy( tamanio_str_vect , tamanio_str );
	int tamanio = atoi(tamanio_str_vect);
	int partes = ( tamanio / ( TAM - 1 ) ) + 1;
				 /* TAM - 1: Debido a que el último es '\0' */
	
	///Recepción y escritura del archivo.
	int ciclo;
	char buffer[TAM];
	memset( buffer , '\0' , TAM );
	for( ciclo = 1 ; ciclo <= partes ; ciclo++ )
	{
		
		///Recibe parte y guarda:
		char parte[TAM];
		Sockets_Leer_mensaje_UDP( sockfdUDP ,
								  parte ,
				  	  	  	  	  TAM ,
				  	  	  	  	 &serv_addr ,
				  	  	  	  	  addr_size );
		strcpy( buffer , parte );
		fwrite( buffer , 1 , strlen(buffer) , archivo );
		///Envía confirmación:
		Sockets_Enviar_mensaje_UDP( sockfdUDP ,
								   &serv_addr ,
								    addr_size ,
								    "0" ) ;
		///Imprime porcentaje de la tranferencia:
		if( mostrar_porcentaje )
			printf( "%s: %3.2f %%\n" ,
					 nombre_del_archivo ,
					(float)ciclo * 100.0 / (float)partes );
		
	}
	
	fclose(archivo);
	
	return 0;
	
}

#endif
