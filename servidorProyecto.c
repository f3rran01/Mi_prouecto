#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <mysql.h>
#include <string.h>



typedef struct 
{
	char nombre [20];
	int socket;
} Conectado;

typedef struct 
{
	Conectado conectados [100];
	int num;
} ListaConectados;


int PonConectado (ListaConectados *lista, char nombre, int socket)
{
	if (lista->num == 100)
		return -1;
		
	else
	{
		//lock//no se escribe asi, mirar en la leccion 3, tmb se debe añadir en la funcion eliminar
			lista->conectados[lista->num].socket = socket;
		strcpy (lista->conectados[lista->num].nombre, nombre);
		lista->num = lista->num +1;
		//unlock//no se escribe asi, mirar en la leccion 3, tmb se debe añadir en la funcion eliminar
		return 0;
		
	}
}


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
		int terminar =0;
		// Entramos en un bucle para atender todas las peticiones de este cliente
		//hasta que se desconecte
	while (terminar ==0)
	{
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
		char nombre[20];
		char password[20];
		if ((codigo !=0))
		{
			p = strtok( NULL, "/");
			strcpy (nombre, p);
			printf ("Codigo: %d, Nombre: %s\n", codigo, nombre);
			
		}
		
		if (codigo ==0) //petici?n de desconexi?n
			terminar=1;
		//Ahora escribimos las consultas de la BBDD, cada una con un codigo distinto
		
		else if (codigo ==1) 
		{ 
			int victorias;
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
			//printf ("Respuesta: %s %d\n",row[0],victorias);
			printf (respuesta);
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
			printf (respuesta);
		
			
			write  (sock_conn, respuesta,strlen(respuesta));
			//write (sock_conn,resultado,strlen(resultado));
		
		}
			
		else if (codigo==4)
		{
		
			p = strtok(NULL, "/");
			strcpy (password, p);
			printf ("Codigo: %d, Nombre: %s, password: %s\n", codigo, nombre, password);
			char consulta [100];
			strcpy (consulta,"SELECT jugadores.username FROM (jugadores) WHERE jugadores.username = '"); 
			strcat (consulta, nombre);
			strcat (consulta,"' AND jugadores.password = '");
			strcat (consulta, password);
			strcat (consulta, "';");
			printf("consulta = %s\n", consulta);
			err=mysql_query (conn, consulta);
			
			
			if (err!=0) {
				printf ("Error, nombre o contraseña incorrectos %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			
			resultado = mysql_store_result (conn);
			row = mysql_fetch_row (resultado);
			
			if (row == NULL)
			{
				printf ("Error, nombre o contraseña incorrectos\n");
				strcpy(respuesta,"Nombre o contraseña incorrectos\n");
			}
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
		
		else if (codigo ==5)
		{
			p = strtok( NULL, "/");
			strcpy (password, p);
			char consulta [100];
			
			strcpy(consulta, "INSERT INTO jugadores VALUES (NULL,'");
			
			strcat (consulta, nombre); 
			strcat (consulta, "'");
			strcat (consulta, ",'"); 
			strcat (consulta, password); 
			strcat (consulta, "');");
			err = mysql_query(conn, consulta);
			
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			

			resultado = mysql_store_result (conn);
			row = mysql_fetch_row (resultado);
	
			if (row == NULL)
			{
				printf ("Error, no se ha podido registrar\n");
				strcpy(respuesta,"No se ha podido registrar\n");
			}
			else
				
				while (row !=NULL)
				{
					printf ("Username: %s\n", row[1]);
					row = mysql_fetch_row (resultado);
					sprintf(respuesta, "Registrado %s,%s;", nombre, password);
				}
			write (sock_conn,respuesta, strlen(respuesta));
		}
		else if (codigo==3)
		{   
			
		
		int partidas;
			int ganadas;
			
			char consulta [80];
			char consulta2 [80];
	
			printf("\n");

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
			printf ("Respuesta: %s %d\n",resultado);
			// Enviamos la respuesta
			write (sock_conn,resultado,strlen(resultado));
		}
	}
		mysql_close (conn);
	
		close(sock_conn); 
	}
}

