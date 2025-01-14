#pragma once

#include <Cobra/Asset/AssetManagerBase.h>
#include <Cobra/Asset/AssetMetadata.h>

namespace Cobra {

	using AssetRegistry = std::map<AssetHandle, AssetMetadata>;

	class EditorAssetManager : public AssetManagerBase
	{
	private:
		AssetRegistry m_AssetRegistry;
		mutable AssetMap m_LoadedAssets;
	public:
		Ref<Asset> GetAsset(AssetHandle handle) const override;

		bool IsAssetHandleValid(AssetHandle handle) const override;
		bool IsAssetLoaded(AssetHandle handle) const override;
		AssetType GetAssetType(AssetHandle handle) const override;

		void ImportAsset(const std::filesystem::path& filepath);

		void SerializeAssetRegistry();
		bool DeserializeAssetRegistry();

		const AssetMetadata& GetMetadata(AssetHandle handle) const;
		const std::filesystem::path& GetFilePath(AssetHandle handle) const;
		const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }
	};

}