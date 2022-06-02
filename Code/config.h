/*Datos que se muestran al arrancar y cuando estan en pausa*/
/* #define se crean como variables globales alo largo del programa*/
#define LEAD_GAME "Lead Bare Metal"
#define LEAD_VERSION "1.0.0"
#define TETRIS_URL     "https://github.com/programble/bare-metal-tetris"

/*Dimensiones de LEAD*/
#define WELL_WIDTH (22)   // Ancho
#define WELL_HEIGHT (20)  // Alto
#define WELL_WIDTH2 (14)  // para nivel 2

/*Intervalos iniciales en ms en que aplicar la gravedad*/
#define INITIAL_SPEED (200)

/*Retraso en ms en que desaparecen enemigos*/
#define CLEAR_DELAY (100)

/* PUNTAJE: El puntaje se incrementa en 3 por cada enemigo eliminado*/
#define SCORE_FACTOR_1 (100)
#define SCORE_FACTOR_2 (300)
#define SCORE_FACTOR_3 (500)
#define SCORE_FACTOR_4 (800)

/* Amount to increase the score for a soft drop */
#define SOFT_DROP_SCORE (1)

/* Factor by which to multiply the number of rows dropped to increase the score
 * for a hard drop */
#define HARD_DROP_SCORE_FACTOR (2)

/* Number of rows that need to be cleared to increase level */
#define ROWS_PER_LEVEL (10)
