/*
 * answers.h
 *
 *  Created on: Sep 25, 2019
 *      Author: vovan
 */

#ifndef ANSWERS_H_
#define ANSWERS_H_

#include <string>
#include <time.h>
#include "FreeRTOS.h"

class Answers
{
private:

	std::string oracle = "[Oracle] ";
	std::string hm = "Hmmm...";
	std::string ans1 = "Try again.";
	std::string ans2 = "Maybe";
	std::string ans3 = "The lord moves in mysterious ways.";
	std::string ans4 = "Don't quit your day job.";

public:
	Answers(){};
	virtual ~Answers(){};
	void randAns();
	void returnHm();
};


#endif /* ANSWERS_H_ */
