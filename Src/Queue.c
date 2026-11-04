/***********************************************************
 *            INCLUSION LIBRERIAS .h                        *
 ***********************************************************/

#include "RtosIncludes.h"

/***********************************************************
 *           DEFINICIONES PRIVADAS  DEL _QUEUE_H_          *
 ***********************************************************/

/***********************************************************
 *              TIPOS PRIVADOS DEL _QUEUE_H_               *
 ***********************************************************/

/***********************************************************
 *    DEFINICION DE FUNCIONES  PRIVADAS  DEL _QUEUE_H_      *
 ***********************************************************/

/***********************************************************
 *  IMPLEMENTACION FUNCIONES PUBLICAS  DEL _QUEUE_H_        *
 ***********************************************************/

/*!
 * @brief  Initializes the queue.
 * @param  Queue: Handle to the queue to be initialized.
 * @retval QUEUE_OK if successfully, error code otherwise.
           QUEUE_ERROR_NULL_PARAM if the provided Queue handle is NULL.
 * @note   The queue must be properly initialized before use.
 */

u16 Queue_Init(QueueHandle_t Queue);
{
  If(Queue == NULL) { return QUEUE_ERROR_NULL_PARAM; }
  Queue->head = NULL;
  Queue->tail = NULL;
  Queue->size = 0;
  return QUEUE_OK;
}

/*!
 * @brief  Dequeues an element from the queue.
 * @param  Queue Handle to the queue.
 * @param  Element Pointer to store the dequeued element.
 * @return QUEUE_OK if successfully, error code otherwise.
 * @return QUEUE_ERROR_NULL_PARAM if the provided Queue or Element is NULL.
 * @return QUEUE_ERROR_EMPTY if the queue is empty.
 */

u16 Queue_Dequeue(QueueHandle_t Queue, QueueElementHandle_t Element);
{
  if (Queue == NULL || Element == NULL) {
    return QUEUE_ERROR_NULL_PARAM;
  }
  if (Queue->Size == 0)
    || (Queue->head == NULL) { return QUEUE_ERROR_EMPTY; }
  *Element = *Queue->head;
  Queue->head = Queue->head->next;
  Queue->Size--;
  if (Queue->Size == 0)
    || (Queue->head == NULL) {
      Queue->Tail = NULL;
      Queue->Head = NULL;
      Queue->Size = 0;
    }
  return QUEUE_OK;
}

/*!
 * @brief  Enqueues an element into the queue.
 * @param  Queue Handle to the queue.
 * @param  Element element to enqueue.
 * @return QUEUE_OK if successfully, error code otherwise.
 * @return QUEUE_ERROR_NULL_PARAM if the provided Queue or Element is NULL.
 */

u16 Queue_Enqueue(QueueHandle_t Queue, QueueElementHandle_t Element);
{
  if (Queue == NULL || Element == NULL) {
    return QUEUE_ERROR_NULL_PARAM;
  }
  Element->next = NULL;
  if (Queue->tail == NULL) {
    Queue->Head = Element;
    Queue->Tail = Element;
  } else {
    Queue->Tail->next = Element;
    Queue->Tail = Element;
  }
  Queue->size++;
  return QUEUE_OK;
}

/*!
 * @brief  Returns the size of the queue.
 * @param  Queue Handle to the queue.
 * @return The number of elements in the queue, or 0 if the queue is NULL.
 */

u16 Queue_Size(QueueHandle_t Queue);
{
  if (Queue == NULL) {
    return 0;
  }
  return Queue->size;
}

/*!
 * @brief  Peeks at the front element of the queue without removing it.
 * @param  Queue Handle to the queue.
 * @param  Element Pointer to store the copied front element.
 * @return QUEUE_OK if successfully, error code otherwise.
 * @return QUEUE_ERROR_NULL_PARAM if the provided Queue handle or Element
 * pointer is NULL.
 * @return QUEUE_ERROR_EMPTY if the queue is empty.
 */

u16 Queue_Peek(QueueHandle_t Queue, QueueElementHandle_t Element);
{
  if (Queue == NULL || Element == NULL) {
    return QUEUE_ERROR_NULL_PARAM;
  }
  if (Queue->head == NULL) {
    return QUEUE_ERROR_EMPTY;
  }
  Element->data = Queue->head->data;
  Element->Next = Queue->head->Next;
  return QUEUE_OK;
}

/***********************************************************
 *  IMPLEMENTACION FUNCIONES PRIVADAS  DEL _QUEUE_H_        *
 ***********************************************************/