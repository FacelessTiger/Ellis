#pragma once

#include <glm/glm.hpp>

#include "Ellis/Renderer/VertexArray.h"

namespace Ellis {

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0,
			OpenGL = 1
		};
	private:
		static API s_API;
	public:
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() const = 0;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) = 0;

		inline static API GetAPI() { return s_API; }
	};

}