#pragma once

#include "Cobra/Scene/Scene.h"

namespace Cobra {

	class SceneSerializer
	{
	private:
		Ref<Scene> m_Scene;
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::filesystem::path& filepath);
		void SerializeRuntime(const std::filesystem::path& filepath);

		bool Deserialize(const std::filesystem::path& filepath);
		bool DeserializeRuntime(const std::filesystem::path& filepath);
	};

}