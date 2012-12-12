/**
 *	@file ringBuffer.c
 *  @brief Ring buffer, multithread safe with overwriting possibility.
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 * 			include file :
 *
******************************************/

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <libSAL/print.h>
#include <libSAL/mutex.h>

#include <libNetwork/error.h>

#include "ringBuffer.h"

/*****************************************
 * 
 * 			implementation :
 *
******************************************/


network_ringBuffer_t* NETWORK_NewRingBuffer(	unsigned int buffSize, unsigned int buffCellSize)
{
    /** -- Create a new ring buffer not overwritable -- */
	return NETWORK_NewRingBufferWithOverwriting(	buffSize, buffCellSize, 0 );
}

network_ringBuffer_t* NETWORK_NewRingBufferWithOverwriting(	unsigned int buffSize, 
														unsigned int buffCellSize, 
														int overwriting )
{
    /** -- Create a new ring buffer -- */
    
    /** local declarations */
	network_ringBuffer_t* pRingBuff =  malloc( sizeof(network_ringBuffer_t));
	
	if(pRingBuff)
	{
		pRingBuff->buffSize = buffSize;
		pRingBuff->buffCellSize = buffCellSize;
		pRingBuff->buffIndexInput = 0;
		pRingBuff->buffIndexOutput = 0;
		pRingBuff->overwriting = overwriting;
		sal_mutex_init( &(pRingBuff->mutex) );
		pRingBuff->dataBuffer = malloc( buffCellSize * buffSize );
		
		if( pRingBuff->dataBuffer == NULL)
		{
            NETWORK_DeleteRingBuffer(&pRingBuff);
		}
	}
	
	return pRingBuff;
}

void NETWORK_DeleteRingBuffer(network_ringBuffer_t** ppRingBuff)
{
    /** -- Delete the ring buffer -- */
    
    /** local declarations */
	network_ringBuffer_t* pRingBuff = NULL;
	
	if(ppRingBuff)
	{
		pRingBuff = *ppRingBuff;
		
		if(pRingBuff)
		{
			sal_mutex_destroy(&(pRingBuff->mutex));
			free(pRingBuff->dataBuffer);
            pRingBuff->dataBuffer = NULL;
		
			free(pRingBuff);
            pRingBuff = NULL;
		}
		*ppRingBuff = NULL;
	}
}

int NETWORK_RingBuffPushBack(network_ringBuffer_t* pRingBuff, const void* pNewData)
{
    /** -- Add the new data at the back of the ring buffer -- */
    
    /** local declarations */
	int error = NETWORK_OK;
	void* buffPointor = NULL;
	
	sal_mutex_lock(&(pRingBuff->mutex));
	
	if( NETWORK_RingBuffGetFreeCellNb(pRingBuff) || pRingBuff->overwriting)
	{	
		if( !NETWORK_RingBuffGetFreeCellNb(pRingBuff) )
		{
			(pRingBuff->buffIndexOutput) += pRingBuff->buffCellSize;
		}
		
		buffPointor = pRingBuff->dataBuffer + 
					( pRingBuff->buffIndexInput % (pRingBuff->buffSize * pRingBuff->buffCellSize) );
		
		memcpy(buffPointor, pNewData, pRingBuff->buffCellSize);
		
		pRingBuff->buffIndexInput += pRingBuff->buffCellSize;
	}
    else
    {
        error = NETWORK_ERROR_BUFFER_SIZE;
    }
	
	sal_mutex_unlock(&(pRingBuff->mutex));

	return error;
}

int NETWORK_RingBuffPopFront(network_ringBuffer_t* pRingBuff, void* pPopData)
{
    /** -- Pop the oldest data -- */
    
    /** local declarations */
	void* buffPointor = NULL;
	int error = NETWORK_OK;
	
	sal_mutex_lock(&(pRingBuff->mutex));
	
	if( !NETWORK_RingBuffIsEmpty(pRingBuff) )
	{
		if(pPopData != NULL)
		{
			buffPointor = 	pRingBuff->dataBuffer + 
					(pRingBuff->buffIndexOutput % (pRingBuff->buffSize * pRingBuff->buffCellSize) );
					
			memcpy(pPopData, buffPointor, pRingBuff->buffCellSize);
		}
		
		(pRingBuff->buffIndexOutput) += pRingBuff->buffCellSize;
	}
	else
	{
		error = NETWORK_ERROR_BUFFER_EMPTY;
	}
	
	sal_mutex_unlock(&(pRingBuff->mutex));
	
	return error;
}

int NETWORK_RingBuffFront(network_ringBuffer_t* pRingBuff, void* pFrontData)
{
    /** -- Return a pointer on the front data -- */
    
    /** local declarations */
	int error = NETWORK_OK;
	void* buffPointor = NULL;
					
	sal_mutex_lock(&(pRingBuff->mutex));
	
	buffPointor = pRingBuff->dataBuffer + 
					(pRingBuff->buffIndexOutput % (pRingBuff->buffSize * pRingBuff->buffCellSize) );
	
	if( !NETWORK_RingBuffIsEmpty(pRingBuff) )
	{
		memcpy(pFrontData, buffPointor, pRingBuff->buffCellSize);
	}
    else
    {
        error = NETWORK_ERROR_BUFFER_SIZE;
    }
	
	sal_mutex_unlock(&(pRingBuff->mutex));
	
	return error;
}

void NETWORK_RingBuffPrint(network_ringBuffer_t* pRingBuff)
{
    /** -- Print the state of the ring buffer -- */
    
	sal_mutex_lock(&(pRingBuff->mutex));
	
	SAL_PRINT(PRINT_WARNING," pointer dataBuffer :%d \n",pRingBuff->dataBuffer);
	SAL_PRINT(PRINT_WARNING," buffSize :%d \n",pRingBuff->buffSize);
	SAL_PRINT(PRINT_WARNING," buffCellSize :%d \n",pRingBuff->buffCellSize);
	SAL_PRINT(PRINT_WARNING," buffIndexOutput :%d \n",pRingBuff->buffIndexOutput);
	SAL_PRINT(PRINT_WARNING," buffIndexInput :%d \n",pRingBuff->buffIndexInput);
	SAL_PRINT(PRINT_WARNING," overwriting :%d \n",pRingBuff->overwriting);
	SAL_PRINT(PRINT_WARNING," data : \n",pRingBuff->overwriting);
	
	sal_mutex_unlock(&(pRingBuff->mutex));
	
	NETWORK_RingBuffDataPrint(pRingBuff);
}

void NETWORK_RingBuffDataPrint(network_ringBuffer_t* pRingBuff)
{
    /** -- Print the contents of the ring buffer -- */
    
    /** local declarations */
	void* it = NULL;
	int  iindex = 0;
	int  ii = 0;
    
    sal_mutex_lock(&(pRingBuff->mutex));
	
	for( 	iindex = pRingBuff->buffIndexOutput ; 
			iindex < pRingBuff->buffIndexInput ;
			iindex += pRingBuff->buffCellSize )
	{
		it = pRingBuff->dataBuffer + (iindex % (pRingBuff->buffSize * pRingBuff->buffCellSize) );
		
		SAL_PRINT(PRINT_WARNING,"	- 0x: ");
		for(ii = 0 ; ii < pRingBuff->buffCellSize ; ++ii)
		{
			SAL_PRINT(PRINT_WARNING,"%2x | ",*((uint8_t*)it));
			++it;
		}
		SAL_PRINT(PRINT_WARNING,"\n");
	}
	
	sal_mutex_unlock(&(pRingBuff->mutex));
}
