#include "Deque.h"
#include <stdlib.h>
#include <string.h>

/// @brief Ensure buffer capacity before push operations
/// @param cb Pointer to circular buffer
/// @return None
static void CBuffer_EnsureCapacity(GenericDeque_t* cb) {
    /* Place the actual comment here */
    /* Evict oldest or reallocate memory if buffer reaches capacity */
    if (cb->Size >= CBUFFER_MAX_SIZE) {
        if (cb->OnChangeCallBack != NULL) cb->OnChangeCallBack(eCBuff_IsFull);
        CBuffer_PopBack(cb);
    }
    else if (cb->Size == cb->ReservedSize) {
        CBuffSize_t newCap = cb->ReservedSize + (cb->ReservedSize * 15 / 100);
        newCap = (newCap <= cb->ReservedSize) ? cb->ReservedSize + 1 : newCap;
        newCap = (newCap > CBUFFER_MAX_SIZE) ? CBUFFER_MAX_SIZE : newCap;
        
        GenericElement_t* newArr = calloc(newCap, sizeof(GenericElement_t));
        
        if (newArr != NULL) {
            for (CBuffSize_t i = 0; i < cb->Size; i++) {
                newArr[i] = cb->Arr[(cb->Tail + i) % cb->ReservedSize];
            }
            free(cb->Arr);
            cb->Arr = newArr;
            cb->Tail = 0;
            cb->Head = cb->Size;
            cb->ReservedSize = newCap;

            if (cb->OnChangeCallBack != NULL) cb->OnChangeCallBack(eCBuff_ReAllocate);
        }
        else {
            CBuffer_PopBack(cb);
        }
    }
}

/// @brief Create a generic circular buffer
/// @param ReservedSize Initial capacity
/// @return Pointer to new deque instance
GenericDeque_t* CBuffer_Create(CBuffSize_t ReservedSize) {
    /* Place the actual comment here */
    /* Allocate and initialize buffer structure */
    ReservedSize = (ReservedSize == 0) ? 1 : ReservedSize;
    ReservedSize = (ReservedSize > CBUFFER_MAX_SIZE) ? CBUFFER_MAX_SIZE : ReservedSize;
    
    GenericDeque_t* cb = malloc(sizeof(GenericDeque_t));
    if (cb != NULL) {
        cb->Arr = calloc(ReservedSize, sizeof(GenericElement_t));
        if (cb->Arr != NULL) {
            cb->ReservedSize = ReservedSize;
            cb->Size = cb->Head = cb->Tail = 0;
            cb->OnChangeCallBack = NULL;
        }
        else {
            free(cb);
            cb = NULL;
        }
    }
    return cb;
}

/// @brief Destroy buffer memory
/// @param cb Pointer to circular buffer
/// @return None
void CBuffer_Destroy(GenericDeque_t* cb) {
    /* Place the actual comment here */
    /* Free all elements and object pointer */
    if (cb != NULL) {
        CBuffer_Clear(cb);
        if (cb->Arr != NULL) free(cb->Arr);
        free(cb);
    }
}

/// @brief Check if buffer is empty
/// @param cb Pointer to circular buffer
/// @return True if empty
bool CBuffer_IsEmpty(const GenericDeque_t* cb) {
    /* Place the actual comment here */
    /* Validate size parameter */
    return (cb == NULL || cb->Size == 0);
}

/// @brief Check if buffer is full
/// @param cb Pointer to circular buffer
/// @return True if at max capacity
bool CBuffer_IsFull(const GenericDeque_t* cb) {
    /* Place the actual comment here */
    /* Verify against max limit */
    return (cb != NULL && cb->Size >= CBUFFER_MAX_SIZE);
}

/// @brief Get current element count
/// @param cb Pointer to circular buffer
/// @return Element count
CBuffSize_t CBuffer_Size(const GenericDeque_t* cb) {
    /* Place the actual comment here */
    /* Return logical size */
    return (cb != NULL) ? cb->Size : 0;
}

/// @brief Assign state change callback
/// @param cb Pointer to circular buffer
/// @param CallBack Function pointer
/// @return None
void CBuffer_SetCallback(GenericDeque_t* cb, void (*CallBack)(enum OnChangeType Type)) {
    /* Place the actual comment here */
    /* Attach user callback */
    if (cb != NULL) cb->OnChangeCallBack = CallBack;
}

/// @brief Clear state change callback
/// @param cb Pointer to circular buffer
/// @return None
void CBuffer_ClearCallback(GenericDeque_t* cb) {
    /* Place the actual comment here */
    /* Detach callback */
    if (cb != NULL) cb->OnChangeCallBack = NULL;
}

/// @brief Push element to front
/// @param cb Pointer to circular buffer
/// @param Data Pointer to payload
/// @param DataSize Size of payload
/// @return New buffer size
CBuffSize_t CBuffer_PushFront(GenericDeque_t* cb, const CBuffBase_t Data, CBuffSize_t DataSize) {
    /* Place the actual comment here */
    /* Write to head and advance pointer */
    if (cb == NULL || Data == NULL || DataSize == 0) return cb ? cb->Size : 0;
    
    CBuffer_EnsureCapacity(cb);
    uint8_t* newPtr = malloc(DataSize);
    if (newPtr == NULL) return cb->Size;
    
    if (cb->Arr[cb->Head].DataPtr != NULL) free(cb->Arr[cb->Head].DataPtr);
    
    cb->Arr[cb->Head].DataPtr = newPtr;
    memcpy(cb->Arr[cb->Head].DataPtr, Data, DataSize);
    cb->Arr[cb->Head].DataSize = DataSize;
    
    cb->Head = (cb->Head + 1) % cb->ReservedSize;
    cb->Size++;
    
    if (cb->OnChangeCallBack != NULL) cb->OnChangeCallBack(eCBuff_PushFront);
    return cb->Size;
}

/// @brief Push element to back
/// @param cb Pointer to circular buffer
/// @param Data Pointer to payload
/// @param DataSize Size of payload
/// @return New buffer size
CBuffSize_t CBuffer_PushBack(GenericDeque_t* cb, const CBuffBase_t Data, CBuffSize_t DataSize) {
    /* Place the actual comment here */
    /* Retreat tail and write data */
    if (cb == NULL || Data == NULL || DataSize == 0) return cb ? cb->Size : 0;
    
    CBuffer_EnsureCapacity(cb);
    CBuffSize_t newTail = (cb->Tail + cb->ReservedSize - 1) % cb->ReservedSize;
    uint8_t* newPtr = malloc(DataSize);
    
    if (newPtr == NULL) return cb->Size;
    if (cb->Arr[newTail].DataPtr != NULL) free(cb->Arr[newTail].DataPtr);
    
    cb->Arr[newTail].DataPtr = newPtr;
    memcpy(cb->Arr[newTail].DataPtr, Data, DataSize);
    cb->Arr[newTail].DataSize = DataSize;
    
    cb->Tail = newTail;
    cb->Size++;
    
    if (cb->OnChangeCallBack != NULL) cb->OnChangeCallBack(eCBuff_PushBack);
    return cb->Size;
}

/// @brief Remove element from front
/// @param cb Pointer to circular buffer
/// @return New buffer size
CBuffSize_t CBuffer_PopFront(GenericDeque_t* cb) {
    /* Place the actual comment here */
    /* Free head memory and retreat index */
    if (cb == NULL || cb->Size == 0) return cb ? cb->Size : 0;
    
    CBuffSize_t frontIdx = (cb->Head + cb->ReservedSize - 1) % cb->ReservedSize;
    if (cb->Arr[frontIdx].DataPtr != NULL) {
        free(cb->Arr[frontIdx].DataPtr);
        cb->Arr[frontIdx].DataPtr = NULL;
    }
    
    cb->Arr[frontIdx].DataSize = 0;
    cb->Head = frontIdx;
    cb->Size--;
    
    if (cb->OnChangeCallBack != NULL) cb->OnChangeCallBack(eCBuff_PopFront);
    return cb->Size;
}

/// @brief Remove element from back
/// @param cb Pointer to circular buffer
/// @return New buffer size
CBuffSize_t CBuffer_PopBack(GenericDeque_t* cb) {
    /* Place the actual comment here */
    /* Free tail memory and advance index */
    if (cb == NULL || cb->Size == 0) return cb ? cb->Size : 0;
    
    if (cb->Arr[cb->Tail].DataPtr != NULL) {
        free(cb->Arr[cb->Tail].DataPtr);
        cb->Arr[cb->Tail].DataPtr = NULL;
    }
    
    cb->Arr[cb->Tail].DataSize = 0;
    cb->Tail = (cb->Tail + 1) % cb->ReservedSize;
    cb->Size--;
    
    if (cb->OnChangeCallBack != NULL) cb->OnChangeCallBack(eCBuff_PopBack);
    return cb->Size;
}

/// @brief Access generic element by index
/// @param cb Pointer to circular buffer
/// @param Index Logical position
/// @return Pointer to element
GenericElement_t* CBuffer_At(GenericDeque_t* cb, int32_t Index) {
    /* Place the actual comment here */
    /* Calculate physical index map */
    if (cb == NULL || cb->Size == 0) return NULL;
    
    int32_t logicalIdx = (Index < 0) ? (Index + cb->Size) : Index;
    if (logicalIdx < 0 || logicalIdx >= cb->Size) return NULL;
    
    CBuffSize_t physicalIdx = (cb->Head + cb->ReservedSize - 1 - logicalIdx) % cb->ReservedSize;
    return &cb->Arr[physicalIdx];
}

/// @brief Get front element
/// @param cb Pointer to circular buffer
/// @return Pointer to element
GenericElement_t* CBuffer_Front(GenericDeque_t* cb) {
    /* Place the actual comment here */
    /* Fetch index 0 */
    return CBuffer_At(cb, 0);
}

/// @brief Get back element
/// @param cb Pointer to circular buffer
/// @return Pointer to element
GenericElement_t* CBuffer_Back(GenericDeque_t* cb) {
    /* Place the actual comment here */
    /* Fetch index -1 */
    return CBuffer_At(cb, -1);
}

/// @brief Clear buffer contents
/// @param cb Pointer to circular buffer
/// @return None
void CBuffer_Clear(GenericDeque_t* cb) {
    /* Place the actual comment here */
    /* Erase all items and reset pointers */
    if (cb != NULL) {
        bool changed = (cb->Size > 0);
        
        while (cb->Size > 0) {
            if (cb->Arr[cb->Tail].DataPtr != NULL) {
                free(cb->Arr[cb->Tail].DataPtr);
                cb->Arr[cb->Tail].DataPtr = NULL;
            }
            cb->Arr[cb->Tail].DataSize = 0;
            cb->Tail = (cb->Tail + 1) % cb->ReservedSize;
            cb->Size--;
        }
        
        cb->Head = cb->Tail = 0;
        if (changed && cb->OnChangeCallBack != NULL) cb->OnChangeCallBack(eCBuff_Clear);
    }
}