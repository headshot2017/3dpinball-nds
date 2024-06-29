#include "pch.h"
#include "dsi.h"

bool dsi::on = false;

void dsi::init()
{
	on = isDSiMode();
}

bool dsi::isDSi()
{
	return on;
}
