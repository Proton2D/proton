#pragma once
#ifdef PT_EDITOR
#include "Proton/Editor/Panels/EditorPanel.h"

#include <filesystem>

namespace proton {

	class Texture;

	class ContentBrowserPanel : public EditorPanel
	{
	public:
		virtual void OnCreate() override;
		virtual void OnImGuiRender() override;
		virtual void OnUpdate(float ts) override;

	private:
		void Update();
		bool DrawDirectoryItem(const std::filesystem::path& path, Texture* icon);

	private:
		float m_IconSize = 64.0f;
		float m_Padding = 18.0f;

		std::filesystem::path m_ContentRoot = "content";
		std::filesystem::path m_CurrentDirectory = "content";
		std::vector<std::filesystem::path> m_Files;
		std::vector<std::filesystem::path> m_Folders;
		float m_UpdateTimer = 0.0f;

		Unique<Texture> m_BackIcon;
		Unique<Texture> m_FolderIcon;
		Unique<Texture> m_FileIcon;
		Unique<Texture> m_ImageIcon;
		Unique<Texture> m_PrefabIcon;
		Unique<Texture> m_SceneIcon;
	};

}
#endif
