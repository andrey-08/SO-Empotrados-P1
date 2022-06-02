#include "config.h" //incluye el archivo cabecera con las diferentes variables ya declaradas

typedef unsigned char       u8;  //varable de 8 bits sin bit reservado para signo
typedef signed char         s8;  //varable con bit reservado para signo
typedef unsigned short      u16; //Variable de 16 bits sin signo
typedef signed short        s16; 
typedef unsigned int        u32;
typedef signed int          s32;
typedef unsigned long long  u64; // Variable de 64 bits sin bit de signo
typedef signed long long    s64;

#define noreturn __attribute__((noreturn)) void

typedef enum bool {
	false,
	true
}bool;

//Algoritmo exponencial con la funcion "pow()"
static inline double pow(double a, double b){ // a elevado a b
	double result = 1;
	while(b-- > 0)
		result *= a;
	return result;
}

/* Port I/O */

static inline u8 inb(u16 p){
	u8 r;
	asm("inb %1, %0" : "=a" (r) : "dN" (p));
	return r;
}

static inline void outb (u16 p, u8 d){
	asm("outb %1, %0" : : "dN" (p), "a" (d));
}

/* Divide por 0(en un bucle para satisfacer el atributo de noreturn) para activar
una division por cero ISR, que no se controla y provoca un restablecimiento completo*/

noreturn reset (void){
	volatile u8 one = 1, zero = 0;
	while (true)
		one /= zero;
}

/* Timing */

/*Devuelve el # de ticks de la CPU desde el inicio */
static inline u64 rdtsc(void){
	u32 hi, lo;
	asm("rdtsc" : "=a" (lo), "=d" (hi));
	return ((u64) lo) | (((u64) hi) << 32);
}

/* Devuelve el segundo campo enemigo[lyd] de el tiempo real del reloj (RTC)
	Tenga en cuenta que el valor puede o no estar representado de tal manera que
	debe formatearse en hexadecimal para mostrar al segundo enemigo[lyd]
	(Es decir, 0x30 para el segundo 30) */

u8 rtcs(void){
	u8 last = 0, sec;
	do { /* Hasta que el valor sea el mismo 2 veces*/
		/*Esperar a que la actualizacion no este en curso*/
		do{ 
			outb(0x70, 0x0A); 
		} while (inb(0x71) & 0x80);
		outb(0x70, 0x00);
		sec = inb(0x71); 
	}while (sec != last && (last = sec) );
	return sec;
}

/* EL numero de ticks de CPU por milisegundos */
u64 tpms;

/* Establezca el numeros de ticks por milisegundos del CPU basado en el numero de
	ticks en el ultimo segundo, si el segundo ha cambiado desde la ultima llamada.
	Esto se llama en cada iteracion del bucle principal para proporcionar una 
	sincronizacion precisa*/

void tps (void){

	static u64 ti = 0;
	static u8 last_sec= 0xFF;
	u8 sec= rtcs();
	if (sec != last_sec){
		last_sec = sec;
		u64 tf = rdtsc();
		tpms = (u32) ((tf - ti) >> 3) / 125; //Menos posibilidades de truncamiento
		ti = tf;
	}
}

/* IDs utilizados para mantener separados los tiempos de operacion*/
enum timer{
	TIMER_UPDATE,
	TIMER_CLEAR,
	TIMER_LENGTH
};

u64 timers[TIMER_LENGTH] = {0};

/*Retorna TRUE si han transcurrido al menos ms desde la ultima llamada
  que retorno TRUE para este temporizador. Cuando se llama en iteracion
  del bucle principal, tiene el efecto de devolver TRUE una vez cada milisegundo*/
bool interval (enum timer timer, u32 ms){
	u64 tf = rdtsc();
	if (tf - timers[timer] >= tpms * ms){
		timers[timer] = tf;
		return true;
	}
	else return false;
}

/* Retorna verdadero si han transcurrido al menos ms dede la primera llamada
	para este temporizador y reinicia el temporizador*/

bool wait(enum timer timer, u32 ms){
	if (timers[timer]){
		if (rdtsc() - timers [timer >= tpms * ms]) {
			timers[timer] = 0;
			return true;
		}
		else return false;
	}
	else{
		timers[timer] = rdtsc();
		return false;
	}
}

/* Video Output */

/* 7 posibles colores de visualizacion, se pueden hacer tonos mas brillantes
   con (i.e. BRIGHT | BLUE) */

enum color { // variable que define todos los posibles valores que pueda contener
	BLACK,	//0
	BLUE,		//1
	GREEN,	//2
	CYAN,		//3
	RED,		//4
	MAGENTA,	//5
	YELLOW,	//6
	GRAY,		//7
	BRIGHT	//8
};

#define COLS (80) // eje X
#define ROWS (25) // eje y
u16 *const video = (u16*) 0xB8000;

/* Muestra un caracter en x, y en color de primer plano fg(foreground) y color
   de fondo bg(background).*/

void putc(u8 x, u8 y, enum color fg, enum color bg, char c){
	u16 z= (bg << 12) | (fg << 8) | c;  // recordand que << es un desplazamiento y | es or
	video[y * COLS + x] = z;
}

/* Muestra una cadena que comienza en "x", "y" en color fb y bg. Los caracteres
	del string no se interpretan (por ejemplo, \ n, \ b, \ t, etc.)*/

void puts(u8 x, u8 y, enum color fg, enum color bg, const char *s){
	for (; *s; s++, x++)
		putc(x, y, fg, bg, *s); 

	/*(x: posici[on en x, y: posici[on en y, fg: primer capa, bg: capa el fondo, *s: caracter a mostrar en pantalla])
	*/
}


/* Limpia la pantalla para mostrar el color bg (background). */

void clear (enum color bg){
	u8 x, y;
	for (y = 0; y < ROWS; y++)
		for (x = 0; x < COLS; x++)
			putc(x, y, bg, bg, ' ');
}




/* Dibuja en pantalla la informacion general (Portada del juego)*/

/* #define TITLE_X (COLS/ 2 - 9)
	#define TITLE_Y (ROWS/ 2 - 10)*/

void draw_about(void){ 
	//puts(u8 x, u8 y, enum color fg, enum color bg, const char *s)  

	/*Dibuja parte superior de portada LEAD*/
	puts(34, 3, BLACK, BLUE, "            ");
	puts(34, 4, BLACK, BLUE, " ");
	puts(45, 4, BLACK, BLUE, " ");
	puts(34, 5, BLACK, BLUE, " ");
	puts(45, 5, BLACK, BLUE, " ");
	puts(35, 5, GRAY, BLACK, "   LEAD   ");
	puts(34, 6, BLACK, BLUE, " ");
	puts(45, 6, BLACK, BLUE, " ");
	puts(34, 7, BLACK, BLUE, "            ");

	/*Informacion de portada*/
	puts(4, 14, GRAY, BLACK, "Instituto Tecnologico de Costa Rica");
	puts(4, 16, GRAY, BLACK, "SO Empotrados");
	puts(4, 18, GRAY, BLACK, "Jose Andrey Sequeira Ruiz");
	puts(4, 20, GRAY, BLACK, "Profesor: Ernesto Rivera");

	/*Dibuja Nave en portada*/

	
	/*puts(61, 17, BLACK, BLUE,    "   ");
	puts(58, 18, BLACK, BLUE, "         ");*/

	puts(49, 20, BRIGHT|GREEN, BLACK, "Presione ENTER para continuar");
}


/* Input del teclado */
#define KEY_R     (0x13) // for reset
#define KEY_P     (0x19) // for pause
#define KEY_LEFT  (0x4B) // for moving left
#define KEY_RIGHT (0x4D) // for moving right
#define KEY_ENTER (0x1C) // for enter game
#define KEY_SPACE (0x39) // for shooting

/* Devuelve el codigo de escaneo de la tecla actual, si ha cambiado desde l ultima llamada
	de lo contrarioi devuelve 0. Cuando se llama en cada iteracion del bucle principal devuelve
	un valor distinto de cero en un evento de tecla*/

u8 scan (void){
	static u8 key = 0;
	u8 scan = inb(0x60); 	// Entrada para lectura del teclado
	if (scan != key) 			// Si es diferente de 0 se presiona tecla
		return key = scan;	// Por lo que retorna el valor de la tecla presionada
	else return 0;
}

/* Formateo */

/* Formatee n en el radio r (2-16) como una cadena de longitud w*/

char *itoa(u32 n, u8 r, u8 w){
	static const char d[16] = "0123456789ABCDEF";
	static char s[34];
	s[33]=0;
	u8 i=33;
	do{
		i--;
		s[i]=d[n % r];
		n /= r;
	} while (i > 33 - w);
	return (char *) (s+i);
}

/*Random*/

/* Genera un # aleatorio de 0 inclusivo a un rango exclusivo del numero
	de ticks del CPU desde el inicio*/
u32 rand(u32 range){
	return (u32) rdtsc() % range;
}

/* Mezcla una matriz de bytes (arr) de longitud (len) en lugar usando Fisher- Yates*/

void shuffle(u8 arr[], u32 len){
	u32 i, j;
	u8 t;
	for (i= len-1; i>0; i--){
		j = rand(i+1);
		t=arr[i];
		arr[i]= arr[j];
		arr[j]=t;
	}
}

/* Para creacion de naves (enemigos y jugador)*/

u8 ships[5][2][3] = {
	/*Los numeros de estas listas estan relacionados segun el #color de "num color"*/
 	{ //Nave jugador Azul

 		{0,1,0}, 
 		{1,1,1}
 	},
 	{ //Enemigo color rojo
 		{4,0,4},
 		{0,4,0}
 	},
 	{ //Enemigo color CYAN
 		{3,0,3},
 		{0,3,0}
 	},
 	{ //Enemigo color amarillo
 		{6,0,6},
 		{0,6,0}
 	},
 	{ //Enemigo color verde
 		{2,0,2},
 		{0,2,0}
 	}
};

u8 meteo[1][1][2] = {
	{
		{6,6}
	}
};


// 			puts(WELL_X+player.x * 2 + x*2, player.y + y, YELLOW, ships[player.i][y][x], "#");
// x=0 y=0;				18 + 22 + 0*2 = 40, 	11+0=11,				Y,			ships[0][0][0]
// x=1 y=0;       	18 + 22 + 1*2 = 42, 	   11+0=,				Y,			ships[0][0][0]  


/* Estructura para informacion de naves (Player y enemigos)*/
struct ship_inf{ 
	u8 i;    		// Escoger que nave pintar, Enemigo o player.
	s8 x, y; 		// Posicion de la nave
	bool estado;	// Estado de la nave (presente o no), bool ya que va a cont T o F
};

/* Se usa una logica parecida a la nave pero para LA BALA */

struct bullet_ship{
	s8 x, y; //solo hay movimiento en y y x para ubicar
	bool estado; // si bala existe o no existe
};

struct meteorite{
	u8 i;
	s8 x, y;
	bool estado;
};


struct wall_loc{
	s8 x, y;
	bool direccion;
};

struct wall_loc wall_I[18];
struct wall_loc wall_D[18];


struct ship_inf player; /* Hacemos que jugador sea una estructura conformada por la estructura ship_inf*/
struct bullet_ship bullet[5]; // un array para que se puedan disparar solo 5 balas seguidas.
struct meteorite met[3];

#define REFER_MAX  (31) //(COLS/2 - WELL_WIDTH2) 
#define REFER_MIN  (21) //(REFER_MAX-10)	        

#define REFER_MAXD  (59)//(COLS/2 + WELL_WIDTH2) //54
#define REFER_MIND  (49)//(REFER_MAXD - 10)      //44 49

s8 posicion_x= REFER_MAX;
s8 posicion_xD= REFER_MAXD;


/* Pueba con un solo enemigo, primero se prueba con un solo enemigo para ver el correcto funcionamiento y luego
	cambiamos el codigo para que sean varios enemigos*/
struct ship_inf enemy[4];

u32 speed= INITIAL_SPEED, score=0, lives=4, level=1;

/* Funcion para detectar colision con paredes de la zona de juego*/

bool collide(s8 x, s8 y){
	if (x<0 || x>(WELL_WIDTH-3) || y<2 || y>WELL_HEIGHT)
		return true;
	else
		return false;
}


bool collide_E(s8 y){
	if ( y<2 || y>WELL_HEIGHT)
		return true;
	else
		return false;
}

bool collide_l2(s8 x){

	if (x <= (wall_I[17].x ) || (x+3) >= wall_D[17].x ){
		lives -= 1;
		player.estado = false;
		return true;
	}
	else
		return false;
}

bool collide_met(s8 y){
	if(y<1 || y>WELL_HEIGHT){
		return true;
	}
	else
		return false;
}


///////////// Funciones para la deteccion de colision /////////////////////

/* colision bala con enemigo*/

void colision_B_E(void){
/* Primero hacemos un for que recorra cada una de las balas, donde verifique si esa bala ha impactado
	a alguno de los enemigos, enemigos que se recorren con otro for*/

	for(int yy=0; yy<5; yy++){
		if(bullet[yy].estado){
			for(int xx=0; xx<4; xx++){
				if((bullet[yy].y<=(enemy[xx].y+2))){
					if((bullet[yy].x>=enemy[xx].x)&&(bullet[yy].x<=(enemy[xx].x+2))){
						enemy[xx].estado=false;
						bullet[yy].estado=false;
						score += 1;
					}
				}

			}
		}
	}
}

/* Colision enemigo con jugador*/

void colision_E_P(void){
	for(int e=0; e<4; e++){
		if(enemy[e].estado){
			if(((enemy[e].x>=player.x)&&(enemy[e].x<(player.x + 3))) || ((enemy[e].x+3)<=(player.x+3))&&((enemy[e].x+3)> player.x)){
				if((enemy[e].y+2)>=player.y){
					enemy[e].estado=false;
					player.estado=false;
					lives -=1;
				}
			}
		}
	}
}

void colision_M_P(void){

	for(int x=0; x<3; x++){
		if(met[x].estado){
			if(((met[x].x>=player.x)&&(met[x].x<(player.x+3)))||((met[x].x+2)<=(player.x+3))&&((met[x].x+3)>player.x)){
				if((met[x].y+2)>=player.y){
					met[x].estado=false;
					player.estado=false;
					lives -=1;
				}
			}
		}
	}
}

/* Funcion para inicializar los diferentes aspectos de los personajes (player, enemigo y bala)*/

void init(void){
	player.i=0;
	player.y=WELL_HEIGHT;  // 20
	player.x=(WELL_WIDTH/2); // 11
	player.estado= false;

	/* Inicializar los valores de las balas*/
	/* se realiza con un for debido a que es un array */
	for(int xx=0; xx<5; xx++){
		bullet[xx].y= player.y - 1;
		bullet[xx].x= player.x + 1;
		bullet[xx].estado = false;
	}
	/* Inicializar los valores de los enemigos*/
	for(int ee=0; ee<4; ee++){
		enemy[ee].i= ee+1;
		enemy[ee].y= 2;
		enemy[ee].x= 3 + ee*4;
		enemy[ee].estado=false;
	}

	/// Valore necesarios a inicializar a default ///
	posicion_x= REFER_MAX;
	posicion_xD= REFER_MAXD;
	level=1;
	score=0;
}

/* Se crea una funcion que permita ver el estado de la nave o Bala para saber si
	si tiene que spawnear otra segun el estado*/
void spawnear (void){

	if(player.estado==false){
		player.y=WELL_HEIGHT;  // 
		player.x=(WELL_WIDTH/2 - 1); // 10
		player.estado= true;
	}

	for(int ee = 0; ee<4; ee++){
		if(enemy[ee].estado==false){
			enemy[ee].y = 2;
			enemy[ee].x = 3 + ee*4;;
			enemy[0].estado= true;
		}
	}

	
}

void spawnear2(void){
	if(player.estado==false){
		player.y=WELL_HEIGHT;  
		player.x=(COLS/2 -1); 
		player.estado= true;
	}

	for(int m=0; m<3; m++){
		if(met[m].estado==false){
			met[m].x=(COLS/2 - 7) + m*6;
			met[m].y= 3;
			met[0].estado=true;
		}

	}

}

#define WELL_X  (COLS / 2 - WELL_WIDTH) // (80/2 - 22)= 18
#define WELL_X2 (COLS / 2 - WELL_WIDTH2) // (80/2 - 14)= 26

/*#define STATUS_X (COLS * 3/4)
#define STATUS_Y (ROWS /2 -4)*/

#define SCORE_X (COLS/2 - 4)
#define SCORE_Y (ROWS-2)

#define LIVES_X (3)
#define LIVES_Y (SCORE_Y-2)

/*Ahora creamos una funcion que permita dibujar los componentes del juego*/

s8 move_wall=0;

void draw(void){
	clear(BLACK);
	u8 x, y;

	/* Pintar los bordes del area dejuego */

	for (y=2; y<WELL_HEIGHT; y++){
		putc(WELL_X-1, y, BLACK, GRAY, ' '); //Pared izquierda
		putc(COLS / 2 + WELL_WIDTH, y, BLACK, GRAY, ' '); //Pared Derecha
	}

	// Para crear efecto de movimiento en las paredes//
	for (y=move_wall; y<WELL_HEIGHT; y+=2){
		putc(WELL_X-1, y, GRAY, BLACK, ' '); //Pared izquierda
		putc(COLS / 2 + WELL_WIDTH, y, GRAY, BLACK, ' '); //Pared Derecha
	}

	if(move_wall < 5)
		move_wall +=1;
	else
		move_wall=0;


	/*Se corrobora el estado de la nave*/
	if(player.estado == true){
		for(y=0; y<2; y++){
			for(x=0; x<3; x++){
				if (ships[player.i][y][x])
					puts(WELL_X+player.x * 2 + 2*x, player.y + y, YELLOW, ships[player.i][y][x], "#");
			}
		}
	}

	/* Codigo para el pintado de la bala, misma logica del movimiento del jugador*/

	for(int bb = 0; bb < 5; bb++){
		if(bullet[bb].estado == true)
			//puts(bullet[bb].x, bullet[bb].y, GRAY, BLACK, "||");
			puts(WELL_X + bullet[bb].x*2, bullet[bb].y, GRAY, BLACK, "|");
	}

	for(int ee=0; ee<4; ee++){
		if(enemy[ee].estado == true){
			for(y=0; y < 2; y++){
				for(x=0; x < 3; x++){
					if(ships[enemy[ee].i][y][x])
						if(y==0)
							puts(WELL_X+enemy[ee].x*2 + 2*x, enemy[ee].y + y, ships[enemy[ee].i][y][x], BLACK, "_" );
						else
							puts(WELL_X+enemy[ee].x*2 + 2*x, enemy[ee].y + y, ships[enemy[ee].i][y][x], BLACK, "V" );
				}
			}
		}
	}

	/*Mostrar informacion en la pantalla de juego*/
	status:
		// SCORE //
		puts(SCORE_X - 4, SCORE_Y, GRAY, BLACK, "SCORE:");
		puts(SCORE_X+5, SCORE_Y, BRIGHT|BLUE, BLACK, itoa(score, 10, 5));

		// VIDAS //
		puts(LIVES_X, SCORE_Y, GRAY, BLACK, "LIVES:");
		puts(LIVES_X+9, SCORE_Y, BRIGHT|RED, BLACK, itoa(lives, 10, 1));
}
 
////////////////// Funcion para dibujar zona de juego del nivel 2 /////////////////////


/* Para inicializar valores en la estructura de paredes*/

void init_2(void){ 
	u8 x, y;
	u8 cont=0;
	for(x=0; x<18; x++){
		if(posicion_x >= REFER_MIN && cont < 10){
			posicion_x = posicion_x - 1;
			posicion_xD= posicion_xD - 1;
			wall_I[x].direccion=true;
			wall_D[x].direccion=true;
			cont +=1;
		}
		if(posicion_x <= REFER_MAX && cont >= 10){
			posicion_x = posicion_x + 1;
			posicion_xD= posicion_xD + 1;
			wall_I[x].direccion=false;
			wall_D[x].direccion=false;
			cont +=1;
		}
		wall_I[x].x=posicion_x;
		wall_I[x].y= 3 + x;
		wall_D[x].x= posicion_xD;
		wall_D[x].y= 3 + x;

		if (cont == 19)
			cont = 0;
	}
 /// Iniciar valores del jugador /// 
	player.i=0;
	player.y=WELL_HEIGHT;  
	player.x=(COLS/2 - 1); 
	player.estado= false;

 /// Iniciar valores del meteorito /// 
	for(int m=0; m<3; m++){
			met[m].x=(COLS/2 - 7)+ m*6;
			met[m].y= 3;
			met[m].estado=false;
			met[m].i=0;
	}
	
}


void draw_2(){
	clear(BLACK);
	u8 x, y;

////// Para movimiento de paredes del mapa ///////
	for(x=0; x<18; x++){
		putc(wall_I[x].x, wall_I[x].y, BLACK, GRAY, ' ');
		putc(wall_D[x].x, wall_D[x].y, BLACK, GRAY, ' ');
	}

	for(x=0; x<18; x++){

		if(wall_I[x].x >= REFER_MIN && wall_I[x].direccion==true){
			wall_I[x].x -=1;
		}

		if(wall_I[x].x <= REFER_MAX && wall_I[x].direccion==false){
			wall_I[x].x +=1;
		}

		if(wall_I[x].x == REFER_MIN)
			wall_I[x].direccion = false;
		if(wall_I[x].x == REFER_MAX)
			wall_I[x].direccion = true;
		
	}

	for(x=0; x<18; x++){
		
		if(wall_D[x].x >= REFER_MIND && wall_D[x].direccion==true){
			wall_D[x].x -=1;
		}

		if(wall_D[x].x <= REFER_MAXD && wall_D[x].direccion==false){
			wall_D[x].x +=1;
		}

		if(wall_D[x].x == REFER_MIND)
			wall_D[x].direccion = false;
		if(wall_D[x].x == REFER_MAXD)
			wall_D[x].direccion = true;
	}

	//////////// Para dibujar nave player //////////////

	/*Se corrobora el estado de la nave*/
	if(player.estado == true){
		for(y=0; y<2; y++){
			for(x=0; x<3; x++){
				if (ships[player.i][y][x])
					puts(player.x+x, player.y + y, YELLOW, ships[player.i][y][x], "#");
			}
		}
	}

	//////////// Para dibujar meteorito //////////////

	for(int m=0; m<3; m++){
		if(met[m].estado == true){
			for(y=0; y<1; y++){
				for(x=0; x<2; x++){
					if (meteo[met[m].i][y][x])
						puts(met[m].x + x, met[m].y + y, BRIGHT|meteo[met[m].i][y][x], BLACK, "X" );
				}
			}
		}
	}

	status2:

		// SCORE //
		puts(SCORE_X - 4, SCORE_Y, GRAY, BLACK, "SCORE:");
		puts(SCORE_X+5, SCORE_Y, BRIGHT|BLUE, BLACK, itoa(score, 10, 5));

		// VIDAS //
		puts(LIVES_X, SCORE_Y, GRAY, BLACK, "LIVES:");
		puts(LIVES_X+9, SCORE_Y, BRIGHT|RED, BLACK, itoa(lives, 10, 1));

}

////////// Funciones de movimiento //////////

/* Intenta mover la nave una distancia dx y dy, devuelve true si tiene exito*/

bool move_player(s8 dx, s8 dy){
	if(!(player.estado))
		return false;
	if(collide(player.x + dx, player.y+dy)){
		return false;
	}
	player.x += dx;
	player.y += dy;
	return true;
}

bool move_player2(s8 dx, s8 dy){
	if(!(player.estado))
		return false;
	 // Movimiento de nave player en caso de que sea en nivel 2
	if(collide_l2(player.x + dx))
		return false;
	player.x += dx;
	return true;
}

bool move_bullet(s8 dx, s8 dy, s8 b){
	if (!(bullet[b].estado))
		return false;
	if (collide(bullet[b].x + dx, bullet[b].y + dy))
		return false;
	bullet[b].x += dx;
	bullet[b].y += dy;
	return true;
}

/* Prueba de enemigo*/
bool move_enemy(s8 dx, s8 dy, s8 e){
	if(!(enemy[e].estado))
		return false;
	if(collide_E(enemy[e].y+dy)){
		lives -=1;
		return false;
	}
	enemy[e].x += dx;
	enemy[e].y += dy;
	return true;
}


bool move_meteo(s8 dx, s8 dy, s8 m){
	if(!(met[m].estado))
		return false;
	if(collide_met(met[m].y+dy)){
		score += 1;
		return false;
	}
	met[m].y += dy;
	met[m].x += dx;
	
	return true;
}

/* Funcion que permite colocar el estado de la bala en True en caso de que se dispare
	eso sucede cuando la funcion se llama*/
void disparar(void){
	for(int bb = 0; bb<5; bb++){
		if(bullet[bb].estado==false){
			bullet[bb].estado=true;
			bullet[bb].x = player.x + 1;
			bullet[bb].y = player.y -1;
			return;
		}
	}
}

/* Funcion para actualizar el estado de ciertos elementos como:
	movimiento de bala y de los enemigos*/
void update(void){
	for(int bb=0; bb<5; bb++){
		if(!(move_bullet(0,-1, bb)))
			bullet[bb].estado=false;
	}

	for(int ee=0; ee<4; ee++){
		if(!(move_enemy(0, 1, ee)))
			enemy[ee].estado=false;
		if(enemy[3].y==4)
			enemy[1].estado=true;
		if(enemy[1].y==5)
			enemy[2].estado=true;
		if(enemy[0].y==5)
			enemy[3].estado=true;
	}

}

void update2(void){
	for(int m=0; m<3; m++){
		if(!(move_meteo(0,1, m)))
			met[m].estado=false;
		if(met[0].y==10)
			met[1].estado=true;
		if(met[1].y==9)
			met[2].estado=true;
	}
}

/* Funcion para detectar cuando se ha perdido el juego GAME OVER */

bool game_over(){
	if(lives==0){
		lives=4;
		return true;
	}
	return false;
}

void draw_GameOver(void){
	puts(COLS/2 - 8 , WELL_HEIGHT/2, BRIGHT|GREEN, BLACK, "G A M E   O V E R !!");
}

/* Funcion para detectar cambio de nivel*/
bool next_level(s8 l){
	if(score==13 && l==1)
		return true;
	if(score==26 && l==2)
		return true;
	
	return false;
}

void drawlevel_2(void){
	puts(COLS/2 - 6 , WELL_HEIGHT/2, BRIGHT|GREEN, BLACK, "L E V E L  2");
}

void draw_win(void){
	puts(COLS/2 - 7 , WELL_HEIGHT/2, BRIGHT|GREEN, BLACK, "Y O U  W I N !!!");
}


/////////// Funcion principal del juego /////////////////

noreturn kernel_main(){ 

begin:
	clear(BLACK);
	draw_about();
	init();

	/*Deteccion de tecla para iniciar*/
	u8 key, ult_tecla;
	int out1=0;
	while (out1 != 1){
		if ((key = scan())){
			if (key == KEY_ENTER)
				out1 = 1;
		}
	}

	/* Espera un segundo completo para calibrar el tiempo*/
	u32 itpms;
	tps();
	itpms=tpms;
	while (tpms==itpms)
		tps();
	itpms=tpms;
	while (tpms==itpms)
		tps();

	clear(BLACK);
	spawnear();
	draw();

loop:	
	tps();
	bool updated = false;

	if ((key=scan())){
		ult_tecla=key;

		switch (key){
			case KEY_RIGHT:
				move_player(2,0);
				break;

			case KEY_LEFT:
				move_player(-2,0);
				break;

			case KEY_SPACE:
				disparar();
				break;
		}
		updated = true;
	}

	if(interval(TIMER_UPDATE, speed)){
		update();
		spawnear();
		updated=true;

	}

	if(updated){
		draw();
		colision_B_E();
		colision_E_P();

		if(game_over()){ // Comprueba si hemos perdido todas las vidas
			clear(BLACK);
			draw_GameOver();

			itpms=tpms;
			while (tpms==itpms)
				tps();
			itpms=tpms;
			while (tpms==itpms)
				tps();

			goto begin;
		}

		if(next_level(1)){
			clear(BLACK);
			goto loop2;

		}
	}

	goto loop;

	loop2:
		
		drawlevel_2();
		init_2();

		tps();
		itpms=tpms;
		while (tpms==itpms)
			tps();
		itpms=tpms;
		while (tpms==itpms)
			tps();

		clear(BLACK);
		spawnear2();
		draw_2();

	loop2_1:

		tps();
		bool updated2 = false;

		if ((key=scan())){
			ult_tecla=key;

			switch (key){
				case KEY_RIGHT:
					move_player2(2,0);
					break;

				case KEY_LEFT:
					move_player2(-2,0);
					break;

				// case KEY_SPACE:
				// 	disparar();
				// 	break;
			}
			//updated2 = true;
		}

		if(interval(TIMER_UPDATE, speed)){
			update2();
			spawnear2();
			updated2=true;
		}

		if (updated2){
			draw_2();
			colision_M_P();

			if(game_over()){ // Comprueba si hemos perdido todas las vidas
				clear(BLACK);
				draw_GameOver();

				itpms=tpms;
				while (tpms==itpms)
					tps();
				itpms=tpms;
				while (tpms==itpms)
					tps();
				goto begin;
			}
			if(next_level(2)){
				clear(BLACK);
				draw_win();

				itpms=tpms;
				while (tpms==itpms)
					tps();
				itpms=tpms;
				while (tpms==itpms)
					tps();

				goto begin;

			}
		}

	goto loop2_1;

}
