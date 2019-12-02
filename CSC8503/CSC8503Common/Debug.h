#pragma once
#include "../../Plugins/OpenGLRendering/OGLRenderer.h"
#include <vector>
#include <string>

namespace NCL {
	class Debug
	{
	public:
		static void Print(const std::string& text, const Vector2&pos, const Vector4& colour = Vector4(0.75f, 0.75f, 0.75f, 1));
		static void DrawLine(const Vector3& startpoint, const Vector3& endpoint, const Vector4& colour = Vector4(1, 0.2f, 0.2f, 1));
		static void DrawLine(const Vector3& startpoint, const Vector3& direction, const float& length, const Vector4& colour = Vector4(1, 0.2f, 0.2f, 1));
		static void DrawCube(const Vector3& position, const Vector3& halfSize, const Vector4& color = Vector4(1, 0.2f, 0.2f, 1));

		static void SetRenderer(OGLRenderer* r) {
			renderer = r;
		}

		static void FlushRenderables();

	protected:
		struct DebugStringEntry {
			std::string	data;
			Vector2 position;
			Vector4 colour;
		};

		struct DebugLineEntry {
			Vector3 start;
			Vector3 end;
			Vector4 colour;
		};

		Debug() {}
		~Debug() {}

		static std::vector<DebugStringEntry>	stringEntries;
		static std::vector<DebugLineEntry>	lineEntries;

		static OGLRenderer* renderer;
	};
}

