#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <mysql.h>
#include <string.h>



int main(int argc, char *argv[])
{
			MYSQL *conn;
			int err;
			MYSQL_RES *resultado;
			MYSQL_ROW row;
			conn = mysql_init(NULL);
			if (conn==NULL) 
			{
				printf ("Error al crear la conexion: %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			
			conn = mysql_real_connect (conn, "localhost","root", "mysql", "Juego",0, NULL, 0);
			if (conn==NULL) {
				printf ("Error al inicializar la conexion: %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
	int sock_conn, sock_listen, ret;
	struct sockaddr_in serv_adr;
	char peticion[512];
	char respuesta[512];
	// INICIALITZACIONS
	// Obrim el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	// Fem el bind al port
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	// asocia el socket a cualquiera de las IP de la m?quina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	// escucharemos en el port 9050
	serv_adr.sin_port = htons(9050);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	//La cola de peticiones pendientes no podr? ser superior a 4
	if (listen(sock_listen, 15) < 0)
		printf("Error en el Listen");
	int i;
	
	// Atenderemos infinitas peticiones
	
	for(;;){
		printf ("Escuchando\n");
		
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexi?n\n");
		//sock_conn es el socket que usaremos para este cliente
		
		// Ahora recibimos su peticion
		ret=read(sock_conn,peticion, sizeof(peticion));
		printf ("Recibida una petición\n");
		// Tenemos que a?adirle la marca de fin de string 
		// para que no escriba lo que hay despues en el buffer
		peticion[ret]='\0';
		
		//Escribimos la peticion en la consola
		
		printf ("La petición es: %s\n",peticion);
		char *p = strtok(peticion, "/");
		int codigo =  atoi (p);
		/*p = strtok( NULL, "/");
		char nombre[20];
		strcpy (nombre, p);
		printf ("Codigo: %d, Nombre: %s\n", codigo, nombre);*/
		
		//Ahora escribimos las consultas de la BBDD, cada una con un codigo distinto
		
		if (codigo ==1) 
		{ 
			
			p = strtok( NULL, "/");
			char nombre[20];
			strcpy (nombre, p);
			printf ("Codigo: %d, Nombre: %s\n", codigo, nombre);
			MYSQL *conn;
			int err;
			// Estructura especial para almacenar resultados de consultas
			MYSQL_RES *resultado;
			MYSQL_ROW row;
			int victorias;
			
			
			
			conn = mysql_init(NULL);
			if (conn==NULL) 
			{
				printf ("Error al crear la conexion: %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			
			conn = mysql_real_connect (conn, "localhost","root", "mysql", "Juego",0, NULL, 0);
			if (conn==NULL) {
				printf ("Error al inicializar la conexion: %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			
			
			err=mysql_query (conn, "SELECT distinct jugadores.username,jugadores.victorias FROM (jugadores) WHERE jugadores.victorias = (SELECT MAX(jugadores.victorias) FROM (jugadores))");
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			
			
			resultado = mysql_store_result (conn);
			row = mysql_fetch_row (resultado);
			
			
			if (row == NULL)
				printf ("No se han obtenido datos en la consulta\n");
			else
				printf ("Jugador que ha conseguido el mayor número de victorias:\n");	
			while (row !=NULL)
				
			{		
				victorias = atoi(row[1]);
				
				sprintf (respuesta,"Username del jugador:%s \n Numero de victorias conseguidas: %d\n", row[0],victorias);
				
				row = mysql_fetch_row (resultado);
			}
			printf ("Respuesta: %s %d\n",resultado);
			
			write (sock_conn,respuesta,strlen(respuesta));
			
		}	
	
	    else if (codigo == 2)
		{   
			int puntos;
			
			// consulta SQL para obtener una tabla con todos los datos
			// de la base de datos
			err=mysql_query (conn, "SELECT jugadores.username,jugadores.puntos FROM (jugadores)ORDER  BY puntos DESC");
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			
			resultado = mysql_store_result (conn);
			row = mysql_fetch_row (resultado);
			
			if (row == NULL)
				printf ("No se han obtenido datos en la consulta\n");
			else
				printf("Ranking de los jugadores \n");
			while (row !=NULL)
				
			{
				puntos = atoi (row[1]);
				printf ("Username: %s, puntos: %d\n", row[0], puntos);
				sprintf(respuesta,"%s / %s  %d ",respuesta,row[0],puntos);
				// obtenemos la siguiente fila
				row = mysql_fetch_row (resultado);
				
			}

			write  (sock_conn, respuesta,strlen(respuesta));
			//write (sock_conn,resultado,strlen(resultado));
		
		
		}
		else if (codigo==4)
		{
			p = strtok( NULL, "/");
			char nombre[20];
			strcpy (nombre, p);
			p = strtok(NULL, "/");
			char password[80];
			strcpy (password, p);
			printf ("Codigo: %d, Nombre: %s, password: %s\n", codigo, nombre, password);		
			err=mysql_query (conn, "SELECT jugadores.username,jugadores.password FROM jugadores WHERE jugadores.username =' ");
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			
			resultado = mysql_store_result (conn);
			row = mysql_fetch_row (resultado);
			
			if (row == NULL)
				printf ("No se han obtenido datos en la consulta\n");
			else
				printf ("Nombre y contraseña de los jugadores\n");
			while (row !=NULL)
			{
				
				printf ("Username: %s, Contraseña: %s \n", row[0], row[1]);
				// obtenemos la siguiente fila
				row = mysql_fetch_row (resultado);
				strcpy(respuesta,"Logueado correctamente");
			}
			write (sock_conn,respuesta, strlen(respuesta));
				
		}
		else if (codigo==3)
		{   
			p = strtok( NULL, "/");
			char nombre[20];
			strcpy (nombre, p);
			printf ("Codigo: %d, Nombre: %s\n", codigo, nombre);
			MYSQL *conn;
			int err;
			// Estructura especial para almacenar resultados de consultas 
			MYSQL_RES *resultado;
			MYSQL_ROW row;
			int partidas;
			int ganadas;
			
			char consulta [80];
			char consulta2 [80];
	
			printf("\n");
			//Creamos una conexion al servidor MYSQL 
			conn = mysql_init(NULL);
			if (conn==NULL)
			{
				printf ("Error al crear la conexion: %u %s\n", mysql_errno(conn), mysql_error(conn));			
				exit (1);
			}
			//inicializar la conexion
			conn = mysql_real_connect (conn, "localhost","root", "mysql", "Juego",0, NULL, 0);
			if (conn==NULL)
			{
				printf ("Error al inicializar la conexion: %u %s\n", 
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			
			// consulta SQL para obtener una tabla con todos los datos
			// de la base de datos
			strcpy (consulta,"SELECT COUNT(participacion.partidaid) FROM (jugadores,participacion) WHERE jugadores.username = '"); 
			strcat (consulta, nombre);
			strcat (consulta,"' AND jugadores.id = participacion.jugadorid;");
			
			err=mysql_query (conn, consulta);
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
	
			
			resultado = mysql_store_result (conn);
			row = mysql_fetch_row (resultado);
			
			if (row == NULL)
				printf ("No se han obtenido datos en la consulta\n");
			else
				while (row !=NULL){
					
					partidas = atoi (row[0]);

					row = mysql_fetch_row (resultado);
				}
			
			// consulta2 SQL para obtener una tabla con todos los datos
			// de la base de datos
			strcpy (consulta2,"SELECT COUNT(partidas.ganador) FROM partidas WHERE partidas.ganador='"); 
			strcat (consulta2, nombre);
			strcat (consulta2,"';");
				
			err=mysql_query (conn, consulta2);
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
				
			resultado = mysql_store_result (conn);
				
			row = mysql_fetch_row (resultado);
			
			if (row == NULL)
				printf ("No se han obtenido datos en la consulta\n");
			else
				while (row !=NULL)
				{
					 
					ganadas = atoi (row[0]);
					
					row = mysql_fetch_row (resultado);
				}
			
			float winrate = (float)ganadas / (float)partidas;
			
			printf ("El usuario %s ha jugado un total de %d partidas ganando %d.\n", nombre, partidas, ganadas);
			printf("El WINRATE de %s es del %.2f%\n",nombre,winrate);
						
			sprintf(resultado,"%.2f%\n",winrate);	
			
		}
		
		
		printf ("Respuesta: %s %d\n",resultado);
		// Enviamos la respuesta
		write (sock_conn,resultado,strlen(resultado));
		
		close(sock_conn); 
		mysql_close (conn);
	
		close(sock_conn); 
	}
}

