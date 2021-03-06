/***********************************************************************************
@file   app.c
@brief  functions to send and receive data in/from the network
@authors: Gustavo Weber Denardin
          Carlos Henrique Barriquello

Copyright (c) <2009-2013> <Universidade Federal de Santa Maria>

  * Software License Agreement
  *
  * The Software is owned by the authors, and is protected under 
  * applicable copyright laws. All rights are reserved.
  *
  * The above copyright notice shall be included in
  * all copies or substantial portions of the Software.
  *  
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  * THE SOFTWARE.
**********************************************************************************/

#include "BRTOS.h"

/* Config. files */
#include "BRTOSConfig.h"
#include "BoardConfig.h"
#include "AppConfig.h"

#include "unet_api.h"
#include "unet_router.h"
#include "packet.h"
#include "app.h" 

#if WINNT
#include "assert.h"
#else
#define assert(x)   if(!(x)) while(1){};
#endif


#if (BOOTLOADER_ENABLE == 1)
  #include "MCF51_Bootloader.h"    
#endif

#if PROCESSOR == COLDFIRE_V1 && !__GNUC__
#pragma warn_implicitconv off
#pragma warn_unusedarg off
#endif

/********/

INT8U    Config_LUX_THRESHOLD_MIN = 0;
INT8U    Config_LUX_THRESHOLD_MAX = 0;
INT8U    Config_PARENT_THRESHOLD = 0;
INT8U    Config_PARENT_THRESHOLD_MIN = 0;
INT8U    Config_REPORT_PERIOD_1000MS = 30;
INT8U    Config_REPORT_JITTER_100MS = 1;

#define START_ROUTE 0

extern packet_t packet_up;
uint8_t NetGeneralONOFF(uint8_t state, addr64_t *destiny)
{
	uint8_t res = 0;
	packet_t  *p = &packet_up;
	uint8_t payload_len = 4;

	/* create a app packet for testing router up */
	REQUIRE_OR_EXIT(packet_acquire_up() == PACKET_ACCESS_ALLOWED, exit_on_require_error);

	/* add fake payload */
	REQUIRE_OR_EXIT(((UNET_APP_HEADER_START+payload_len) < PACKET_END), exit_on_require_error);

	p->packet[UNET_APP_HEADER_START] = APP_01;
	p->packet[UNET_APP_HEADER_START+1] = GENERAL_PROFILE;
	p->packet[UNET_APP_HEADER_START+2] = GENERAL_ONOFF;
	p->packet[UNET_APP_HEADER_START+3] = state;

	res = unet_packet_up_sendto(destiny, payload_len);

	PRINTF_APP(1,"Sending packet ");
	if(res == RESULT_PACKET_SEND_OK)
	{
		PRINTF_APP(1,"OK\r\n");
		extern BRTOS_Sem * Router_Up_Route_Request;
		OSSemPost(Router_Up_Route_Request);
	}else
	{
		extern const char* route_error_tostring[];
		PRINTF_APP(1,"failed due to reason %s \r\n", route_error_tostring[res]);
	}

	//assert(res == RESULT_DESTINATION_UNREACHABLE);

	packet_release_up();

	return res;

	exit_on_require_error:
	return RESULT_PACKET_SEND_ERROR;
}


void Decode_General_Profile(APP_PACKET *app)
{
	switch(app->APP_Command)
    {
      /* message to turn on/off */
      case GENERAL_ONOFF:

        if (app->APP_Command_Attribute == ON)
        {
#if defined BRTOS_PLATFORM
#if BRTOS_PLATFORM == FRDM_KL25Z
        	GPIOPinReset(GPIOB_BASE, GPIO_PIN_18);
#endif
#endif
        }
        if(app->APP_Command_Attribute == OFF)
        {
#if defined BRTOS_PLATFORM
#if BRTOS_PLATFORM == FRDM_KL25Z
        	GPIOPinSet(GPIOB_BASE, GPIO_PIN_18);
#endif
#endif

        }
        break;
    }
}

/**
* @fn     Decode_General_Profile
* @brief  function to decode messages for General profile
**/
#if 0
void Decode_General_Profile(void)
{
    switch(app_packet.APP_Command)
    {
      /* message to turn on/off */
      case GENERAL_ONOFF:

        if (app_packet.APP_Command_Attribute == ON)
        {              
           //SensorLight_Disable();

        }
        if(app_packet.APP_Command_Attribute == OFF)
        {          
            //SensorLight_Enable();

        }  
        break;       
      
      case DEBUG_PKT:
        //Process_Debug_Packet();
        break; 
      
      case RADIO_TXPL:
        /* set TX output power */           
        UNET_RADIO.set(TXPOWER, app_packet.APP_Command_Attribute & 0xF8);

        
#if (defined TEST_PIN && TEST_PIN == 1)        
        PIN_RELAY = ON;
        DelayTask(500);
        PIN_RELAY = OFF;
#endif        
        
      break;
      
      case APP_CONFIG_PARAM:
        switch (app_packet.APP_Command_Attribute){
          case LUX_THRESHOLD_MIN:
          Config_LUX_THRESHOLD_MIN = app_packet.APP_Payload[0];
          break;
          case LUX_THRESHOLD_MAX:
          Config_LUX_THRESHOLD_MAX = app_packet.APP_Payload[0];
          break;
          case PARENT_THRESHOLD:
          Config_PARENT_THRESHOLD = app_packet.APP_Payload[0];
          break;
          case PARENT_THRESHOLD_MIN:
          Config_PARENT_THRESHOLD_MIN = app_packet.APP_Payload[0];
          break;
          case REPORT_PERIOD_1000MS:
          Config_REPORT_PERIOD_1000MS = app_packet.APP_Payload[0];
          break;
          case REPORT_JITTER_100MS:
          Config_REPORT_JITTER_100MS = app_packet.APP_Payload[0];
          break;          
          default:
          break;
        }
      break;
      
      case CREATE_UP_PATH:
      break;      
                    
      default:
#if (UNET_DEVICE_TYPE == PAN_COORDINATOR)
    	  //(void)UARTPutString(0x4006A000, "Comando gen�rico! \n\r");
#endif
        break;
    }
}

/**
* @fn     Decode_Lighting_Profile
* @brief  function to decode messages for Lighting profile
**/
void Decode_Lighting_Profile(void)
{
#ifdef SET_LIGHT_LEVEL
    INT8U level = 0;
#endif
       
    switch(app_packet.APP_Command)
    {
      case LIGHTING_DIMMING:
		#ifdef SET_LIGHT_LEVEL
        level = app_packet.APP_Command_Attribute;
        SET_LIGHT_LEVEL();
        #endif        
                                
        break;
      
      default:
        break;
    }
}


/**
* @fn     Decode_SmartEnergy_Profile
* @brief  function to decode messages for Smart Energy profile
**/

void Decode_SmartEnergy_Profile(void)
{   
    INT8U Attribute = 0;
    switch(app_packet.APP_Command)
    {
      case SIMPLE_METERING:
        Attribute = app_packet.APP_Command_Attribute;
        
        switch(Attribute)
        {
          case CURRENT:
            break;
            
          case VOLTAGE:
            break;
            
          case ENERGY:
            break;
            
          default:
            break;
        }
        
        break;
      
      default:
        break;
    }
}




/**
* @fn     NetSimpleMeasureSE
* @brief  function to send a message for Smart Energy profile
**/

INT8U NetSimpleMeasureSE(INT8U MeasureType, INT16U Value16, INT32U Value32)
{
  INT8U j = 0;
  INT8U status = 0;  
  
  acquireRadio();
  
#if 0
  NWKPayload[0] = APP_01;
  NWKPayload[1] = SMART_ENERGY_PROFILE;
  NWKPayload[2] = MULTIPLE_METERING;
  NWKPayload[3] = MeasureType;
  
  
  switch(MeasureType)
  {
    case CURRENT:
      NWKPayload[4] = (INT8U)(Value16 >> 8);
      NWKPayload[5] = (INT8U)(Value16 & 0xFF);
      j = 2;
      break;
    case VOLTAGE:
      NWKPayload[4] = (INT8U)(Value16 >> 8);
      NWKPayload[5] = (INT8U)(Value16 & 0xFF);            
      j = 2;
      break;
    case ENERGY:
      NWKPayload[4] = (INT8U)(Value32 >> 24);
      NWKPayload[5] = (INT8U)((Value32 & 0xFFFFFF) >> 16);
      NWKPayload[6] = (INT8U)((Value32 & 0xFFFF) >> 8);
      NWKPayload[7] = (INT8U)(Value32 & 0xFF);
      j = 4;
      break;
    case ACTIVE_POWER:
      NWKPayload[4] = (INT8U)(Value32 >> 24);
      NWKPayload[5] = (INT8U)((Value32 & 0xFFFFFF) >> 16);
      NWKPayload[6] = (INT8U)((Value32 & 0xFFFF) >> 8);
      NWKPayload[7] = (INT8U)(Value32 & 0xFF);
      j = 4;
      break;
    case REACTIVE_POWER:
      NWKPayload[4] = (INT8U)(Value32 >> 24);
      NWKPayload[5] = (INT8U)((Value32 & 0xFFFFFF) >> 16);
      NWKPayload[6] = (INT8U)((Value32 & 0xFFFF) >> 8);
      NWKPayload[7] = (INT8U)(Value32 & 0xFF);
      j = 4;
      break;
    case APPARENT_POWER:
      NWKPayload[4] = (INT8U)(Value32 >> 24);
      NWKPayload[5] = (INT8U)((Value32 & 0xFFFFFF) >> 16);
      NWKPayload[6] = (INT8U)((Value32 & 0xFFFF) >> 8);
      NWKPayload[7] = (INT8U)(Value32 & 0xFF);
      j = 4;
      break;
    case POWER_FACTOR:
      NWKPayload[4] = (INT8U)(Value16 >> 8);
      NWKPayload[5] = (INT8U)(Value16 & 0xFF);            
      j = 2;
      break;
    
  }
  
  
  status = DownRoute(START_ROUTE,(INT8U)(4+j));   
#endif

  releaseRadio();
  
  return status;
}




/**
* @fn     NetGeneralInfo
* @brief  function to send message for General profile
**/

INT8U NetGeneralInfo(INT8U MeasureType, INT8U Value8,INT16U Value16, INT16U destiny)
{
  INT8U j = 0;
  INT8U status = 0;
  
  acquireRadio();

#if 0
  NWKPayload[0] = APP_01;
  NWKPayload[1] = GENERAL_PROFILE;
  NWKPayload[2] = SIMPLE_METERING;
  NWKPayload[3] = MeasureType;
  
  
  switch(MeasureType)
  {
    case TEMPERATURE:
    case FAILURE_REPORT:
      NWKPayload[4] = (INT8U)Value8;
      j = 1;
      break;
    default:
      NWKPayload[4] = (INT8U)((Value16 & 0xFFFF) >> 8);
      NWKPayload[5] = (INT8U)(Value16 & 0xFF);
      j = 2;
      break;      
  }
  

  if (destiny == 0){
	  status = DownRoute(START_ROUTE,(INT8U)(j));
  }else{
	  #if (USE_REACTIVE_UP_ROUTE == 1)
	  status = ReactiveUpRoute(START_ROUTE,(INT8U)(j),destiny);
	  #else
	  status = NO_ROUTE_AVAILABLE;
	  #endif
  }
#endif
  
  releaseRadio();
  
  return status;
}


/**
* @fn     NetLightingProfile
* @brief  function to send message for Lighting profile
**/

INT8U NetLightingProfile(INT8U Command, INT8U Parameter, INT8U Value8, INT16U Value16)
{
  INT8U j = 0;
  INT8U status = 0;
  
  acquireRadio();
  
#if  0
  NWKPayload[0] = APP_01;
  NWKPayload[1] = LIGHTING_PROFILE;
  NWKPayload[2] = Command;
  NWKPayload[3] = Parameter;
  
  
  switch(Parameter)
  {
    case LIGHT_LEVEL:
      NWKPayload[4] = (INT8U)(Value16 >> 8);
      NWKPayload[5] = (INT8U)(Value16 & 0xFF);
      j = 2;
      break;  
    case LAMP_STATE:
      NWKPayload[4] = (INT8U)Value8;
      j = 1;
      break;    
    case SENSOR_STATE:
      NWKPayload[4] = (INT8U)Value8;
      j = 1;
      break;                
  }
  

  status = DownRoute(START_ROUTE,(INT8U)(4+j));  
  
#endif

  releaseRadio();
  
  return status;
}


/**
* @fn     NetDebugPacket
* @brief  function to send message for Debug profile
**/
INT8U NetDebugPacket(INT8U Command, INT8U Parameter, INT8U Value8, INT16U Value16, INT16U destiny)
{
  INT8U j = 0;
  INT8U status = 0;
  
  acquireRadio();
  
#if 0
  NWKPayload[0] = APP_01;
  NWKPayload[1] = GENERAL_PROFILE;
  NWKPayload[2] = Command;
  NWKPayload[3] = Parameter;
 

  NWKPayload[4] = Value16 >> 8;
  NWKPayload[5] = Value16 & 0xFF; 
  j=2;
  
  status = OneHopRoute((INT8U)(4+j),destiny);
  
#endif

  releaseRadio();
  
  return status;
}

/**
* @fn     UNET_TX_ToBaseSation
* @brief  function to send data to nearest base station (coordinator)
**/

INT8U UNET_TX_ToBaseSation(INT8U Command, INT8U Attribute, _uint32_t * ptr_data, INT8U nbr_bytes){
    
    INT8U j = 0; 

#if 0
    if(nbr_bytes > MAX_APP_PAYLOAD_SIZE-2 || ptr_data == NULL) {
        return SEND_ERROR;
    }

    if(thisNodeDepth >= ROUTE_TO_BASESTATION_LOST){
        return SEND_ERROR;
    }
    
    acquireRadio();
    
      /* set app header */
      NWKPayload[0] = APP_01;
      NWKPayload[1] = GENERAL_PROFILE;
      NWKPayload[2] = Command;
      NWKPayload[3] = Attribute;
      
      for(j=0; j<nbr_bytes;j++){        
        NWKPayload[j+4] = ptr_data->u8[j];
      }
      
      NWKPayload[j+4+0] = (INT8U)(macAddr >> 8);
      NWKPayload[j+4+1] = (INT8U)(macAddr & 0xFF);
      
      j = DownRoute(START_ROUTE,(INT8U)j+4+2);
    
      releaseRadio();
#endif

      return j;
}

#endif



static FAIL_T fail = NO_FAIL;

void ReportFailure_Get(FAIL_T *failure_kind){
  *failure_kind = fail;
}

void ReportFailure_Set(FAIL_T failure_kind){
  fail = failure_kind;
}



