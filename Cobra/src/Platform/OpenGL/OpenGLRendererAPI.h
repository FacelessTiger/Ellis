#pragma once

#include "Cobra/Renderer/RendererAPI.h"

namespace Cobra {

	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void Init() override;
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void SetClearColor(const glm::vec4& color) override;
		void Clear() const override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;

		void SetLineWidth(float width) override;
	};

}