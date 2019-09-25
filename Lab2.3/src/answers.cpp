/*
 * answers.cpp
 *
 *  Created on: Sep 25, 2019
 *      Author: vovan
 */
#include "answers.h"

void Answers::randAns()
{
	int i = rand() % 3;
	if(i == 0)
	{
		DEBUGOUT("%s%s\r\n", oracle.c_str(), ans1.c_str());
	}
	else if(i == 1)
	{
		DEBUGOUT("%s%s\r\n", oracle.c_str(), ans2.c_str());
	}
	else if(i == 2)
	{
		DEBUGOUT("%s%s\r\n", oracle.c_str(), ans3.c_str());
	}
	else if(i == 3)
	{
		DEBUGOUT("%s%s\r\n", oracle.c_str(), ans4.c_str());
	}
}

void Answers::returnHm()
{
	DEBUGOUT("%s%s\r\n", oracle.c_str(), hm.c_str());
}

