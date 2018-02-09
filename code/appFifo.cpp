/*
File:   appFifo.cpp
Author: Taylor Robbins
Date:   02\09\2018
Description: 
	** Handles the input and output Fifos that push data onto the bus
	** and handle receiving the data off the bus.
	** This happens asynchronously to the main thread that runs the UI.
*/

ThreadFunction(TestThreadFunction)
{
	// DEBUG_PrintLine("Thread started with input %p", threadInput);
	
	u32 counter = 10;
	while (counter > 0)
	{
		DEBUG_PrintLine("Counting... %u", counter);
		
		Sleep(1000);
		counter--;
	}
	
	DEBUG_WriteLine("I'm done counting!");
	
	return 0;
}