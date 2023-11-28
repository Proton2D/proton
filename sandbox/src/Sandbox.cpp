#include <Proton.h>

#include "MainLayer.h"

class Sandbox : public proton::Application
{
public:
	virtual bool OnCreate() override
	{
		PushLayer(new MainLayer());
		return true;
	}
};

PROTON_APPLICATION_ENTRY_POINT(Sandbox);
