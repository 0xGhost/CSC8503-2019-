#include "Debug.h"

using namespace NCL;

OGLRenderer* Debug::renderer = nullptr;

std::vector<Debug::DebugStringEntry>	Debug::stringEntries;
std::vector<Debug::DebugLineEntry>		Debug::lineEntries;


void Debug::Print(const std::string& text, const Vector2& pos, const Vector4& colour) {
	DebugStringEntry newEntry;

	newEntry.data = text;
	newEntry.position = pos;
	newEntry.colour = colour;

	stringEntries.emplace_back(newEntry);
}

void Debug::DrawLine(const Vector3& startpoint, const Vector3& endpoint, const Vector4& colour) {
	DebugLineEntry newEntry;

	newEntry.start = startpoint;
	newEntry.end = endpoint;
	newEntry.colour = colour;

	lineEntries.emplace_back(newEntry);
}

void Debug::DrawLine(const Vector3& startpoint, const Vector3& direction, const float& length, const Vector4& colour)
{
	Debug::DrawLine(startpoint, startpoint + direction.Normalised() * length, colour);
}

void NCL::Debug::DrawCube(const Vector3& position, const Vector3& halfSize, const Vector4& color)
{
	Debug::DrawLine(position + halfSize * Vector3(1, 1, 1), position + halfSize * Vector3(1, 1, -1), color);
	Debug::DrawLine(position + halfSize * Vector3(1, 1, -1), position + halfSize * Vector3(-1, 1, -1), color);
	Debug::DrawLine(position + halfSize * Vector3(-1, 1, -1), position + halfSize * Vector3(-1, 1, 1), color);
	Debug::DrawLine(position + halfSize * Vector3(-1, 1, 1), position + halfSize * Vector3(1, 1, 1), color);

	Debug::DrawLine(position + halfSize * Vector3(1, -1, 1), position + halfSize * Vector3(1, -1, -1), color);
	Debug::DrawLine(position + halfSize * Vector3(1, -1, -1), position + halfSize * Vector3(-1, -1, -1), color);
	Debug::DrawLine(position + halfSize * Vector3(-1, -1, -1), position + halfSize * Vector3(-1, -1, 1), color);
	Debug::DrawLine(position + halfSize * Vector3(-1, -1, 1), position + halfSize * Vector3(1, -1, 1), color);

	Debug::DrawLine(position + halfSize * Vector3(1, 1, 1), position + halfSize * Vector3(1, -1, 1), color);
	Debug::DrawLine(position + halfSize * Vector3(1, 1, -1), position + halfSize * Vector3(1, -1, -1), color);
	Debug::DrawLine(position + halfSize * Vector3(-1, 1, 1), position + halfSize * Vector3(-1, -1, 1), color);
	Debug::DrawLine(position + halfSize * Vector3(-1, 1, -1), position + halfSize * Vector3(-1, -1, -1), color);
}

void Debug::FlushRenderables() {
	if (!renderer) {
		return;
	}
	for (const auto& i : stringEntries) {
		renderer->DrawString(i.data, i.position, i.colour);
	}

	for (const auto& i : lineEntries) {
		renderer->DrawLine(i.start, i.end, i.colour);
	}

	stringEntries.clear();
	lineEntries.clear();
}