
#include "torreta.h"

//------------------------------------------------------
// PROCEDIMIENTOS DE INICIALIZACION DE LOS OBJETOS ESPECIFICOS
//------------------------------------------------------

int RX = 0;

void InicializaTorreta (TipoTorreta *p_torreta) {
	//wiringPiSetupGpio();

	p_torreta->intentos = 0;

	p_torreta->servo_x.incremento = SERVO_INCREMENTO;
	p_torreta->servo_x.minimo 	= SERVO_MINIMO;
	p_torreta->servo_x.maximo 	= SERVO_MAXIMO;


	p_torreta->servo_y.incremento = SERVO_INCREMENTO;
	p_torreta->servo_y.minimo 	= SERVO_MINIMO;
	p_torreta->servo_y.maximo 	= SERVO_MAXIMO;


	p_torreta->servo_x.inicio 	= SERVO_MINIMO + (SERVO_MAXIMO - SERVO_MINIMO)/2;
	p_torreta->posicion.x 	= p_torreta->servo_x.inicio;
	p_torreta->servo_y.inicio 	= SERVO_MINIMO + (SERVO_MAXIMO - SERVO_MINIMO)/2;
	p_torreta->posicion.y 	= p_torreta->servo_y.inicio;

	if(p_torreta->posicion.x > p_torreta->servo_x.maximo)
		p_torreta->posicion.x = p_torreta->servo_x.maximo;

	if(p_torreta->posicion.x < p_torreta->servo_x.minimo)
		p_torreta->posicion.x = p_torreta->servo_x.minimo;

	if(p_torreta->posicion.y > p_torreta->servo_y.maximo)
		p_torreta->posicion.y = p_torreta->servo_y.maximo;

	if(p_torreta->posicion.y < p_torreta->servo_y.minimo)
		p_torreta->posicion.y = p_torreta->servo_y.minimo;

	softPwmCreate (SERVO_PIN_2, p_torreta->servo_x.inicio, SERVO_PWM_RANGE); // Internamente ya hace: piHiPri (90) ;
	softPwmCreate (SERVO_PIN_1, p_torreta->servo_y.inicio, SERVO_PWM_RANGE); // Internamente ya hace: piHiPri (90) ;
	softPwmWrite(SERVO_PIN_1, p_torreta->posicion.y);
	softPwmWrite(SERVO_PIN_2, p_torreta->posicion.x);


	wiringPiISR (IR_RX_PIN, INT_EDGE_RISING, impact);

	p_torreta->tmr = tmr_new (timer_duracion_disparo_isr);

}

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

int CompruebaComienzo (fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags_system & FLAG_SYSTEM_START);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaRestart (fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags_system & FLAG_SYSTEM_RESTART);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaJoystickUp (fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags_system & FLAG_JOYSTICK_UP);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaJoystickDown (fsm_t* fsm_player){
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags_system & FLAG_JOYSTICK_DOWN);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaJoystickLeft (fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags_system & FLAG_JOYSTICK_LEFT);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaJoystickRight (fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags_system & FLAG_JOYSTICK_RIGHT);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaTimeoutDisparo (fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags_system & FLAG_SHOOT_TIMEOUT);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaImpacto (fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags_system & FLAG_TARGET_DONE);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaTriggerButton (fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags_system & FLAG_TRIGGER_BUTTON);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaFinalJuego (fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags_system & FLAG_SYSTEM_END);
	piUnlock(SYSTEM_FLAGS_KEY);

	return result;
}

//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------


void impact (void) {
	piLock (SYSTEM_FLAGS_KEY);
	flags_teclado |= FLAG_TARGET_DONE;
	piUnlock (SYSTEM_FLAGS_KEY);
}

void ComienzaSistema (fsm_t* this) {
	//TipoTorreta *p_torreta;
	//p_torreta = (TipoTorreta*)(this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags_system &= ~FLAG_SYSTEM_START;
	piUnlock(SYSTEM_FLAGS_KEY);

	piLock(SYSTEM_FLAGS_KEY);
	flags_system &= ~FLAG_SYSTEM_RESTART;
	piUnlock(SYSTEM_FLAGS_KEY);

	digitalWrite (IR_TX_PIN, LOW);

	printf("\nComienza sistema\n");
	fflush(stdout);
}

void MueveTorretaArriba (fsm_t* this) {
	TipoTorreta *p_torreta;
	p_torreta = (TipoTorreta*)(this->user_data);

	flags_system &= (~FLAG_JOYSTICK_UP);

	if(p_torreta->posicion.y + p_torreta->servo_y.incremento <= p_torreta->servo_y.maximo) {
		p_torreta->posicion.y = p_torreta->posicion.y + p_torreta->servo_y.incremento;

		softPwmWrite(SERVO_PIN_1, p_torreta->posicion.y);

		printf("[SERVO][POSICION]=[%d]\n", p_torreta->posicion.y);
		fflush(stdout);
	}
}

void MueveTorretaAbajo (fsm_t* this) {
	TipoTorreta *p_torreta;
	p_torreta = (TipoTorreta*)(this->user_data);

	flags_system &= (~FLAG_JOYSTICK_DOWN);

	if(p_torreta->posicion.y - p_torreta->servo_y.incremento >= p_torreta->servo_y.minimo) {
		p_torreta->posicion.y = p_torreta->posicion.y - p_torreta->servo_y.incremento;

		softPwmWrite(SERVO_VERTICAL_PIN, p_torreta->posicion.y);

		printf("[SERVO][POSICION]=[%d]\n", p_torreta->posicion.y);
		fflush(stdout);
	}
}

void MueveTorretaIzquierda (fsm_t* this) {
	TipoTorreta *p_torreta;
	p_torreta = (TipoTorreta*)(this->user_data);

	flags_system &= (~FLAG_JOYSTICK_LEFT);

	if(p_torreta->posicion.x + p_torreta->servo_x.incremento <= p_torreta->servo_x.maximo) {
		p_torreta->posicion.x = p_torreta->posicion.x + p_torreta->servo_x.incremento;

		softPwmWrite(SERVO_PIN_2, p_torreta->posicion.x);

		printf("[SERVO][POSICION]=[%d]\n", p_torreta->posicion.x);
		fflush(stdout);
	}
}

void MueveTorretaDerecha (fsm_t* this) {
	TipoTorreta *p_torreta;
	p_torreta = (TipoTorreta*)(this->user_data);

	flags_system &= (~FLAG_JOYSTICK_RIGHT);

	if(p_torreta->posicion.x - p_torreta->servo_x.incremento >= p_torreta->servo_x.minimo) {
		p_torreta->posicion.x = p_torreta->posicion.x - p_torreta->servo_x.incremento;

		softPwmWrite(SERVO_PIN_2, p_torreta->posicion.x);

		printf("[SERVO][POSICION]=[%d]\n", p_torreta->posicion.x);
		fflush(stdout);
	}
}

void DisparoIR (fsm_t* this) {
	TipoTorreta *p_torreta;
	p_torreta = (TipoTorreta*)(this->user_data);


	piLock(PLAYER_FLAGS_KEY);
	flags_player |= FLAG_START_DISPARO;
	piUnlock(PLAYER_FLAGS_KEY);

	piLock(SYSTEM_FLAGS_KEY);
	flags_system &= (~FLAG_TRIGGER_BUTTON);
	piUnlock(SYSTEM_FLAGS_KEY);

	//if(digitalRead(IR_RX_PIN) == LOW){
		//piLock(SYSTEM_FLAGS_KEY);
	//	flags_system |= FLAG_TARGET_DONE;
	//	piUnlock(SYSTEM_FLAGS_KEY);
	//}


	digitalWrite (IR_TX_PIN, HIGH);
	tmr_startms(p_torreta->tmr, SHOOTING_PERIOD);


	printf("\nDisparo Led\n");
	fflush(stdout);


}

void FinalDisparoIR (fsm_t* this) {
	TipoTorreta *p_torreta;
	p_torreta = (TipoTorreta*)(this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags_system &= (~FLAG_SHOOT_TIMEOUT);
	piUnlock(SYSTEM_FLAGS_KEY);


	if(p_torreta->intentos >= 2){
		piLock(SYSTEM_FLAGS_KEY);
		flags_system |= FLAG_SYSTEM_END;
		piUnlock(SYSTEM_FLAGS_KEY);
	} else {
		p_torreta->intentos = p_torreta->intentos + 1;
	}

	digitalWrite (IR_TX_PIN, LOW);

	printf("\nFinal Disparo Led\n");
	fflush(stdout);

}

void ImpactoDetectado (fsm_t* this) {
	TipoTorreta *p_torreta;
	p_torreta = (TipoTorreta*)(this->user_data);


	piLock(PLAYER_FLAGS_KEY);
	flags_player |= FLAG_START_IMPACTO;
	piUnlock(PLAYER_FLAGS_KEY);

	piLock(SYSTEM_FLAGS_KEY);
	flags_system &= (~FLAG_TARGET_DONE);
	piUnlock(SYSTEM_FLAGS_KEY);


	p_torreta->puntuacion ++;
	if(p_torreta->intentos >= 2){
		piLock(SYSTEM_FLAGS_KEY);
		flags_system |= FLAG_SYSTEM_END;
		piUnlock(SYSTEM_FLAGS_KEY);
	} else {
		p_torreta->intentos = p_torreta->intentos + 1;
	}
	digitalWrite (IR_TX_PIN, LOW);

	printf("\nImpacto Detectado\n");
	fflush(stdout);
}

void FinalizaJuego (fsm_t* this) {
	TipoTorreta *p_torreta;
	p_torreta = (TipoTorreta*)(this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags_system &= (~FLAG_SYSTEM_END);
	piUnlock(SYSTEM_FLAGS_KEY);

	printf("\nFinal Juego\n");
	printf("\n Tu puntuacion es: [%d]\n", p_torreta->puntuacion);
	fflush(stdout);
}

void timer_duracion_disparo_isr (union sigval value) {
	piLock(SYSTEM_FLAGS_KEY);
	flags_system |= FLAG_SHOOT_TIMEOUT;
	piUnlock(SYSTEM_FLAGS_KEY);
}
