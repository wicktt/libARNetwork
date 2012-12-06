/**
 *	@file paramNewIoBuffer.h
 *  @brief prameters used to set the parameters of a new inOutBuffer
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_PARAMNEWIOBUFFER_H_
#define _NETWORK_PARAMNEWIOBUFFER_H_

#include <libNetwork/frame.h>

/*****************************************
 * 
 * 			ParamNewIoBuffer header:
 *
******************************************/

/**
 *  @brief used to set the parameters of a new inOutBuffer
**/
typedef struct network_paramNewIoBuffer_t  
{
    int id;						/**< Identifier used to find the InOutBuffer in a list*/
    eNETWORK_Frame_Type dataType;		/**< Type of the data stored in the buffer*/
    int sendingWaitTime;		/**< Time in millisecond between 2 send when the InOutBuffer is used with a libNetwork/sender*/
    int ackTimeoutMs;			/**< Timeout in millisecond before retry to send the data waiting an acknowledgement when the InOutBuffer is used with a libNetwork/sender*/
    int nbOfRetry;				/**< Maximum number of retry of sending before to consider a failure when the InOutBuffer is used with a libNetwork/sender*/
    
    unsigned int buffSize;		/**< Maximum number of data stored*/
    unsigned int buffCellSize;	/**< Size of one data in byte*/
    int overwriting;			/**< Indicator of overwriting possibility (1 = true | 0 = false)*/
    int deportedData;			/**< Indicator of using data deported */

}network_paramNewIoBuffer_t;

/**
 *  @brief initialization of the paramNewIoBuffer with default parameters
 * 	@pre before to use the paramNewIoBuffer the paramaters useful must be set.
 * 	@param[in,out] pParam Pointer on the parameters for the new input or output buffer
**/
void NETWORK_ParamNewIoBufferDefaultInit(network_paramNewIoBuffer_t* pParam); 

/**
 *  @brief check the values of the paramNewIoBuffer
 * 	@param[in] pParam Pointer on the parameters for the new input or output buffer
 * 	@return 1 if the paramNewIoBuffer is usable for create a new ioBuffer else 0
**/
int NETWORK_ParamNewIoBufferCheck( const network_paramNewIoBuffer_t* pParam );

#endif // _NETWORK_PARAMNEWIOBUFFER_H_
