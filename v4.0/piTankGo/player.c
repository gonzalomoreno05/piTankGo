
#include "player.h"
#include <string.h>
#include <softTone.h>

//------------------------------------------------------
// PROCEDIMIENTOS DE INICIALIZACION DE LOS OBJETOS ESPECIFICOS
//------------------------------------------------------


int InicializaEfecto (TipoEfecto *p_efecto, char *nombre, int *array_frecuencias, int *array_duraciones, int num_notas) {
	int i;

	for(i=0; i<num_notas; i++){
		p_efecto->frecuencias[i] = array_frecuencias[i];
		p_efecto->duraciones[i] = array_duraciones[i];
	}

	strcpy(p_efecto->nombre , nombre);

	p_efecto -> num_notas = num_notas;
	piLock(STD_IO_BUFFER_KEY);
	printf("[%i]\n", p_efecto->num_notas);
	piUnlock(STD_IO_BUFFER_KEY);

	return p_efecto->num_notas;
}

// Procedimiento de inicializacion del objeto especifico
// Nota: parte inicialización común a InicializaPlayDisparo y InicializaPlayImpacto
void InicializaPlayer (TipoPlayer *p_player) {

	p_player -> posicion_nota_actual = 0;
	p_player->p_efecto = &p_player->efecto_disparo;
	p_player -> frecuencia_nota_actual = p_player->p_efecto->frecuencias[0];
	p_player -> duracion_nota_actual = p_player->p_efecto->duraciones[0];
}

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

int CompruebaStartDisparo (fsm_t* this) {
	int result;

	piLock(PLAYER_FLAGS_KEY);
	result = (flags_player & FLAG_START_DISPARO);
	piUnlock(PLAYER_FLAGS_KEY);

	return result;
}

int CompruebaStartImpacto (fsm_t* this) {
	int result;

	piLock(PLAYER_FLAGS_KEY);
	result = (flags_player & FLAG_START_IMPACTO);
	piUnlock(PLAYER_FLAGS_KEY);

	return result;
}

int CompruebaNuevaNota (fsm_t* this){
	int result = 0;

	piLock(PLAYER_FLAGS_KEY);
	result = (flags_player & FLAG_PLAYER_END);
	piUnlock(PLAYER_FLAGS_KEY);

	return !result;
}

int CompruebaNotaTimeout (fsm_t* this) {
	int result;

	piLock(PLAYER_FLAGS_KEY);
	result = (flags_player & FLAG_NOTA_TIMEOUT);
	piUnlock(PLAYER_FLAGS_KEY);

	return result;
}

int CompruebaFinalEfecto (fsm_t* this) {
	int result = 0;

	piLock(PLAYER_FLAGS_KEY);
	result = (flags_player & FLAG_PLAYER_END);
	piUnlock(PLAYER_FLAGS_KEY);

	return result;
}

//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

void InicializaPlayDisparo (fsm_t* this) {

	TipoPlayer *p_player;

	piLock(PLAYER_FLAGS_KEY);
	flags_player &= ~FLAG_START_DISPARO;
	flags_player &= ~FLAG_START_IMPACTO;
	flags_player &= ~FLAG_NOTA_TIMEOUT;
	flags_player &= ~FLAG_PLAYER_STOP;
	flags_player &= ~FLAG_PLAYER_END;
	piUnlock(PLAYER_FLAGS_KEY);

	p_player = (TipoPlayer*) (this->user_data);
	p_player->p_efecto = &p_player->efecto_disparo;
	p_player -> posicion_nota_actual = 0;
	p_player -> duracion_nota_actual = p_player->efecto_disparo.duraciones[0];
	p_player -> frecuencia_nota_actual = p_player->efecto_disparo.frecuencias[0];

	tmr_startms(p_player->tmr, p_player->duracion_nota_actual);
	softToneWrite(23, p_player->frecuencia_nota_actual);

	piLock(STD_IO_BUFFER_KEY);
	printf("InicializaPlayDisparo\n");
	piUnlock(STD_IO_BUFFER_KEY);

}

void InicializaPlayImpacto (fsm_t* this) {

	TipoPlayer *p_player;

	piLock(PLAYER_FLAGS_KEY);
	flags_player &= ~FLAG_START_DISPARO;
	flags_player &= ~FLAG_START_IMPACTO;
	flags_player &= ~FLAG_NOTA_TIMEOUT;
	flags_player &= ~FLAG_PLAYER_STOP;
	flags_player &= ~FLAG_PLAYER_END;
	piUnlock(PLAYER_FLAGS_KEY);

	p_player = (TipoPlayer*) (this->user_data);
	p_player->p_efecto = &p_player->efecto_impacto;
	p_player -> posicion_nota_actual = 0;
	p_player -> duracion_nota_actual = p_player->efecto_impacto.duraciones[0];
	p_player -> frecuencia_nota_actual = p_player->efecto_impacto.frecuencias[0];

	tmr_startms(p_player->tmr, p_player->duracion_nota_actual);
	softToneWrite(23, p_player->frecuencia_nota_actual);

	piLock(STD_IO_BUFFER_KEY);
	printf("InicializaPlayImpacto\n");
	piUnlock(STD_IO_BUFFER_KEY);
}

void ComienzaNuevaNota (fsm_t* this) {

	TipoPlayer *p_player;
	p_player = (TipoPlayer*) (this->user_data);
	tmr_startms(p_player->tmr, p_player->duracion_nota_actual);

	piLock(PLAYER_FLAGS_KEY);
	flags_player &= ~FLAG_START_DISPARO;
	flags_player &= ~FLAG_START_IMPACTO;
	flags_player &= ~FLAG_NOTA_TIMEOUT;
	flags_player &= ~FLAG_PLAYER_STOP;
	flags_player &= ~FLAG_PLAYER_END;
	piUnlock(PLAYER_FLAGS_KEY);

	piLock(STD_IO_BUFFER_KEY);
	printf("[PLAYER][ComienzaNuevaNota][NUEVA NOTA ([%i] DE [%i])]\n", p_player->posicion_nota_actual, p_player->p_efecto->num_notas);
	piUnlock(STD_IO_BUFFER_KEY);
}

void ActualizaPlayer (fsm_t* this) {

	TipoPlayer *p_player;
	p_player = (TipoPlayer*) (this->user_data);
	if(p_player->posicion_nota_actual >= p_player->p_efecto->num_notas-1){
		piLock(PLAYER_FLAGS_KEY);
		flags_player |= FLAG_PLAYER_END;
		piUnlock(PLAYER_FLAGS_KEY);
		piLock(STD_IO_BUFFER_KEY);
		printf("[PLAYER][ActualizaPlayer][REPRODUCIDAS TODAS LAS NOTAS]\n");
		piUnlock(STD_IO_BUFFER_KEY);
	} else {
		p_player -> posicion_nota_actual += 1;
		p_player -> duracion_nota_actual = p_player->p_efecto->duraciones[p_player->posicion_nota_actual];
		p_player -> frecuencia_nota_actual = p_player->p_efecto->frecuencias[p_player->posicion_nota_actual];
		piLock(STD_IO_BUFFER_KEY);
		printf("[PLAYER][ActualizaPlayer][NUEVA NOTA ([%i] DE [%i])]\n", p_player->posicion_nota_actual, p_player->p_efecto->num_notas);
		piUnlock(STD_IO_BUFFER_KEY);
		softToneWrite(23, p_player->frecuencia_nota_actual);

	}

	piLock(PLAYER_FLAGS_KEY);
	flags_player &= ~FLAG_NOTA_TIMEOUT;
	piUnlock(PLAYER_FLAGS_KEY);

}

void FinalEfecto (fsm_t* this) {

	piLock(PLAYER_FLAGS_KEY);
	flags_player &= ~FLAG_START_DISPARO;
	flags_player &= ~FLAG_START_IMPACTO;
	flags_player &= ~FLAG_NOTA_TIMEOUT;
	flags_player &= ~FLAG_PLAYER_STOP;
	flags_player &= ~FLAG_PLAYER_END;
	piUnlock(PLAYER_FLAGS_KEY);

	softToneWrite(23,0);

	piLock(STD_IO_BUFFER_KEY);
	printf("[PLAYER][FINAL EFECTO]\n");
	piUnlock(STD_IO_BUFFER_KEY);
}

//------------------------------------------------------
// PROCEDIMIENTOS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------

void timer_player_duracion_nota_actual_isr (union sigval value) {
	piLock(PLAYER_FLAGS_KEY);
	flags_player |= FLAG_NOTA_TIMEOUT;
	piUnlock(PLAYER_FLAGS_KEY);
}
