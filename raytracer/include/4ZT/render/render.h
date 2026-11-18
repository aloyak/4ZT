#pragma once

#include <memory>

#include "4ZT/settings.h"

namespace FourZT {

class Camera;
class World;
class Image;

class Renderer {
public:
	virtual ~Renderer() = default;

	static std::unique_ptr<Renderer> Create(const Settings& settings);

	virtual Image Render(const Settings& settings,
						 const Camera& camera,
						 const World& world) = 0;
};

}