/**
 *	@file inOutBuffer.h
 *  @brief input or output buffer used by libNetwork::receiver or libNetwork::sender
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _IN_OUT_BUFFER_H_
#define _IN_OUT_BUFFER_H_

#include <libNetwork/ringBuffer.h>
#include <libNetwork/common.h>

/**
 *  @brief used to set the parameters of a new inOutBuffer
**/
typedef struct network_paramNewInOutBuffer_t  
{
    int id;						/**< Identifier used to find the InOutBuffer in a list*/
    eAR_CMD_TYPE dataType;		/**< Type of the data stored in the buffer*/
    int sendingWaitTime;		/**< Time in millisecond between 2 send when the InOutBuffer is used with a libNetwork::sender*/
    int ackTimeoutMs;			/**< Timeout in millisecond before retry to send the data waiting an acknowledgement when the InOutBuffer is used with a libNetwork::sender*/
    int nbOfRetry;				/**< Maximum number of retry of sending before to consider a failure when the InOutBuffer is used with a libNetwork::sender*/
    
    unsigned int buffSize;		/**< Maximum number of data stored*/
    unsigned int buffCellSize;	/**< Size of one data in byte*/
    int overwriting;			/**< Indicator of overwriting possibility (1 = true | 0 = false)*/

}network_paramNewInOutBuffer_t;

/**
 *  @brief Input buffer used by libNetwork::sender or output buffer used by libNetwork::receiver
**/
typedef struct network_inOutBuffer_t  
{
    int id;		/**< Identifier used to find the InOutBuffer in a table*/
    network_ringBuffer_t* pBuffer;	/**< Pointer on the ringBuffer used to store the data*/
    eAR_CMD_TYPE dataType;		/**< Type of the data stored in the buffer*/
    int sendingWaitTime;	/**< Time in millisecond between 2 send when the InOutBuffer if used with a libNetwork::sender*/
    int ackTimeoutMs;	/**< Timeout in millisecond after retry to send the data when the InOutBuffer is used with a libNetwork::sender*/
    int nbOfRetry;	/**< Maximum number of retry of sending before to consider a failure when the InOutBuffer is used with a libNetwork::sender*/
    //	timeoutCallback(network_inOutBuffer_t* this)
    
    int isWaitAck;	/**< Indicator of waiting an acknowledgement  (1 = true | 0 = false). Must be accessed through inOutBuffeIsWaitAck()*/
    int seqWaitAck; /**< Sequence number of the acknowledge waiting if used with a libNetwork::sender or of the last command stored if used with a libNetwork::reveiver*/
    int waitTimeCount;	 /**< Counter of time to wait before the next sending*/
    int ackWaitTimeCount;	/**< Counter of time to wait before to consider a timeout without receiving an acknowledgement*/
    int retryCount;		/**< Counter of sending retry remaining before to consider a failure*/
    
    sal_mutex_t mutex;	/**< Mutex to take before to use the ringBuffer*/

}network_inOutBuffer_t;

/**
 *  @brief Create a new input or output buffer
 * 	@warning This function allocate memory
 *	@post deleteInOutBuffer() must be called to delete the input or output buffer and free the memory allocated
 * 	@param pParam Pointer on the parameters for the new input or output buffer
 * 	@return Pointer on the new input or output buffer
 * 	@see deleteInOutBuffer()
**/
network_inOutBuffer_t* newInOutBuffer(const network_paramNewInOutBuffer_t *pParam); 

/**
 *  @brief Delete the input or output buffer
 * 	@warning This function free memory
 * 	@param ppInOutBuff address of the pointer on the input or output buffer to delete
 *	@see newInOutBuffer()
**/
void deleteInOutBuffer(network_inOutBuffer_t** ppInOutBuff);

/**
 *  @brief Pass an acknowledgement to a inOutBuffer.
 * 	@details If the inOutBuffer is waiting about an acknowledgement and seqNum is equal to the sequence number waited, the inOutBuffer pops the last data and delete its is waiting acknowledgement.
 * 	@param pInOutBuff Pointer on the input or output buffer
 * 	@param seqNum sequence number of the acknowledgement
 * 	@return error equal to 0 if the data has been correctly acknowledged otherwise equal to 1
**/
int inOutBufferAckReceived(network_inOutBuffer_t* pInOutBuff, int seqNum);

/**
 *  @brief search a inOutBuffer with its identifier in a table
 * 	@param pptabInOutBuff address of the table of pointer of inOutBuffer
 * 	@param tabSize size of the table of pointer of inOutBuffer
 * 	@param id identifier of the inOutBuffer searched
 * 	@return address of the inOutBuffer with the identifier searched (equal to NULL if the inOutBuffer is not find)
**/
network_inOutBuffer_t* inOutBufferWithId(	network_inOutBuffer_t** pptabInOutBuff,
												int tabSize, int id);

/**
 *  @brief Get if the inOutBuffer is waiting an acknowledgement.
 * 	@param pInOutBuff Pointer on the input or output buffer
 * 	@return IsWaitAck equal to 1 if the inOutBuffer is waiting an acknowledgement otherwise equal to 0
**/
int inOutBuffeIsWaitAck( network_inOutBuffer_t* pInOutBuff);


#endif // _IN_OUT_BUFFER_H_
