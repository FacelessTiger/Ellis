#pragma once

#include <filesystem>

#include "Ellis/Renderer/Texture.h"

namespace Ellis {

	class ContentBrowserPanel
	{
	private:
		std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
	public:
		ContentBrowserPanel();

		void OnImGuiRender();
	};

}