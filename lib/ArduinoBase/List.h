// Arduino Base Libs
// 18.10.2021
// Stefan Rau
// Dynamic list class

#pragma once
#ifndef _List_h
#define _List_h

#include "Debug.h"

/// <summary>
/// Class that contains a single list element. Base is a double chained list.
/// </summary>
class ListElement
{
public:
	ListElement* _mPrevious = nullptr;	// pointer to predecissor - the 1st element has nullptr 
	ListElement* _mNext = nullptr;		// pointer to successor - the last element has nullptr 
	void* _mObject = nullptr;			// pointer to the contained object
};

/// <summary>
/// Provides the functionality for list processing
/// </summary>
class ListCollection
{
public:
	ListCollection();
	~ListCollection();

	/// <summary>
	/// Adds an object to the list
	/// </summary>
	/// <param name="iObject">Object to add</param>
	void Add(void* iObject);

	/// <summary>
	/// Gets the 1st object of the list
	/// </summary>
	/// <returns>Object to get</returns>
	void* GetFirst();

	/// <summary>
	/// Gets the last object of the list
	/// </summary>
	/// <returns>Object to get</returns>
	void* GetLast(); 

	/// <summary>
	/// Gets the object at the index  
	/// </summary>
	/// <param name="iIndex">Index of the object to get</param>
	/// <returns>Object to get</returns>
	void* Get(int iIndex);

	/// <summary>
	/// Calculates the size of the object list
	/// </summary>
	/// <returns>Size of list</returns>
	int Count();

	/// <summary>
	/// Starts a new iteration
	/// </summary>
	void IterateStart();

	/// <summary>
	/// Iterates the list and gets internally the next element
	/// </summary>
	/// <returns>Object at the current iteration step</returns>
	void* Iterate();

private:
	ListElement* _mFirst = nullptr;		// pointer to 1st element of the list
	ListElement* _mLast = nullptr;		// pointer to last element of the list
	ListElement* _mIterator = nullptr;	// iterator for iterating through a list
};

#endif