#include "stdafx.h"
#include "krproject.h"

#include <KR3/initializer.h>

Main::Main() noexcept
	:WebCanvas2D(1024, 768)
{
}
Main::~Main() noexcept
{
}

void Main::onDraw() noexcept
{
}


int main()
{
	Main main;
	main_loop(&main);
	return 0;
}
