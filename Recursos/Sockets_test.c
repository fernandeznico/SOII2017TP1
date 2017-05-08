#include <stdio.h>

#include "Sockets.h"
#include "Error.h"

void Servidor()
{
	
	int servidor = 0;
	int puerto = 6020;
	Sockets_Crear_Socket_INET_TCP( &servidor , &puerto );
	
	int conexion;
	struct direccion dir;
	memset( &dir , '0' , sizeof( dir ) );
	struct sockaddr_in cli_addr;
	printf("\n Servidor disponible y a la espera de una conexión.");
	fflush( stdout );
	Sockets_Aceptar_un_cliente( &conexion ,
								servidor ,
								cli_addr ,
								&dir );
	
	///PROGRAMA:
	char * msj_re;
	msj_re = Sockets_Leer_mensaje_largo_TCP( conexion );
	
	printf( "\n msj_re=%s" , msj_re );
	
}

void Cliente()
{
	
	char * ip = "127.0.0.1";
	struct hostent * server = Sockets_Verificar_host_IPv4( ip );
	
	int sockfdTCP = Sockets_Crear_Y_Conectar_Socket_INET_TCP( server ,
															  6020 );
	
	printf( "\n Conexión establecida con el servidor AWS (%s:%i)" ,
			 ip ,
			 6020 );
	 
	///PROGRAMA:
	Sockets_Enviar_mensaje_largo_TCP( sockfdTCP , 
									 "97g12371g23198273gb81273g123g\n"
									 "7h018hgp21831g2'uh8f8s7df8df8\n"
									 "uhf087dsfhspdf8h7sd87fgs8dfgf\n"
									 "iushd879hsda0987sdha87sdha8s7\n"
									 "s87hasd7a8shdas8hdas987dhdsa9\n"
									 "uhf087dsfhspdf8h7sd87fgs8dfgf\n"
									 "uhf087dsfhspdf8h7sd87fgs8dfgf\n"
									 "uhf087dsfhspdf8h7sd87fgs8dfgf\n"
									 "uhf087dsfhspdf8h7sd87fgs8dfgf\n"
									 "uhf087dsfhspdf8h7sd87fgs8dfgf\n"
									 "uhf087dsfhspdf8h7sd87fgs8dfgf:" );
	
}

int main( int argc , char **argv )
{
	
	int flag = 1;
	
	while( flag )
	{
		
		printf( "\n Ingrese 's' para servidor o 'c' para cliente: " );
		char opcion = getc( stdin );
		
		switch( opcion )
		{
			
			case 's':
				Servidor();
				flag = 0;
				break;
				
			case 'c':
				Cliente();
				flag = 0;
				break;
				
		}
		
	}
	
	printf( "\n" );
	
	return 0;
	
}
