/**
 *	@file sender.c
 *  @brief manage the data sending
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 * 			include file :
 *
******************************************/

#include <stdlib.h>

#include <stddef.h>

#include <unistd.h>
#include <string.h>

#include <libSAL/print.h>
#include <libSAL/socket.h>
#include <libSAL/endianness.h>

#include <libNetwork/error.h>
#include <libNetwork/frame.h>
#include "buffer.h"
#include <libNetwork/deportedData.h>
#include "ioBuffer.h"

#include "sender.h"

/*****************************************
 * 
 * 			define :
 *
******************************************/
					
#define MILLISECOND 1000

/*****************************************
 * 
 * 			private header:
 *
******************************************/
/**
 *  @brief send the data
 * 	@param pSender the pointer on the Sender
 *	@note only call by NETWORK_RunSendingThread()
 * 	@see NETWORK_RunSendingThread()
**/
void NETWORK_SenderSend(network_sender_t* pSender);

/**
 *  @brief add data to the sender buffer
 * 	@param pSender the pointer on the Sender
 *	@param pinputBuff
 * 	@param seqNum
 * 	@note only call by NETWORK_RunSendingThread()
 * 	@see NETWORK_RunSendingThread()
**/
int NETWORK_SenderAddToBuffer(	network_sender_t* pSender,const network_ioBuffer_t* pinputBuff,
                                int seqNum);

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


network_sender_t* NETWORK_NewSender(	unsigned int sendingBufferSize, unsigned int numOfInputBuff,
								network_ioBuffer_t** ppTab_input)
{	
	/** -- Create a new sender -- */
	
	/** local declarations */
	network_sender_t* pSender =  NULL;
	int error = NETWORK_OK;
	
	/** Create the sender */
	pSender =  malloc( sizeof(network_sender_t));
	
	if(pSender)
	{
		pSender->isAlive = 1;
		pSender->numOfInputBuff = numOfInputBuff;
		pSender->pptab_inputBuffer = ppTab_input;
		
		/** Create the Sending buffer */
		pSender->pSendingBuffer = NETWORK_NewBuffer(sendingBufferSize, 1);
			
		if(pSender->pSendingBuffer == NULL)
		{
			error = NETWORK_ERROR_NEW_BUFFER;
		}
		
		/** delete the sender if an error occurred */
		if(error != NETWORK_OK)
		{
            SAL_PRINT(PRINT_ERROR,"error: %d occurred \n", error );
			NETWORK_DeleteSender(&pSender);
		}
	}
	
	return pSender;
}

void NETWORK_DeleteSender(network_sender_t** ppSender)
{
	/** -- Delete the sender -- */
	
	/** local declarations */
	network_sender_t* pSender = NULL;
	
	if(ppSender)
	{
		pSender = *ppSender;
		
		if(pSender)
		{
			NETWORK_DeleteBuffer( &(pSender->pSendingBuffer) );
		
			free(pSender);
            pSender = NULL;
		}
		*ppSender = NULL;
	}
}

void* NETWORK_RunSendingThread(void* data)
{
	/** -- Manage the sending of the data on the sender' socket -- */
	
	/** local declarations */
	network_sender_t* pSender = data;
	int seq = 1;
	int indexInput = 0;
	int callbackReturn = 0;
	network_ioBuffer_t* pInputTemp = NULL;
	
	
	while( pSender->isAlive )
	{		
		usleep(MILLISECOND);
		
		/** for each input buffer try to send the data if necessary */
		for(indexInput = 0 ; indexInput < pSender->numOfInputBuff ; ++indexInput  )
		{
			pInputTemp = pSender->pptab_inputBuffer[indexInput];
			
			/** decrement the time to wait */
			if(pInputTemp->waitTimeCount > 0)
			{
				--(pInputTemp->waitTimeCount);
			}

			if( NETWORK_IoBufferIsWaitAck(pInputTemp)  ) 
			{
				if(pInputTemp->ackWaitTimeCount == 0)
				{
					if(pInputTemp->retryCount == 0)
					{
						/** if there are timeout and too sending retry ... */
						
						//callbackReturn = pInputTemp->timeoutcallback();
						SAL_PRINT(PRINT_DEBUG," !!! too retry !!! \n");
						
						if(callbackReturn)
						{
							pInputTemp->retryCount = pInputTemp->nbOfRetry;
						}
					}
					else
					{
						/**
						 *  if there is a timeout, retry to send the data 
						 *  and decrement the number of retry still possible
						**/
						NETWORK_SenderAddToBuffer(pSender, pInputTemp, pInputTemp->seqWaitAck);
						pInputTemp->ackWaitTimeCount = pInputTemp->ackTimeoutMs;
						
						if(pInputTemp->retryCount > 0 )
						{
							--(pInputTemp->retryCount);
						}
					}
				}
				
				/** decrement the time to wait before considering as a timeout */
				if(pInputTemp->ackWaitTimeCount > 0 )
				{
					--(pInputTemp->ackWaitTimeCount);
				}
			}
			else if( !NETWORK_RingBuffIsEmpty(pInputTemp->pBuffer) && !pInputTemp->waitTimeCount)
			{
				/** try to add the latest data of the input buffer in the sending buffer*/
				if( !NETWORK_SenderAddToBuffer(pSender, pInputTemp, seq) )
				{				
					pInputTemp->waitTimeCount = pInputTemp->sendingWaitTime;
					
					switch(pInputTemp->dataType)
					{
						case network_frame_t_TYPE_DATA_WITH_ACK:
							/** 
							 * reinitialize the input buffer parameters,
							 * save the sequence wait for the acknowledgement,
							 * and pass on waiting acknowledgement.
							**/
							pInputTemp->isWaitAck = 1;
							pInputTemp->seqWaitAck = seq;
							pInputTemp->ackWaitTimeCount = pInputTemp->ackTimeoutMs;
							pInputTemp->retryCount = pInputTemp->nbOfRetry;	
						break;
						
						case network_frame_t_TYPE_DATA:
							/** pop the data sent*/
							NETWORK_RingBuffPopFront(pInputTemp->pBuffer, NULL);
						break;
						
						case network_frame_t_TYPE_ACK:
							/** pop the acknowledgement sent*/
							NETWORK_RingBuffPopFront(pInputTemp->pBuffer, NULL);
						break;
						
						case network_frame_t_TYPE_KEEP_ALIVE:
						
						break;
						
						default:

						break;
					}
					
					/** increment the sequence number */
					++seq;
				}
			}
		}
		NETWORK_SenderSend(pSender);
	}

	sal_close(pSender->socket);
        
    return NULL;
}

void NETWORK_StopSender(network_sender_t* pSender)
{
	/** -- Stop the sending -- */
	pSender->isAlive = 0;
}

int NETWORK_SenderAckReceived(network_sender_t* pSender, int id, int seqNum)
{
	/** -- Receive an acknowledgment fo a data -- */
	
	/** local declarations */
	network_ioBuffer_t* pInputBuff = NULL;
	int error = NETWORK_OK;
    
	pInputBuff = NETWORK_IoBufferFromId( pSender->pptab_inputBuffer, pSender->numOfInputBuff, id );
	
	if(pInputBuff != NULL)
	{
		/**
		 *  transmit the acknowledgment to the input buffer. 
		 * 	if the acknowledgment is suitable the waiting data is popped
		**/
		error = NETWORK_IoBufferAckReceived( pInputBuff, seqNum );
	}
    else
    {
        error = NETWORK_ERROR_ID_UNKNOWN;
    }
	
	return error;
}

int NETWORK_SenderConnection(network_sender_t* pSender, const char* addr, int port)
{
	/** -- Connect the socket in UDP to a port of an address -- */
	
	/** local declarations */
	struct sockaddr_in sendSin;
	
	sendSin.sin_addr.s_addr = inet_addr(addr);
	sendSin.sin_family = AF_INET;
	sendSin.sin_port = htons(port);
	
	pSender->socket = sal_socket(  AF_INET, SOCK_DGRAM, 0);

	return sal_connect( pSender->socket, (struct sockaddr*)&sendSin, sizeof(sendSin) );
}

/*****************************************
 * 
 * 			private implementation:
 *
******************************************/

void NETWORK_SenderSend(network_sender_t* pSender)
{	
	/**  -- send the data -- */
	
	/** local declarations */
	int nbCharCopy = 0;
	
	if( !NETWORK_BufferIsEmpty(pSender->pSendingBuffer) )
	{	
		nbCharCopy = pSender->pSendingBuffer->pFront - pSender->pSendingBuffer->pStart;
			
		sal_send(pSender->socket, pSender->pSendingBuffer->pStart, nbCharCopy, 0);
			
		pSender->pSendingBuffer->pFront = pSender->pSendingBuffer->pStart;
	}
}

int NETWORK_SenderAddToBuffer( network_sender_t* pSender,const network_ioBuffer_t* pinputBuff,
						        int seqNum)
{
	/** -- add data to the sender buffer -- */
	
	/** local declarations */
	int error = NETWORK_OK;
    uint32_t droneEndianInt32 = 0;
	int sizeNeed = 0;
    int dataSize = 0;
    network_DeportedData_t deportedData;
	
    /** get the data size */
    if( pinputBuff->deportedData )
    {
        /** if the data is deported get the size of the date pointed*/
        NETWORK_RingBuffFront( pinputBuff->pBuffer, &deportedData);
        dataSize = deportedData.dataSize;
    }
    else
    {
        dataSize = pinputBuff->pBuffer->buffCellSize;
    }
    
    /** calculate the size needed */
    sizeNeed = offsetof(network_frame_t, data) + dataSize;
    
	if( NETWORK_BufferGetFreeCellNb(pSender->pSendingBuffer) >= sizeNeed )
	{	
		/** add type */
		droneEndianInt32 =  htodl( (uint32_t) pinputBuff->dataType );
		memcpy( pSender->pSendingBuffer->pFront, &(droneEndianInt32), sizeof(uint32_t));
		pSender->pSendingBuffer->pFront +=  sizeof(uint32_t) ;
		
		/** add id */
		droneEndianInt32 =  htodl(pinputBuff->id);
		memcpy( pSender->pSendingBuffer->pFront, &(droneEndianInt32), sizeof(uint32_t));
		pSender->pSendingBuffer->pFront +=  sizeof(uint32_t) ;
		
		/** add seq */
		droneEndianInt32 =  htodl(seqNum);
		memcpy( pSender->pSendingBuffer->pFront, &(droneEndianInt32), sizeof(uint32_t));
		pSender->pSendingBuffer->pFront +=  sizeof(uint32_t) ;
		
		/** add size */
		droneEndianInt32 =  htodl(sizeNeed); 
		memcpy( pSender->pSendingBuffer->pFront, &(droneEndianInt32), sizeof(uint32_t));
		pSender->pSendingBuffer->pFront +=  sizeof(uint32_t) ;
		
		/** add data */
        if( pinputBuff->deportedData )
        {
            /** copy the data pointed by the deportedData*/	
            memcpy(pSender->pSendingBuffer->pFront, deportedData.pData, dataSize);
        }
        else
        {		
            /** copy the data on the ring buffer */			
		    error = NETWORK_RingBuffFront(pinputBuff->pBuffer, pSender->pSendingBuffer->pFront);
		}
        
		if( error == NETWORK_OK )
		{
            /** if the adding of the data is successful, increment the front of the Sending Buffer */
			pSender->pSendingBuffer->pFront += dataSize;
		}
		else
		{
            /** if the adding of the data is failed, decrement the front of the SendingBuffer */
			pSender->pSendingBuffer->pFront -= offsetof(network_frame_t, data);
		}
        
	}
    else
    {
        error = NETWORK_ERROR_BUFFER_SIZE;
    }
	
	return error;
}

