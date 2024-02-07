#pragma once

namespace Quartz
{
	struct GLGraphics
	{
		bool ready;
	};

	bool CreateGL(GLGraphics* pGraphics);
	void DestroyGL(GLGraphics* pGraphics);
}