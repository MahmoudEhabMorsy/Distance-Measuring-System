/*
 * usonic.c
 *
 *  Created on: Oct 12, 2022
 *      Author: Mahmoud Ehab
 */
#include"icu.h"
#include"gpio.h"
#include"usonic.h"
#include<util/delay.h>
/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/

uint8 g_edgeCount = 0;/*global variable to count the edges encountered*/
uint16 g_timePulse=0;/*global variable to store the time captured by icu to use it to calculate the distance*/
/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/
/*Description :
 *Initialize the ICU driver as required.
 *Setup the ICU call back function.
 *Setup the direction for the trigger pin as output pin through the GPIO driver.
 */
void Ultrasonic_init(void){
	/* Create configuration structure for ICU driver */
	Icu_ConfigType Icu_Config = {F_CPU_8,RISING};
	Icu_init(&Icu_Config);
	/* Set the Call back function pointer in the ICU driver */
	Icu_setCallBack(Ultrasonic_edgeProcessing);
	/*Set Trigger Pin as Output*/
	GPIO_setupPinDirection(TRIGGER_PORT_ID,TRIGGER_PIN_ID,PIN_OUTPUT);
}

/*Description:
 *Send the Trigger pulse to the ultrasonic.
 */
void Ultrasonic_Trigger(void){
	/*Set the trigger pin as active*/
	GPIO_writePin(TRIGGER_PORT_ID,TRIGGER_PIN_ID,LOGIC_HIGH);
	_delay_us(10);
	/*clear the trigger pin to create the pulse*/
	GPIO_writePin(TRIGGER_PORT_ID,TRIGGER_PIN_ID,LOGIC_LOW);
}
/* Description:
 *Send the trigger pulse by using Ultrasonic_Trigger function.
 *Start the measurements by the ICU from this moment.
 *Returns The measured distance in Centimeter.
 */
uint16 Ultrasonic_readDistance(void){
	uint16 distance=0;
	/*call the trigger function to start the ultrasonic measurement*/
	Ultrasonic_Trigger();
	/*formula to calculate the distance covered by sound in cm*/
	distance= (((SOUND_SPEED*g_timePulse)/2)*TIME_OF_INSTRUCTION);
	/*effort to correct the error caused by inaccuracy*/
#if(ERROR_CORRECTION_ACTIVATED==1)
	if(distance<58){
		return distance+OFFSET1;
	}
	else if(distance>=58&&distance<129){
		return distance+OFFSET2;
	}
	else if(distance>=129&&distance<200){
		return distance+OFFSET3;
	}
	else if(distance>=200&&distance<272){
		return distance+OFFSET4;
	}
	else if(distance>=272&&distance<343){
		return distance+OFFSET5;
	}
	else if(distance>=343&&distance<415){
		return distance+OFFSET6;
	}
	else if(distance>=415&&distance<431){
		return distance+OFFSET7;
	}
	else {
		return distance;
	}
#elif
	return distance;
	#endif
}

/*Description:
 *This is the call back function called by the ICU driver.
 *This is used to calculate the high time (pulse time) generated by the ultrasonic sensor.
 */
void Ultrasonic_edgeProcessing(void){
	g_edgeCount++;
	if(g_edgeCount == 1)
	{
		/*
		 * Clear the timer counter register to start measurements from the
		 * first detected rising edge
		 */
		Icu_clearTimerValue();
		/* Detect falling edge */
		Icu_setEdgeDetectionType(FALLING);
	}
	else if(g_edgeCount == 2)
	{
		/* Store the Pulse time value */
		g_timePulse = Icu_getInputCaptureValue();
/*Clear the timer counter register to start measurements from the
* first detected rising edge
*/
		Icu_clearTimerValue();
		/*clear the edgecount global variable to continuously measure distance*/
		g_edgeCount=0;
		/* Detect rising edge */
		Icu_setEdgeDetectionType(RISING);
	}
}

